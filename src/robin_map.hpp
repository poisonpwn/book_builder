#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

template <typename K, typename V, std::size_t Capacity, typename Hash>
class RobinMap {
    static_assert(std::is_trivially_copyable_v<K> &&
                  std::is_trivially_copyable_v<V>);

    // making it a power of two makes it so that to modulo by the size you can
    // just do hash & (Capacity - 1) instead of hash % Capacity
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "map capacity must be a power of two");
    static constexpr std::size_t size_mask = Capacity - 1;

    using Dist = int32_t;
    using Index = std::size_t;
    struct Slot {
        K key;
        V value;
        Dist probe_dist{-1};
    };
    std::array<Slot, Capacity> slots;
    std::size_t size_{0};

    static constexpr Index InvalidIdx = static_cast<Index>(-1);

    static inline constexpr Index get_ideal_slot(const K& key) {
        return Hash{}(key)&size_mask;
    }
    static inline constexpr bool reached_max_dist(Dist dist) {
        return dist >= static_cast<Dist>(Capacity);
    }

    Index find_index(const K& key) const {
        Dist dist = 0;
        for (Index i = get_ideal_slot(key); !reached_max_dist(dist);
             i = (i + 1) & size_mask, dist++) {
            const Slot& slot = slots[i];
            if (slot.probe_dist == -1) return -1;
            if (slot.probe_dist < dist) return -1;
            if (slot.key == key) {
                return i;
            }
        }

        return InvalidIdx;
    }

  public:
    bool contains(const K& key) const { return find_index(key) != InvalidIdx; }

    inline std::size_t size() const { return size_; }

    bool insert(K key, V value) {
        if (size_ == Capacity) {
            Index idx = find_index(key);
            if (idx == InvalidIdx) throw std::bad_alloc();
            slots[idx].value = value;
            return false;
        }

        Dist dist = 0;
        for (Index i = get_ideal_slot(key);; i = (i + 1) & size_mask, dist++) {
            Slot& slot = slots[i];
            if (slot.probe_dist == -1) {
                slot = {key, value, dist};
                size_++;
                return true;
            } else if (slot.probe_dist < dist) {
                std::swap(key, slot.key);
                std::swap(value, slot.value);
                std::swap(dist, slot.probe_dist);
            } else if (slot.key == key) {
                slot.value = value;
                return false;
            }
        }
    };

    std::pair<bool, V> find(const K& key) const {
        Index idx = find_index(key);
        if (idx == InvalidIdx) {
            return {false, V{}};
        } else {
            return {true, slots[idx].value};
        }
    }

    bool erase(const K& key) {
        Index curr = find_index(key);
        if (curr == InvalidIdx) return false;

        for (Index next = (curr + 1) & size_mask;;
             curr = next, next = (next + 1) & size_mask) {
            Slot& slot = slots[next];
            if (slot.probe_dist <= 0) break;
            slots[curr] = slots[next];
            slots[curr].probe_dist--;
        }
        slots[curr].probe_dist = -1;
        size_--;

        return true;
    }
};
