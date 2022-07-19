#include <doctest.h>

#include "PathSpace.hpp"

#include <chrono>

using namespace FSNG;

auto sleepThread() {
    auto const number = random_number(5000000*10, 30000000*10);
    std::this_thread::sleep_for(std::chrono::nanoseconds(number));
}

/*TEST_CASE("DiningPhilosophers" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;
    auto think = [](){sleepThread();};
    auto eat   = [](){sleepThread();};
    
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        space.insert("/chopstick/"+std::to_string(i), 0);
        space.insert("/philosopher", [&space, i, think, eat, totalLoops]()->Coroutine{
            std::cout << "Philosopher starting: " << i << std::endl;*/
            /*auto const leftChopstickName  = "/chopstick/"+std::to_string(i);
            auto const rightChopstickName = "/chopstick/"+std::to_string((i+1)%numberOfPhilosophers);
            int currentLoop = 0;
            while(currentLoop++<totalLoops) {
                think();
                auto const roomTicket     = space.grabBlock<int>("/room_ticket");
                auto const leftChopStick  = space.grabBlock<int>(leftChopstickName);
                auto const rightChopStick = space.grabBlock<int>(rightChopstickName);
                eat();
                space.insert(leftChopstickName, leftChopStick);
                space.insert(rightChopstickName, rightChopStick);
                space.insert("/room_ticket", i);
                currentLoop++;
            }*/
            /*std::cout << "Philosopher ending: " << i << std::endl;
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

TEST_CASE("DP" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;

    for(int i = 0; i < numberOfPhilosophers; ++i) {
        std::cout << "Philosopher adding: " << i << std::endl;
        space.insert("/philosopher", [&space, i, totalLoops]()->Coroutine{
            std::cout << "Philosopher starting: " << i << std::endl;
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
}*/