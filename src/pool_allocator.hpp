#pragma once
#include <cstddef>

#include "object_pool.hpp"

// if need be, scope is meant to be used to differentiate pools within the same
// T and Cap
template <typename T, std::size_t Capacity, typename Scope = struct Default>
class PoolAllocator {
  public:
    using value_type = T;

    PoolAllocator() noexcept = default;

    // need to define this explicitly, as Capacity being a non typename
    // parameter, allocator traits from synthesizing it.
    template <typename U>
    PoolAllocator(const PoolAllocator<U, Capacity, Scope>&) noexcept {}

    template <typename U>
    struct rebind {
        using other = PoolAllocator<U, Capacity, Scope>;
    };

    [[nodiscard]] T* allocate(std::size_t n) {
        assert(n == 1);
        return pool().allocate();
    }

    void deallocate(T* ptr, std::size_t n) noexcept {
        assert(n == 1);
        pool().deallocate(ptr);
    }

    friend bool operator==(PoolAllocator<T, Capacity, Scope>,
                           PoolAllocator<T, Capacity, Scope>) noexcept {
        return true;
    };

    friend bool operator!=(PoolAllocator<T, Capacity, Scope>,
                           PoolAllocator<T, Capacity, Scope>) noexcept {
        return false;
    };

  private:
    // make it a Myers' Singleton.
    static ObjectPool<T, Capacity>& pool() {
        static ObjectPool<T, Capacity> inst;
        return inst;
    }
};
