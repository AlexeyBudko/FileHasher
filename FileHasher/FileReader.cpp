#include "FileReader.h"

#include "MyError.h"

#include <algorithm>
#include <filesystem>


namespace
{

auto constexpr ErrorPrefix = "File reader error: ";
auto const BufferMaxSize = Buffer().max_size();

std::ifstream CreateStream(std::filesystem::path const& filePath, std::streamsize& streamSize)
{
    if (!std::filesystem::exists(filePath))
    {
        throw MyError(ErrorPrefix + filePath.string() + " doesn't exist");
    }

    if (!std::filesystem::is_regular_file(filePath))
    {
        throw MyError(ErrorPrefix + filePath.string() + " is not a regular file");
    }

    std::ifstream stream;

    try
    {
        streamSize = 0;

        stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        stream.open(filePath, std::ios::in | std::ios::binary);

        stream.seekg(0, stream.end);
        streamSize = stream.tellg();
        stream.seekg(0, stream.beg);
    }
    catch (std::exception const& error)
    {
        streamSize = 0;

        throw MyError(ErrorPrefix + filePath.string() + " opening failed. Reason: " + error.what());
    }

    return stream;
}

[[noreturn]] void ThrowReadChunkError(std::filesystem::path const& filePath,
    std::streamsize const startPosition,
    std::streamsize const endPosition,
    std::string const& message)
{
    throw MyError(ErrorPrefix + filePath.string() + " read chunk [" + std::to_string(startPosition)
        + ", " + std::to_string(endPosition) + ") failed. Reason: " + message);
}

} // namespace

FileReader::FileReader(std::filesystem::path filePath, std::size_t blockSizeToReadInBytes) :
    m_filePath(std::move(filePath)),
    m_blockSizeToReadInBytes(blockSizeToReadInBytes)
{
    if (blockSizeToReadInBytes > BufferMaxSize)
    {
        throw MyError(ErrorPrefix + filePath.string() + " reading failed. Reason: requested block size "
            + std::to_string(blockSizeToReadInBytes) + " is greater than supported max value "
            + std::to_string(BufferMaxSize));
    }

    m_stream = CreateStream(m_filePath, m_streamSizeInBytes);
}

std::optional<BlockContents> FileReader::ReadNext()
{
    std::lock_guard<std::mutex> const lock{ m_streamReadMutex };

    auto const endOfBlock = static_cast<std::streamsize>(m_currentPositionInBytes + m_blockSizeToReadInBytes);
    auto const sizeToRead = std::min(m_streamSizeInBytes, endOfBlock) - m_currentPositionInBytes;
    if (!sizeToRead)
    {
        return std::nullopt;
    }

    BlockContents block;
    block.Position = m_countOfReadBlocks;

    try
    {
        block.Contents.resize(static_cast<std::size_t>(sizeToRead), '0');
        m_stream.read(block.Contents.data(), sizeToRead);
    }
    catch (std::exception const& error)
    {
        m_currentPositionInBytes = m_streamSizeInBytes = m_countOfReadBlocks = 0;
        m_stream = {};
        ThrowReadChunkError(m_filePath, m_currentPositionInBytes, endOfBlock, error.what());
    }

    m_countOfReadBlocks++;
    m_currentPositionInBytes += sizeToRead;
    return block;
}

void FileReader::Close()
{
    std::lock_guard<std::mutex> const lock{ m_streamReadMutex };

    m_currentPositionInBytes = m_streamSizeInBytes = m_countOfReadBlocks = 0;
    m_stream = {};
}
