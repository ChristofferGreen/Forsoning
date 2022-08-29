#include <catch.hpp>

#include "PathSpace.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <chrono>

using namespace FSNG;

auto sleepThread() {
    auto const number = random_number(5000000*10, 30000000*10);
    std::this_thread::sleep_for(std::chrono::nanoseconds(number));
}

TEST_CASE("DiningPhilosophers") {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;
    auto think = [](auto i){LOG("Philosopher {} thinking", i)sleepThread();LOG("Philosopher {} woke up", i)};
    auto eat   = [](auto i){LOG("Philosopher {} eating", i)sleepThread();LOG("Philosopher {} finished eating", i)};
    
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        LOG("Inserting chopstick {}", i)
        space.insert("/chopstick/"+std::to_string(i), 0);
        LOG("Inserting philosopher {}", i)
        space.insert("/philosopher", [&space, i, think, eat, totalLoops]()->Coroutine{
            LOG("Philosopher starting:  {}", i)
            auto const leftChopstickName  = "/chopstick/"+std::to_string(i);
            auto const rightChopstickName = "/chopstick/"+std::to_string((i+1)%numberOfPhilosophers);
            int currentLoop = 0;
            while(currentLoop<totalLoops) {
                think(i);
                LOG("Philosopher {} grabs room ticket", i)
                auto const roomTicket     = space.grabBlock<int>("/room_ticket");
                LOG("Philosopher {} grabbing left chopstick", i)
                auto const leftChopStick  = space.grabBlock<int>(leftChopstickName);
                LOG("Philosopher {} grabbing right chopstick", i)
                auto const rightChopStick = space.grabBlock<int>(rightChopstickName);
                LOG("Philosopher {} has both chopstick", i)
                eat(i);
                LOG("Philosopher {} inserts left chopstick", i)
                space.insert(leftChopstickName, leftChopStick);
                LOG("Philosopher {} inserts right chopstick", i)
                space.insert(rightChopstickName, rightChopStick);
                LOG("Philosopher {} inserts room ticket", i)
                space.insert("/room_ticket", i);
                LOG("Philosopher {} loops around current loop {} total loops {}", i, currentLoop, totalLoops)
                currentLoop++;
            }
            LOG("Philosopher ending: {}", i)
            co_return 123;
        });
        if(i<(numberOfPhilosophers-1)) {
            LOG("Inserting room ticket {}", i)
            space.insert("/room_ticket", i);
        }
    }
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        LOG("Grabbing philosopher {}", i)
        int const ret = space.grabBlock<int>("/philosopher");
        REQUIRE(ret==123);
        LOG("Grabbed philosopher {} with result {}", i, ret)
    }
}

TEST_CASE("DP") {
    for(int x = 0; x < 2100; ++x) {
        PathSpaceTE space = PathSpace();
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
        }
        try {
            spdlog::drop("file");
            auto logger = spdlog::basic_logger_mt("file", "logs/basic-log.txt", true);
            logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        } catch (const spdlog::spdlog_ex &ex) {}
    }
}