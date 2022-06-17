#include <doctest.h>

#include "PathSpace.hpp"

#include <mutex>

using namespace FSNG;

TEST_CASE("Forge") {
    SUBCASE("Eschelon") {
        Eschelon queue;
        queue.add([]()->Coroutine{co_return 0;}, [](Data const &data){});
        auto const task = queue.popWait();
        CHECK(task.has_value()==true);
        CHECK(task.value().ticket==FirstTicket);
    }

    SUBCASE("Forge") {
        Forge forge;
        auto res = 0;
        auto const ticket = forge.add([]()->Coroutine{co_return 345;}, [&res](Data const &data){
            CHECK(data.is<int>()==true);
            res=data.as<int>();
        });
        forge.wait(ticket);
        CHECK(res==345);
    }

    SUBCASE("Forge Multiple Tasks") {
        Forge forge;
        std::set<int> s;
        std::vector<int> tickets;
        std::shared_mutex mutex;
        for(auto i = 0; i < 128; ++i) {
            tickets.push_back(forge.add([i]()->Coroutine{co_return i;}, [i, &s, &mutex](Data const &data){
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