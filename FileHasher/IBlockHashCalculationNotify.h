#pragma once

#include "Block.h"

#include <cstdint>
#include <exception>


class IBlockHashCalculationNotify
{
public:
    virtual ~IBlockHashCalculationNotify() = default;

public:
    virtual void OnBeforeBlockHashCalculation() = 0;
    virtual void OnBlockHashCalculationCompleted(BlockHash const& blockHash) = 0;
    virtual void OnBlockHashCalculationFailed(std::exception_ptr exception) = 0;
};
