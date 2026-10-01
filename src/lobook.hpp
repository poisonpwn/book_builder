#pragma once

#include <map>

#include "indexed_object_pool.hpp"
#include "order.hpp"
#include "pool_allocator.hpp"
#include "price_level.hpp"
#include "types.hpp"

constexpr uint32_t LadderCapacity = 1 << 19;
constexpr uint32_t OrderPoolCapacity = 1 << 15;
using MapPoolAlloc =
    PoolAllocator<std::pair<const Price, PriceLevel>, LadderCapacity>;
using BidLadder =
    std::map<Price, PriceLevel, std::greater<Price>, MapPoolAlloc>;
using AskLadder = std::map<Price, PriceLevel, std::less<Price>, MapPoolAlloc>;
using OrderPool = IndexedObjectPool<Order, OrderPoolCapacity>;

class LimitOrderBook {
    AskLadder asks;
    BidLadder bids;
    OrderPool& orders;

  public:
    explicit LimitOrderBook(OrderPool& orders)
        : asks(), bids(), orders(orders) {}

    void clear();
    void applyAdd(OrderId order_id, Price price, Qty qty, Side side);
    // return val indicates success
    bool applyCancel(OrderId order_id, Qty cancelled_qty);
    bool applyModify(OrderId order_id, Price new_price, Qty new_qty);
    bool applyFill(OrderId order_id, Qty filled_qty);
};
