#pragma once

#include "Block.h"

#include <filesystem>
#include <fstream>
#include <mutex>


class FileHashPrinter
{
public:
    FileHashPrinter(std::filesystem::path filePath);

public:
    void PrintHash(BlockHash const& blockHash);
    void FlushAllHashes();

public:
    std::filesystem::path const m_filePath;
    std::ofstream m_stream;
    std::mutex m_streamMutex;
};
