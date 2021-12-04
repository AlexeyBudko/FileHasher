#include "FileHashBufferedPrinter.h"

#include <algorithm>
#include <iostream>


namespace
{

auto constexpr BufferMaxSize = 1000;

void SortHashesByBlockPosition(std::vector<BlockHash>& hashes)
{
    std::sort(hashes.begin(), hashes.end(), [](BlockHash const& l, BlockHash const& r)
    {
        return l.Position < r.Position;
    });
}

std::vector<BlockHash> RemoveEmtyOptionalsFromHashes(std::vector<std::optional<BlockHash>> hashes)
{
    auto const removedHashesIt = std::remove_if(hashes.begin(), hashes.end(), [](std::optional<BlockHash> const& maybeBlockHash)
    {
        return !maybeBlockHash.has_value();
    });

    hashes.erase(removedHashesIt, hashes.end());

    std::vector<BlockHash> result;
    result.reserve(hashes.size());

    for (auto&& blockHash : hashes)
    {
        result.push_back(std::move(*blockHash));
    }

    return result;
}

} // namespace


FileHashBufferedPrinter::FileHashBufferedPrinter(FileHashPrinter& printer) :
    m_printer(printer)
{
    m_hashes.reserve(BufferMaxSize);
    m_amountOfNotReservedSpaceInBuffer = BufferMaxSize;
}

void FileHashBufferedPrinter::OnBeforeBlockHashCalculation()
{
    std::unique_lock<std::mutex> lock{ m_mutex };
    m_condition.wait(lock, [this]() { return HasNotReservedSpaceInBuffer(); });
    --m_amountOfNotReservedSpaceInBuffer;
}

void FileHashBufferedPrinter::OnBlockHashCalculationCompleted(BlockHash const& blockHash)
{
    AddHashToBufferAndTryFlush(blockHash);
}

void FileHashBufferedPrinter::OnBlockHashCalculationFailed(std::exception_ptr exception)
{
    std::unique_lock<std::mutex> lock{ m_mutex };
    m_latestException = exception;
    lock.unlock();

    auto constexpr emptyBlockHashToAdd = std::nullopt;
    AddHashToBufferAndTryFlush(emptyBlockHashToAdd);
}

void FileHashBufferedPrinter::FlushBufferedHashes()
{
    std::unique_lock<std::mutex> lock{ m_mutex };

    std::vector<std::optional<BlockHash>> calculatedHashes;
    std::swap(calculatedHashes, m_hashes);

    m_amountOfNotReservedSpaceInBuffer = BufferMaxSize;
    lock.unlock();

    m_condition.notify_all();

    FlushHashes(std::move(calculatedHashes));
}

std::exception_ptr FileHashBufferedPrinter::TakeAwayLatestException()
{
    std::lock_guard<std::mutex> const lock{ m_mutex };

    std::exception_ptr exceptionForTakeAway;
    std::swap(exceptionForTakeAway, m_latestException);

    return exceptionForTakeAway;
}

void FileHashBufferedPrinter::AddHashToBufferAndTryFlush(std::optional<BlockHash> maybeBlockHash)
{
    std::unique_lock<std::mutex> lock{ m_mutex };
    m_hashes.push_back(std::move(maybeBlockHash));
    if (m_hashes.size() < BufferMaxSize)
    {
        return;
    }

    std::vector<std::optional<BlockHash>> calculatedHashes;
    std::swap(calculatedHashes, m_hashes);

    m_amountOfNotReservedSpaceInBuffer = BufferMaxSize;
    lock.unlock();

    m_condition.notify_all();

    try
    {
        FlushHashes(std::move(calculatedHashes));
    }
    catch (std::exception const&)
    {
        std::lock_guard<std::mutex> const lock{ m_mutex };
        m_latestException = std::current_exception();

        throw;
    }
}

bool FileHashBufferedPrinter::HasNotReservedSpaceInBuffer() const
{
    return m_amountOfNotReservedSpaceInBuffer > 0;
}

void FileHashBufferedPrinter::FlushHashes(std::vector<std::optional<BlockHash>> hashes)
{
    auto hashesToFlush = RemoveEmtyOptionalsFromHashes(std::move(hashes));
    SortHashesByBlockPosition(hashesToFlush);
    for (auto const& blockHash : hashesToFlush)
    {
        m_printer.PrintHash(blockHash);
    }
}
