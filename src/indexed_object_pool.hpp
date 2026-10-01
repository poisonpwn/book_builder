#pragma once
#include "object_pool.hpp"

template <typename T, std::size_t Capacity>
class IndexedObjectPool : private ObjectPool<T, Capacity> {
  private:
    using Base = ObjectPool<T, Capacity>;

  public:
    using Base::acquire;
    using Base::release;

    T& operator[](PoolIndex idx) {
        assert(this->owns(idx) && this->is_occupied.test(idx));
        return reinterpret_cast<T&>(this->items[idx].item);
    }

    const T& operator[](PoolIndex idx) const {
        assert(this->owns(idx) && this->is_occupied.test(idx));
        return reinterpret_cast<const T&>(this->items[idx].item);
    }
};
