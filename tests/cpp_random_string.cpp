#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Random String") {
    SUBCASE("Basic") {
        std::string r1 = random_string();
        std::string r2 = random_string();
        CHECK(r1!=r2);
    }
}