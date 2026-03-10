#include "NihonKohdenData.h"
#include <iostream>
#include <string>
#include <algorithm>

static std::string trimNulls(const std::string &s)
{
    std::string r = s;
    r.erase(std::remove(r.begin(), r.end(), '\0'), r.end());
    return r;
}

static std::string jsonEscape(const std::string &s)
{
    std::string r;
    for (unsigned char c : s)
    {
        switch (c)
        {
        case '"':  r += "\\\""; break;
        case '\\': r += "\\\\"; break;
        case '\n': r += "\\n";  break;
        case '\r': r += "\\r";  break;
        case '\t': r += "\\t";  break;
        default:
            if (c >= 0x20) r += static_cast<char>(c);
            break;
        }
    }
    return r;
}

int main(int argc, char *argv[])
{
    std::string filename;
    bool json = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: mferinfo <file.mwf> [--json]\n"
                      << "Print header information from an MFER file.\n\n"
                      << "Options:\n"
                      << "  --json   Output in JSON format\n";
            return 0;
        }
        else if (arg == "--json")
            json = true;
        else if (filename.empty())
            filename = arg;
        else
        {
            std::cerr << "Unexpected argument: " << arg << "\n";
            return 1;
        }
    }

    if (filename.empty())
    {
        std::cerr << "Usage: mferinfo <file.mwf> [--json]\n";
        return 1;
    }

    try
    {
        NihonKohdenData data(filename);
        Header h = data.getHeader();

        if (json)
        {
            std::cout << "{\n"
                      << "  \"preamble\": \""        << jsonEscape(trimNulls(h.preamble))          << "\",\n"
                      << "  \"modelInfo\": \""        << jsonEscape(trimNulls(h.modelInfo))          << "\",\n"
                      << "  \"measurementTime\": \""  << jsonEscape(h.measurementTimeISO)            << "\",\n"
                      << "  \"patientID\": \""        << jsonEscape(trimNulls(h.patientID))          << "\",\n"
                      << "  \"patientName\": \""      << jsonEscape(trimNulls(h.patientName))        << "\",\n"
                      << "  \"birthDate\": \""        << jsonEscape(h.birthDateISO)                  << "\",\n"
                      << "  \"patientSex\": \""       << jsonEscape(h.patientSex)                    << "\",\n"
                      << "  \"samplingInterval\": \"" << jsonEscape(h.samplingIntervalString)        << "\",\n"
                      << "  \"sequenceCount\": "      << h.sequenceCount                             << ",\n"
                      << "  \"channelCount\": "       << static_cast<int>(h.channelCount)            << ",\n"
                      << "  \"nibpEventCount\": "     << h.events.size()                             << ",\n"
                      << "  \"channels\": [\n";

            for (size_t i = 0; i < h.channels.size(); ++i)
            {
                const auto &ch = h.channels[i];
                std::cout << "    {\n"
                          << "      \"index\": "               << i                                          << ",\n"
                          << "      \"attribute\": \""         << jsonEscape(ch.leadInfo.attribute)          << "\",\n"
                          << "      \"samplingResolution\": \"" << jsonEscape(ch.leadInfo.samplingResolution) << "\",\n"
                          << "      \"blockLength\": "         << ch.blockLength                             << "\n"
                          << "    }" << (i + 1 < h.channels.size() ? "," : "") << "\n";
            }
            std::cout << "  ]\n}\n";
        }
        else
        {
            std::cout << h.toString();
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
