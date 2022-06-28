#include <doctest.h>

#include "amutex/shared_atomic_mutex.h"
#include "amutex/upgradable_lock.h"

TEST_CASE("CPP Mutex") {
    SUBCASE("Upgradable") {
        atomic_mutex::shared_atomic_mutex m;
        atomic_mutex::upgradable_lock lock(m);
        lock.upgrade();
    }
}