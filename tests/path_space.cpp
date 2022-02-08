#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("PathSpace") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};

    SUBCASE("Insert") {
        CHECK(space.insert(rootTestPath, 5) == true);
    }
}