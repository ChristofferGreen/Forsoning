#include <doctest.h>

#include "PathSpace.hpp"

#include <chrono>

using namespace FSNG;

TEST_CASE("DiningPhilosophers") {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;
    auto thinkEatDone = [](int const &i, std::string const &type, PathSpaceTE &space){
        if(type=="think"||type=="eat") {
            auto const number = random_number(5000000*10, 30000000*10);
            std::this_thread::sleep_for(std::chrono::nanoseconds(number));
        }
        printm(fmt::format("Philosopher: {} is doing: {} {}", i, type, space.toJSON().dump()));
    };
    
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        space.insert("/chopstick/"+std::to_string(i), i);
        space.insert("/philosopher", [&space, i, thinkEatDone, totalLoops]()->Coroutine{
            int currentLoop = 0;
            while(currentLoop++<totalLoops) {
                thinkEatDone(i, "think", space);
                auto const roomTicket = space.grabBlock<int>("/room_ticket");
                thinkEatDone(i, "leftChopStick", space);
                auto const leftChopStick = space.grabBlock<int>("/chopstick/"+std::to_string(i));
                thinkEatDone(i, "rightChopStick", space);
                auto const rightChopStick = space.grabBlock<int>("/chopstick/"+std::to_string((i+1)%numberOfPhilosophers));
                printm(fmt::format("Philosopher: {} got right chopstick {}", i, space.toJSON().dump()));
                thinkEatDone(i, "eat", space);
                space.insert("/chopstick/"+std::to_string(i), leftChopStick);
                thinkEatDone(i, "/chopstick/"+std::to_string((i+1)%numberOfPhilosophers), space);
                space.insert("/chopstick/"+std::to_string((i+1)%numberOfPhilosophers), rightChopStick);
                thinkEatDone(i, "/room_ticket", space);
                space.insert("/room_ticket", i);
            }
            thinkEatDone(i, "done", space);
            co_return 123;
        });
        if(i<(numberOfPhilosophers-1))
            space.insert("/room_ticket", i);
    }
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        int const ret = space.grabBlock<int>("/philosopher");
        CHECK(ret==123);
        std::cout << ret << std::endl;
    }
}