#pragma once
#include "types.hpp"

struct PriceLevel {
    Qty qty{0};
    OrderCount count{0};
    PoolIndex head{InvalidIdx};
    PoolIndex tail{InvalidIdx};
};
