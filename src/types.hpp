#pragma once

#include <cstdint>
#include <limits>
using PoolIndex = uint32_t;
constexpr PoolIndex InvalidIdx = std::numeric_limits<PoolIndex>::max();

using OrderId = uint64_t;
using OrderCount = uint32_t;
using Price = int32_t;
using Qty = uint32_t;

constexpr uint32_t TICK_RESOLUTION = 10000;

enum class Side : uint8_t {
    Ask,
    Bid,
};

enum class Action : uint8_t {
    Add,
    Cancel,
    Modify,
    Fill,
    Clear,
};
