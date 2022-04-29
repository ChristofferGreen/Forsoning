#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("PathSpace Grab") {
    PathSpaceTE space = PathSpace{};

    SUBCASE("Grab Simple") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        CHECK(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        CHECK(val.value() == 6);
    }
}