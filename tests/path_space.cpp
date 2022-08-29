#include <catch.hpp>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

TEST_CASE("PathSpace") {
    PathSpaceTE space = PathSpace{};

    SECTION("Copy Constructor") {
        space.insert("/test", 123);
        PathSpaceTE space2;
        REQUIRE(space!=space2);
        space2 = space;
        REQUIRE(space==space2);
    }

    SECTION("Operator==") {
        PathSpaceTE space1, space2;
        REQUIRE(space1==space2);
        space1 = PathSpace{};
        REQUIRE(space1!=space2);
        space2 = PathSpace{};
        REQUIRE(space1==space2);
        space1.insert("/test", 123);
        REQUIRE(space1!=space2);
        space2.insert("/test", 123);
        REQUIRE(space1==space2);
    }

    SECTION("Operator== and Grab Small") {
        space.insert("/test", 1);
        space.insert("/test", 2);
        REQUIRE(space.grab<int>("/test").value()==1);
        PathSpaceTE space2 = PathSpace{};
        space2.insert("/test", 2);
        REQUIRE(space==space2);
    }

    SECTION("Operator== and Grab") {
        for(auto i = 0; i < 100; ++i)
            space.insert("/test", i);
        PathSpaceTE space2 = space;
        REQUIRE(space==space2);
        for(auto i = 0; i < 50; ++i)
            REQUIRE(space.grab<int>("/test").value()==i);
        REQUIRE(space!=space2);
        PathSpaceTE space3 = space;
        REQUIRE(space==space3);

        PathSpaceTE space4 = PathSpace{};
        for(auto i = 50; i < 100; ++i)
            space4.insert("/test", i);
        REQUIRE(space==space4);
    }
}