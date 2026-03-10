#include "NihonKohdenData.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
    {
        std::cout << "Usage: mferdump <file.mwf>\n"
                  << "Print a raw hex dump of all MFER tags.\n";
        return 0;
    }
    if (argc != 2)
    {
        std::cerr << "Usage: mferdump <file.mwf>\n";
        return 1;
    }

    try
    {
        NihonKohdenData data(argv[1]);
        data.printHexData();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
