#include "FileHashCalculator.h"
#include "FileHashPrinter.h"
#include "FileReader.h"

#include <boost/program_options.hpp>

#include <filesystem>
#include <iostream>
#include <string>


namespace
{

namespace option
{

auto help = "help";
auto input = "input";
auto output = "output";
auto size = "size";

} // namespace option

void PrintThatOptionIsUnspecified(std::string const& optionName)
{
    std::cout << optionName << " option isn't specified" << std::endl;
}

} // namespace

int main(int argc, char** argv)
{
    int bufferSizeInBytes = 0;
    std::string inputFile;
    std::string outputFile;

    namespace po = boost::program_options;

    po::options_description optionsDescription;
    optionsDescription.add_options()
        (option::help, "Provide description of command line arguments")
        (option::input, po::value<std::string>(&inputFile), "Input file")
        (option::output, po::value<std::string>(&outputFile), "Output file")
        (option::size, po::value<int>(&bufferSizeInBytes)->default_value(1024 * 1024), "Size of block, in bytes");

    po::variables_map commandLineArgs;
    po::store(po::parse_command_line(argc, argv, optionsDescription), commandLineArgs);
    po::notify(commandLineArgs);

    if (argc < 2 || commandLineArgs.count(option::help))
    {
        std::cout << optionsDescription << std::endl;
        return 0;
    }

    if (!commandLineArgs.count(option::input))
    {
        PrintThatOptionIsUnspecified(option::input);
        return 0;
    }

    if (!commandLineArgs.count(option::output))
    {
        PrintThatOptionIsUnspecified(option::output);
        return 0;
    }

    if (bufferSizeInBytes <= 0 || bufferSizeInBytes > 1024 * 1024 * 1024)
    {
        std::cout << option::size << " option value is incorrect. Allowed range (0, 1Gb]." << std::endl;
        return 0;
    }

    try
    {
        FileReader reader{ inputFile, static_cast<size_t>(bufferSizeInBytes) };
        FileHashPrinter printer{ outputFile };

        FileHashCalculator calculator{ reader, printer };
        calculator.CalculateHash();

        printer.FlushAllHashes();
    }
    catch (std::exception const& e)
    {
        std::cout << "Blocks' hash calculation failed. Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
