#include <doctest.h>

#include "PathSpace.hpp"

#include <chrono>

using namespace FSNG;

TEST_CASE("DiningPhilosophers") {
    PathSpaceTE space = PathSpace();
    int const numberOfPhilosophers = 5;
    int const totalLoops = 10;
    auto thinkOrEat = [](int const &i, std::string const &type, PathSpaceTE &space){
        std::this_thread::sleep_for(std::chrono::nanoseconds(random_number(50000, 300000)));
        space.grabBlock<bool>("/print_mutex");
        std::cout << "Philosopher: " << i << " is doing: " << type << std::endl;
        space.insert("/print_mutex", true);
    };
    for(int i = 0; i < numberOfPhilosophers; ++i) {
        space.insert("/chopstick/"+std::to_string(i), i);
        space.insert("/philosopher", [&space, &i, &thinkOrEat]()->Coroutine{
            int currentLoop = 0;
            while(currentLoop++<totalLoops) {
                thinkOrEat(i, "think", space);
                auto const roomTicket = space.grabBlock<int>("/room_ticket");
                auto const leftChopStick = space.grabBlock<int>("/chopstick/"+std::to_string(i));
                auto const rightChopStick = space.grabBlock<int>("/chopstick/"+std::to_string((i+1)%numberOfPhilosophers));
                thinkOrEat(i, "eat", space);
                space.insert("/chopstick/"+std::to_string(i), leftChopStick);
                space.insert("/chopstick/"+std::to_string((i+1)%numberOfPhilosophers), rightChopStick);
            }
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