#pragma once

#include "Block.h"
#include "FileHashPrinter.h"
#include "IBlockHashCalculationNotify.h"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>


class FileHashBufferedPrinter : public IBlockHashCalculationNotify
{
public:
    FileHashBufferedPrinter(FileHashPrinter& printer);

public:
    void OnBeforeBlockHashCalculation() override;
    void OnBlockHashCalculationCompleted(BlockHash const& blockHash) override;
    void OnBlockHashCalculationFailed(std::exception_ptr exception) override;

    void FlushBufferedHashes();
    std::exception_ptr TakeAwayLatestException();

private:
    void AddHashToBufferAndTryFlush(std::optional<BlockHash> maybeBlockHash);
    bool HasNotReservedSpaceInBuffer() const;
    void FlushHashes(std::vector<std::optional<BlockHash>> hashesToFlush);

private:
    FileHashPrinter& m_printer;

    std::size_t m_amountOfNotReservedSpaceInBuffer = 0;
    std::vector<std::optional<BlockHash>> m_hashes;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::exception_ptr m_latestException;
};
