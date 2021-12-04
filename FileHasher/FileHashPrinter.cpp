#include "FileHashPrinter.h"

#include "MyError.h"

#include<exception>
#include <fstream>
#include <string>


namespace
{

auto constexpr ErrorPrefix = "File printer error: ";

std::ofstream CreateStream(std::filesystem::path const& filePath)
{
    std::ofstream stream;

    try
    {
        stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        stream.open(filePath, std::ios::out | std::ios::trunc);
    }
    catch (std::exception const& error)
    {
        throw MyError(ErrorPrefix + filePath.string() + " opening failed. Reason: " + error.what());
    }

    return stream;
}

void ValidateStream(std::ofstream const& stream, std::filesystem::path const& filePath)
{
    if (stream.bad() || !stream.is_open())
    {
        throw MyError(ErrorPrefix + filePath.string() + " stream isn't initialized");
    }
}

} // namespace

FileHashPrinter::FileHashPrinter(std::filesystem::path filePath) :
    m_filePath(std::move(filePath)),
    m_stream(CreateStream(m_filePath))
{
}

void FileHashPrinter::PrintHash(BlockHash const& blockHash)
{
    std::lock_guard<std::mutex> const lock{ m_streamMutex };
    ValidateStream(m_stream, m_filePath);

    try
    {
        m_stream << std::to_string(blockHash.Value);
    }
    catch (std::exception const& error)
    {
        m_stream = {};
        throw MyError(ErrorPrefix + m_filePath.string() + " print hash failed for block #"
            + std::to_string(blockHash.Position) + ". Reason: " + error.what());
    }
}

void FileHashPrinter::FlushAllHashes()
{
    std::lock_guard<std::mutex> const lock{ m_streamMutex };
    ValidateStream(m_stream, m_filePath);

    try
    {
        m_stream.flush();
    }
    catch (std::exception const& error)
    {
        m_stream = {};
        throw MyError(ErrorPrefix + m_filePath.string() + " print hash failed. Reason: " + error.what());
    }
}
