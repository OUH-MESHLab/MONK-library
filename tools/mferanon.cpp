#include "NihonKohdenData.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::string inputFile, outputFile;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: mferanon <input.mwf> -o <output.mwf>\n"
                      << "Zero patient identifying fields (ID, name, DOB, sex).\n";
            return 0;
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc)
            outputFile = argv[++i];
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
        std::cerr << "Usage: mferanon <input.mwf> -o <output.mwf>\n";
        return 1;
    }

    try
    {
        NihonKohdenData data(inputFile);
        data.anonymize();
        data.writeToBinary(outputFile);
        std::cout << "Anonymized: " << inputFile << " -> " << outputFile << "\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
