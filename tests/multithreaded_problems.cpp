#include <catch.hpp>

#include "PathSpace.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <chrono>

using namespace FSNG;

inline auto sleepThread(float from=0.05, float to=0.3) {
    auto const secondInNanoseconds = 1000000000;
    auto const number = random_number(secondInNanoseconds*from, secondInNanoseconds*to);
    std::this_thread::sleep_for(std::chrono::nanoseconds(number));
}

TEST_CASE("DiningPhilosophers") {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;
    auto think = [](auto i){LOG("Philosopher {} thinking", i)sleepThread();LOG("Philosopher {} woke up", i)};
    auto eat   = [](auto i){LOG("Philosopher {} eating", i)sleepThread();LOG("Philosopher {} finished eating", i)};
    
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        space.insert("/chopstick/"+std::to_string(i), 0);
        space.insert("/philosopher", [&space, i, think, eat, totalLoops]()->Coroutine{
            auto const leftChopstickName  = "/chopstick/"+std::to_string(i);
            auto const rightChopstickName = "/chopstick/"+std::to_string((i+1)%numberOfPhilosophers);
            int currentLoop = 0;
            while(currentLoop<totalLoops) {
                think(i);
                auto const roomTicket     = space.grabBlock<int>("/room_ticket");
                auto const leftChopStick  = space.grabBlock<int>(leftChopstickName);
                auto const rightChopStick = space.grabBlock<int>(rightChopstickName);
                eat(i);
                space.insert(leftChopstickName, leftChopStick);
                space.insert(rightChopstickName, rightChopStick);
                space.insert("/room_ticket", i);
                currentLoop++;
            }
            co_return 123;
        });
        if(i<(numberOfPhilosophers-1)) {
            space.insert("/room_ticket", i);
        }
    }
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        int const ret = space.grabBlock<int>("/philosopher");
        REQUIRE(ret==123);
    }
}

TEST_CASE("DiningPhilosophers Simple") {
    for(int x = 0; x < 2100; ++x) {
        auto space = PathSpaceTE::Create<PathSpace>();
        int const numberOfPhilosophers = 5;
        int const totalLoops = 10;

        for(int i = 0; i < numberOfPhilosophers; ++i) {
            LOG("Philosopher adding: {}", i);
            space.insert("/philosopher", [&space, i, totalLoops]()->Coroutine{
                LOG("Philosopher starting: {}", i);
                co_return 123;
            });
        }
        for(int i = 0; i < numberOfPhilosophers; ++i) {
            LOG("Philosopher grabbing: {}", i);
            int const ret = space.grabBlock<int>("/philosopher");
            REQUIRE(ret==123);
            LOG("Philosopher grabbed: {} {}", i, ret);
        }
        SetupHTMLLog();
    }
}