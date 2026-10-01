// Manual test harness, no framework dependency (no build system yet either).
// Build/run:
//   g++ -std=c++17 -Wall -Wextra -g -o /tmp/test_object_pool test_object_pool.cpp
//   /tmp/test_object_pool
#include "../src/object_pool.h"
#include "../src/order.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <new>
#include <set>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr std::size_t kCapacity = 8;

void test_fresh_pool_exhausts_at_capacity() {
    ObjectPool<Order, kCapacity> pool;
    std::set<ObjectPool<Order, kCapacity>::Index> seen;

    for (std::size_t i = 0; i < kCapacity; i++) {
        auto idx = pool.acquire();
        assert(idx >= 0 && static_cast<std::size_t>(idx) < kCapacity);
        assert(seen.insert(idx).second);  // every index handed out exactly once
    }

    bool threw = false;
    try {
        pool.acquire();
    } catch (const std::bad_alloc&) {
        threw = true;
    }
    assert(threw);

    printf("PASS: test_fresh_pool_exhausts_at_capacity\n");
}

void test_read_write_roundtrip() {
    ObjectPool<Order, kCapacity> pool;
    auto idx = pool.acquire();
    pool[idx].order_id = 123;
    pool[idx].qty = 42;

    assert(pool[idx].order_id == 123);
    assert(pool[idx].qty == 42);

    printf("PASS: test_read_write_roundtrip\n");
}

void test_owns() {
    ObjectPool<Order, kCapacity> pool;
    Order* in_pool = pool.allocate();
    Order stack_order{};

    assert(pool.owns(in_pool));
    assert(!pool.owns(&stack_order));

    printf("PASS: test_owns\n");
}

void test_release_and_reacquire_is_lifo() {
    ObjectPool<Order, kCapacity> pool;

    auto a = pool.acquire();
    auto b = pool.acquire();
    (void)a;

    pool.release(b);
    auto reacquired = pool.acquire();
    assert(reacquired == b);  // free list is LIFO: last released comes back first

    printf("PASS: test_release_and_reacquire_is_lifo\n");
}

void test_deallocate_via_pointer() {
    ObjectPool<Order, kCapacity> pool;
    Order* p = pool.allocate();
    pool.deallocate(p);

    // slot should be reusable after deallocate()
    Order* p2 = pool.allocate();
    assert(p2 == p);

    printf("PASS: test_deallocate_via_pointer\n");
}

void test_slot_zero_is_releasable() {
    // Regression check: owns()/release() previously rejected index 0.
    ObjectPool<Order, kCapacity> pool;
    Order* first = pool.allocate();  // first slot handed out is index 0
    pool.deallocate(first);

    printf("PASS: test_slot_zero_is_releasable\n");
}

void test_alignment() {
    ObjectPool<Order, kCapacity> pool;
    for (std::size_t i = 0; i < kCapacity; i++) {
        Order* p = pool.allocate();
        auto addr = reinterpret_cast<std::uintptr_t>(p);
        assert(addr % alignof(Order) == 0);
    }

    printf("PASS: test_alignment\n");
}

void test_full_exhaust_and_refill_cycle() {
    ObjectPool<Order, kCapacity> pool;
    Order* ptrs[kCapacity];

    for (std::size_t i = 0; i < kCapacity; i++) ptrs[i] = pool.allocate();
    for (std::size_t i = 0; i < kCapacity; i++) pool.deallocate(ptrs[i]);

    std::set<Order*> reacquired;
    for (std::size_t i = 0; i < kCapacity; i++) {
        Order* p = pool.allocate();
        assert(reacquired.insert(p).second);
    }

    printf("PASS: test_full_exhaust_and_refill_cycle\n");
}

void test_double_release_aborts() {
    // The occupancy-bitset guard in release() calls assert(), which terminates
    // the process (SIGABRT). Run the offending sequence in a child process so
    // a triggered guard doesn't take down the rest of the test suite with it.
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        ObjectPool<Order, kCapacity> pool;
        auto idx = pool.acquire();
        pool.release(idx);
        pool.release(idx);  // double release: should abort via the guard
        _exit(0);            // only reached if the guard failed to fire
    }

    int status = 0;
    waitpid(pid, &status, 0);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);

    printf("PASS: test_double_release_aborts\n");
}

}  // namespace

int main() {
    test_fresh_pool_exhausts_at_capacity();
    test_read_write_roundtrip();
    test_owns();
    test_release_and_reacquire_is_lifo();
    test_deallocate_via_pointer();
    test_slot_zero_is_releasable();
    test_alignment();
    test_full_exhaust_and_refill_cycle();
    test_double_release_aborts();

    printf("\nall tests passed\n");
    return 0;
}
