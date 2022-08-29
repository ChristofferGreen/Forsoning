#include <catch.hpp>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Random String") {
    SECTION("Basic") {
        std::string r1 = random_string();
        std::string r2 = random_string();
        REQUIRE(r1!=r2);
    }
}