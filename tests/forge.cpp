#include <catch.hpp>

#include "PathSpace.hpp"

#include <mutex>

using namespace FSNG;

TEST_CASE("Forge") {
    PathSpaceTE space = PathSpace{};
    SECTION("Eschelon") {
        Eschelon queue;
        auto ticket = queue.newTicket();
        queue.add(ticket, []()->Coroutine{co_return 0;}, [](Data const &data, Ticket const &ticket, PathSpaceTE &space){}, space);
        auto const task = queue.popWait();
        REQUIRE(task.has_value()==true);
        REQUIRE(task.value().ticket==FirstTicket);
    }

    SECTION("Coroutine") {
        for(int i = 0; i < 1; ++i) {
            Forge forge;
            auto res = 0;
            bool hasRun = false;
            auto const ticket = forge.add([]()->Coroutine{
                    co_return 345;
                }, [&res, &hasRun](Data const &data, Ticket const &ticket, PathSpaceTE &space){
                    REQUIRE(data.is<int>()==true);
                    res=data.as<int>();
                    REQUIRE(res==345);
                    hasRun=true;
                }, space);
            forge.wait(ticket);
            REQUIRE(hasRun==true);
            REQUIRE(res==345);
            LOG("Forge loop nbr: {}", i)
        }
    }

    SECTION("Multiple Tasks") {
        Forge forge;
        std::set<int> s;
        std::vector<int> tickets;
        std::shared_mutex mutex;
        for(auto i = 0; i < 128; ++i) {
            tickets.push_back(forge.add([i]()->Coroutine{co_return i;}, [i, &s, &mutex](Data const &data, Ticket const &ticket, PathSpaceTE &space){
                REQUIRE(data.is<int>()==true);
                auto writeLock = std::unique_lock<std::shared_mutex>(mutex);
                s.insert(i);
            }, space));
        }
        for(auto const &ticket : tickets)
            forge.wait(ticket);
        REQUIRE(s.size()==128);
        for(auto i = 0; i < 128; ++i)
            REQUIRE(s.contains(i));
    }
}