#pragma once

#include "FileReader.h"
#include "FileHashPrinter.h"


class FileHashCalculator
{
public:
    FileHashCalculator(FileReader& fileReader, FileHashPrinter& printer);

    void CalculateHash();

private:
    FileReader& m_fileReader;
    FileHashPrinter& m_printer;
};