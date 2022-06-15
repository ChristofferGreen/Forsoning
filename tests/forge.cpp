#include <doctest.h>

#include "PathSpace.hpp"

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
        auto const ticket = forge.add([]()->Coroutine{co_yield 345;}, [&res](Data const &data){
            CHECK(data.is<int>()==true);
            res=data.as<int>();
        });
        forge.wait(ticket);
        CHECK(res==345);
    }
}