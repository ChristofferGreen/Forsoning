#include <doctest.h>

#include "PathSpace.hpp"

#include <mutex>

using namespace FSNG;

TEST_CASE("Forge") {
    SUBCASE("Eschelon") {
        Eschelon queue;
        queue.add([]()->Coroutine{co_yield 0;}, [](Data const &data){});
        auto const task = queue.popWait();
        CHECK(task.has_value()==true);
        CHECK(task.value().ticket==FirstTicket);
    }

    SUBCASE("Forge") {
        Forge forge;
        auto res = 0;
        bool hasRun = false;
        auto const ticket = forge.add([]()->Coroutine{co_yield 345;}, [&res, &hasRun](Data const &data){
            LOG("Forge test coro start");
            CHECK(data.is<int>()==true);
            res=data.as<int>();
            CHECK(res==345);
            hasRun=true;
            LOG("Forge test coro end");
        });
        forge.wait(ticket);
        CHECK(hasRun==true);
        CHECK(res==345);
    }

    SUBCASE("Forge Multiple Tasks") {
        Forge forge;
        std::set<int> s;
        std::vector<int> tickets;
        std::shared_mutex mutex;
        for(auto i = 0; i < 128; ++i) {
            tickets.push_back(forge.add([i]()->Coroutine{co_yield i;}, [i, &s, &mutex](Data const &data){
                CHECK(data.is<int>()==true);
                auto writeLock = std::unique_lock<std::shared_mutex>(mutex);
                s.insert(i);
            }));
        }
        for(auto const &ticket : tickets)
            forge.wait(ticket);
        CHECK(s.size()==128);
        for(auto i = 0; i < 128; ++i)
            CHECK(s.contains(i));
    }
}