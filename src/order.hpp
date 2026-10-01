#pragma once
#include "types.hpp"

struct Order {
    OrderId order_id;
    Price price;
    PoolIndex prev;
    PoolIndex next;
    Qty qty;
    Side side;
};
