#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Forge") {
    SUBCASE("Eschelon") {
        Eschelon queue;
        queue.add([]()->Coroutine{co_yield 0;}, [](Data const &data){});
        auto const task = queue.popWait();
        CHECK(task.has_value()==true);
        CHECK(task.value().ticket==0);
    }
}