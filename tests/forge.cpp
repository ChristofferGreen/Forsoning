#include <doctest.h>

#include "PathSpace.hpp"

#include <mutex>

using namespace FSNG;

TEST_CASE("Forge") {
    SUBCASE("Eschelon") {
        Eschelon queue;
        auto ticket = queue.newTicket();
        queue.add(ticket, []()->Coroutine{co_return 0;}, [](Data const &data, Ticket const &ticket){});
        auto const task = queue.popWait();
        CHECK(task.has_value()==true);
        CHECK(task.value().ticket==FirstTicket);
    }

    SUBCASE("Coroutine") {
        for(int i = 0; i < 1; ++i) {
            Forge forge;
            auto res = 0;
            bool hasRun = false;
            auto const ticket = forge.add([]()->Coroutine{co_return 345;}, [&res, &hasRun](Data const &data, Ticket const &ticket){
                CHECK(data.is<int>()==true);
                res=data.as<int>();
                CHECK(res==345);
                hasRun=true;
            });
            forge.wait(ticket);
            CHECK(hasRun==true);
            CHECK(res==345);
            LOG("Forge loop nbr: {}", i)
        }
    }

    SUBCASE("Multiple Tasks") {
        Forge forge;
        std::set<int> s;
        std::vector<int> tickets;
        std::shared_mutex mutex;
        for(auto i = 0; i < 128; ++i) {
            tickets.push_back(forge.add([i]()->Coroutine{co_return i;}, [i, &s, &mutex](Data const &data, Ticket const &ticket){
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