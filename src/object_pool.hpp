#pragma once
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <new>

#include "types.hpp"

template <typename T, std::size_t Capacity>
class ObjectPool {
    static_assert(
        Capacity < InvalidIdx,
        "maximum valid index must be less than invalid index sentinel value");

    static_assert(Capacity > 0, "capacity must be atleast 1");

  protected:
    union Slot {
        alignas(T) std::byte item[sizeof(T)];
        PoolIndex next;
    };

    std::array<Slot, Capacity> items{};
    std::bitset<Capacity> is_occupied{};
    PoolIndex free_list_head{0};

  public:
    ObjectPool() {
        for (PoolIndex i = 0; i + 1 < Capacity; i++) {
            items[i].next = i + 1;
        }
        items[Capacity - 1].next = InvalidIdx;
    }

    T* allocate() {
        auto idx = acquire();
        return reinterpret_cast<T*>(convert_idx_to_ptr(idx));
    }

    void deallocate(T* ptr) {
        auto idx = convert_ptr_to_idx(reinterpret_cast<Slot*>(ptr));
        release(idx);
    }

    PoolIndex acquire() {
        if (free_list_head == InvalidIdx) throw std::bad_alloc();
        auto curr = free_list_head;
        free_list_head = items[free_list_head].next;
        assert(!is_occupied.test(curr));
        is_occupied.set(curr, true);
        return curr;
    }

    void release(PoolIndex idx) {
        assert(owns(idx));
        assert(is_occupied.test(idx));
        is_occupied.set(idx, false);
        items[idx].next = free_list_head;
        free_list_head = idx;
    }

    inline bool owns(const T* ptr) {
        return ptr > items.data() && ptr < items.data() + Capacity;
    }

    inline constexpr bool owns(PoolIndex idx) const { return idx < Capacity; }

    inline Slot* convert_idx_to_ptr(PoolIndex idx) {
        return items.data() + idx;
    }

    inline const Slot* convert_idx_to_ptr(PoolIndex idx) const {
        return items.data() + idx;
    }

    inline PoolIndex convert_ptr_to_idx(const Slot* ptr) const {
        return ptr - items.data();
    }
};
