#pragma once

#include "Block.h"

#include <ios>
#include <fstream>
#include <mutex>
#include <optional>
#include <filesystem>
#include <vector>


class FileReader
{
public:
    FileReader(std::filesystem::path filePath, std::size_t blockSizeToRead);

public:
    std::optional<BlockContents> ReadNext();
    void Close();

private:
    std::filesystem::path const m_filePath;
    std::size_t const m_blockSizeToReadInBytes = 0;
    std::size_t m_countOfReadBlocks = 0;
    std::streamsize m_streamSizeInBytes = 0;
    std::streamsize m_currentPositionInBytes = 0;
    std::ifstream m_stream;
    std::mutex m_streamReadMutex;
};
