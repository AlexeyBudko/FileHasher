#pragma once

#include <cstdint>
#include <vector>


using Buffer = std::vector<char>;
using OrdinalNumber = std::uint64_t;
using Hash = std::size_t;

struct BlockContents
{
    Buffer Contents;
    OrdinalNumber Position = 0;
};

struct BlockHash
{
    Hash Value = 0;
    OrdinalNumber Position = 0;
};
