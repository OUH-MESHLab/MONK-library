#include "NihonKohdenData.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ---- per-file result -------------------------------------------------------

struct ChannelInfo
{
    std::string name;
    size_t expected = 0;
    size_t actual = 0;
    size_t nanCount = 0;
};

struct FileResult
{
    fs::path path;
    uintmax_t fileSize = 0;

    // check outcomes
    bool parse_ok = false;
    bool header_ok = false;
    bool data_size_ok = false;
    bool roundtrip_ok = true;   // true = "not failed" (may be skipped)
    bool roundtrip_skip = false;

    // header info (populated on success)
    int channelCount = 0;
    int sequenceCount = 0;
    float samplingInterval = 0;
    size_t nibpEvents = 0;

    // per-channel info
    std::vector<ChannelInfo> channels;

    // first error message encountered
    std::string error;
};

// ---- helpers ---------------------------------------------------------------

static std::string fmtSize(uintmax_t bytes)
{
    if (bytes < 1024ULL)             return std::to_string(bytes) + " B";
    if (bytes < 1024ULL * 1024)      return std::to_string(bytes / 1024) + " KB";
    if (bytes < 1024ULL * 1024 * 1024)
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    return std::to_string(bytes / (1024ULL * 1024 * 1024)) + " GB";
}

static bool filesEqual(const std::string &a, const std::string &b)
{
    std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
    if (!fa || !fb) return false;
    return std::equal(
        std::istreambuf_iterator<char>(fa), {},
        std::istreambuf_iterator<char>(fb), {});
}

static std::vector<fs::path> collectFiles(const fs::path &root)
{
    std::vector<fs::path> files;
    for (const auto &entry : fs::recursive_directory_iterator(
             root, fs::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".mwf")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}

// ---- core test logic -------------------------------------------------------

static FileResult testFile(const fs::path &path, bool doRoundtrip)
{
    FileResult r;
    r.path = path;
    r.fileSize = fs::file_size(path);

    // 1. Parse ----------------------------------------------------------------
    std::unique_ptr<NihonKohdenData> data;
    try
    {
        data = std::make_unique<NihonKohdenData>(path.string());
        r.parse_ok = true;
    }
    catch (const std::exception &e)
    {
        r.error = e.what();
        return r;
    }

    // 2. Header sanity --------------------------------------------------------
    Header h;
    try
    {
        h = data->getHeader();
    }
    catch (const std::exception &e)
    {
        r.error = std::string("getHeader: ") + e.what();
        return r;
    }

    r.channelCount    = static_cast<int>(h.channelCount);
    r.sequenceCount   = static_cast<int>(h.sequenceCount);
    r.samplingInterval = h.samplingInterval;
    r.nibpEvents      = h.events.size();

    r.header_ok = h.channelCount > 0
               && h.sequenceCount > 0
               && h.samplingInterval > 0.0f;
    if (!r.header_ok && r.error.empty())
        r.error = "channelCount=" + std::to_string(h.channelCount)
                + " sequenceCount=" + std::to_string(h.sequenceCount)
                + " samplingInterval=" + std::to_string(h.samplingInterval);

    // 3. Data-size + NaN ratio ------------------------------------------------
    r.data_size_ok = true;
    for (const auto &ch : h.channels)
    {
        ChannelInfo ci;
        ci.name     = ch.leadInfo.attribute;
        ci.expected = static_cast<size_t>(ch.blockLength) * h.sequenceCount;
        ci.actual   = ch.data.size();
        ci.nanCount = static_cast<size_t>(
            std::count_if(ch.data.begin(), ch.data.end(),
                          [](double v) { return std::isnan(v); }));

        if (ci.actual != ci.expected)
        {
            r.data_size_ok = false;
            if (r.error.empty())
                r.error = "Channel '" + ci.name + "': expected "
                        + std::to_string(ci.expected) + " samples, got "
                        + std::to_string(ci.actual);
        }
        r.channels.push_back(ci);
    }

    // 4. Binary roundtrip (small files only) ----------------------------------
    constexpr uintmax_t ROUNDTRIP_LIMIT = 10ULL * 1024 * 1024; // 10 MB
    if (!doRoundtrip || r.fileSize > ROUNDTRIP_LIMIT)
    {
        r.roundtrip_skip = true;
    }
    else
    {
        try
        {
            fs::path tmp = fs::temp_directory_path()
                         / ("monk_rt_" + path.filename().string());
            data->writeToBinary(tmp.string());
            r.roundtrip_ok = filesEqual(path.string(), tmp.string());
            fs::remove(tmp);
            if (!r.roundtrip_ok && r.error.empty())
                r.error = "Roundtrip byte mismatch";
        }
        catch (const std::exception &e)
        {
            r.roundtrip_ok = false;
            r.error = std::string("writeToBinary: ") + e.what();
        }
    }

    return r;
}

// ---- reporting -------------------------------------------------------------

static void printSummary(const std::vector<FileResult> &results)
{
    // column widths
    const int W_NAME = 38, W_SIZE = 8, W_CH = 4, W_SEQ = 6,
              W_NIBP = 5, W_IVL = 8;
    const std::string sep(105, '-');

    std::cout << "\n" << sep << "\n";
    printf("%-*s  %*s  %*s  %*s  %*s  %*s   Parse  Header  DataSz  Roundtrip\n",
           W_NAME, "File", W_SIZE, "Size", W_CH, " Ch", W_SEQ, "  Seq",
           W_NIBP, "NIBP", W_IVL, "   IVL(s)");
    std::cout << sep << "\n";

    int passed = 0, failed = 0;
    for (const auto &r : results)
    {
        bool ok = r.parse_ok && r.header_ok && r.data_size_ok && r.roundtrip_ok;
        ok ? ++passed : ++failed;

        std::string name = r.path.filename().string();
        if (static_cast<int>(name.size()) > W_NAME)
            name = name.substr(0, W_NAME - 3) + "...";

        std::string ivl = r.samplingInterval > 0
                        ? std::to_string(r.samplingInterval).substr(0, 8)
                        : "-";

        printf("%-*s  %*s  %*d  %*d  %*zu  %*s   %-7s%-8s%-8s%s\n",
               W_NAME, name.c_str(),
               W_SIZE, fmtSize(r.fileSize).c_str(),
               W_CH,   r.channelCount,
               W_SEQ,  r.sequenceCount,
               W_NIBP, r.nibpEvents,
               W_IVL,  ivl.c_str(),
               r.parse_ok    ? "PASS"           : "FAIL",
               r.header_ok   ? "PASS"           : (r.parse_ok ? "FAIL" : "-   "),
               r.data_size_ok ? "PASS"           : (r.parse_ok ? "FAIL" : "-   "),
               r.roundtrip_skip ? "SKIP"
                   : (r.roundtrip_ok ? "PASS" : "FAIL")
        );

        if (!r.error.empty())
            std::cout << "   ERROR: " << r.error << "\n";

        // NaN warnings: channels with > 50% NaN
        for (const auto &ch : r.channels)
        {
            if (ch.expected > 0 && ch.nanCount * 2 > ch.expected)
                printf("   WARN: channel '%s' has %zu/%zu NaN values (%.0f%%)\n",
                       ch.name.c_str(), ch.nanCount, ch.expected,
                       100.0 * ch.nanCount / ch.expected);
        }
    }

    std::cout << sep << "\n";
    std::cout << "Result: " << passed << "/" << results.size() << " passed";
    if (failed > 0)
        std::cout << "  (" << failed << " FAILED)";
    std::cout << "\n\n";
}

// ---- main ------------------------------------------------------------------

int main(int argc, char *argv[])
{
    if (argc < 2 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")
    {
        std::cout << "Usage: corpus_test <directory> [--roundtrip]\n"
                  << "Run parse, header, data-size, and optional roundtrip checks\n"
                  << "on every .mwf file found recursively under <directory>.\n\n"
                  << "Options:\n"
                  << "  --roundtrip   Also verify write→re-read byte identity (files ≤ 10 MB)\n";
        return argc < 2 ? 1 : 0;
    }

    fs::path dir = argv[1];
    bool doRoundtrip = (argc >= 3 && std::string(argv[2]) == "--roundtrip");

    if (!fs::is_directory(dir))
    {
        std::cerr << "Error: not a directory: " << dir << "\n";
        return 1;
    }

    auto files = collectFiles(dir);
    if (files.empty())
    {
        std::cerr << "No .mwf files found under " << dir << "\n";
        return 1;
    }

    std::cout << "corpus_test  |  " << files.size() << " files  |  dir: " << dir
              << (doRoundtrip ? "  |  roundtrip ON" : "") << "\n\n";

    std::vector<FileResult> results;
    results.reserve(files.size());

    for (size_t i = 0; i < files.size(); ++i)
    {
        const auto &f = files[i];
        std::cout << "[" << (i + 1) << "/" << files.size() << "] "
                  << f.filename().string() << " (" << fmtSize(fs::file_size(f)) << ") ... "
                  << std::flush;

        results.push_back(testFile(f, doRoundtrip));
        const auto &r = results.back();
        bool ok = r.parse_ok && r.header_ok && r.data_size_ok && r.roundtrip_ok;
        std::cout << (ok ? "OK" : "FAIL") << "\n";
    }

    printSummary(results);

    bool anyFailed = std::any_of(results.begin(), results.end(),
        [](const FileResult &r) {
            return !r.parse_ok || !r.header_ok || !r.data_size_ok || !r.roundtrip_ok;
        });
    return anyFailed ? 1 : 0;
}
