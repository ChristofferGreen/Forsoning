#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Forge Upgradable Mutex") {
    SUBCASE("Basics") {
        UpgradableMutex mutex;
        UnlockedToExclusiveLock upgrade(mutex);
    }
    SUBCASE("One Reader One Writer") {
        bool startSecond = false;
        UpgradableMutex mutex;
        int val = 0;
        std::thread t1([&startSecond, &mutex, &val](){
            UnlockedToExclusiveLock upgrade(mutex);
            startSecond = true;
            val = 3-val;
        });
        std::thread t2([&startSecond, &mutex, &val](){
            while(!startSecond) {}
            UnlockedToExclusiveLock upgrade(mutex);
            val = val+2;
        });
        t1.join();
        t2.join();
        CHECK(val==5);
    }
    SUBCASE("Multiple Writers") {
        for(auto j = 0; j < 100; ++j) {
            UpgradableMutex mutex;
            int val = 0;
            auto l = [&mutex, &val](){
                UnlockedToUpgradedLock lock(mutex);
                auto v = val;
                {
                    UpgradedToExclusiveLock upgrade(mutex);
                    val++;
                }
                CHECK(val==v+1);
            };
            int const numThreads = 30;
            std::vector<std::thread> threads;
            for(auto i = 0; i < numThreads; ++i)
                threads.emplace_back(l);
            for(auto &thread : threads)
                thread.join();
            CHECK(val==numThreads);
        }
    }
}