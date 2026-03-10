#include "NihonKohdenData.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static std::vector<int> parseIndices(const std::string &list)
{
    std::vector<int> result;
    std::istringstream ss(list);
    std::string token;
    while (std::getline(ss, token, ','))
        result.push_back(std::stoi(token));
    return result;
}

int main(int argc, char *argv[])
{
    std::string inputFile, outputFile, channelList;
    double start = 0, end = 0;

    try
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help")
            {
                std::cout << "Usage: mfer2csv <input.mwf> -o <output.csv> [options]\n"
                          << "Export waveform data to CSV.\n\n"
                          << "Options:\n"
                          << "  -o, --output <file>     Output CSV file (required)\n"
                          << "  --channels <0,1,...>    Channel indices to export (default: all)\n"
                          << "  --start <seconds>       Start of interval (default: 0)\n"
                          << "  --end <seconds>         End of interval (default: end of recording)\n";
                return 0;
            }
            else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
                outputFile = argv[++i];
            else if (arg == "--channels" && i + 1 < argc)
                channelList = argv[++i];
            else if (arg == "--start" && i + 1 < argc)
                start = std::stod(argv[++i]);
            else if (arg == "--end" && i + 1 < argc)
                end = std::stod(argv[++i]);
            else if (inputFile.empty())
                inputFile = arg;
            else
            {
                std::cerr << "Unexpected argument: " << arg << "\n";
                return 1;
            }
        }

        if (inputFile.empty() || outputFile.empty())
        {
            std::cerr << "Usage: mfer2csv <input.mwf> -o <output.csv>\n";
            return 1;
        }

        NihonKohdenData data(inputFile);

        if (!channelList.empty())
        {
            int count = static_cast<int>(data.getHeader().channelCount);
            for (int i = 0; i < count; ++i)
                data.setChannelSelection(i, false);
            for (int idx : parseIndices(channelList))
                data.setChannelSelection(idx, true);
        }

        data.setIntervalSelection(start, end);
        data.writeToCsv(outputFile);
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
