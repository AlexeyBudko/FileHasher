#include "FileHashCalculator.h"

#include "Block.h"
#include "FileHashBufferedPrinter.h"
#include "FileReader.h"
#include "IBlockHashCalculationNotify.h"

#include <boost/crc.hpp>
#include <boost/thread/thread.hpp>

#include <cstdint>


namespace
{

class CalculateHashThreadWorker
{
public:
    CalculateHashThreadWorker(FileReader& fileReader, IBlockHashCalculationNotify& notify) :
        m_fileReader(fileReader),
        m_notify(notify)
    {
    }

    void operator()()
    {
        while (true)
        {
            try
            {
                m_notify.OnBeforeBlockHashCalculation();
                auto const maybeBlockOfFile = m_fileReader.ReadNext();
                if (!maybeBlockOfFile.has_value())
                {
                    return;
                }

                BlockHash blockHash;
                blockHash.Position = maybeBlockOfFile->Position;

                boost::crc_32_type result;
                result.process_bytes(maybeBlockOfFile->Contents.data(), maybeBlockOfFile->Contents.size());
                blockHash.Value = result.checksum();

                m_notify.OnBlockHashCalculationCompleted(blockHash);
            }
            catch (std::exception const&)
            {
                m_fileReader.Close();
                HandleException(std::current_exception());
            }
        }
    }

private:
    void HandleException(std::exception_ptr exception)
    {
        try
        {
            m_notify.OnBlockHashCalculationFailed(exception);
        }
        catch (...)
        {
        }
    }

private:
    FileReader& m_fileReader;
    IBlockHashCalculationNotify& m_notify;
};

} // namespace


FileHashCalculator::FileHashCalculator(FileReader& fileReader, FileHashPrinter& printer) :
    m_fileReader(fileReader),
    m_printer(printer)
{
}

void FileHashCalculator::CalculateHash()
{
    FileHashBufferedPrinter bufferedPrinter{ m_printer };

    boost::thread_group threadGroup;
    for (std::size_t i = 0; i < boost::thread::hardware_concurrency(); i++)
    {
        threadGroup.create_thread(CalculateHashThreadWorker(m_fileReader, bufferedPrinter));
    }

    threadGroup.join_all();

    auto exceptionFromController = bufferedPrinter.TakeAwayLatestException();
    if (exceptionFromController)
    {
        std::rethrow_exception(exceptionFromController);
    }

    bufferedPrinter.FlushBufferedHashes();
    m_printer.FlushAllHashes();
}
