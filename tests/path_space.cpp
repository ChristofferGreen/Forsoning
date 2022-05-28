#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

TEST_CASE("PathSpace") {
    PathSpaceTE space = PathSpace{};

    SUBCASE("Copy Constructor") {
        space.insert("/test", 123);
        PathSpaceTE space2;
        CHECK(space!=space2);
        space2 = space;
        CHECK(space==space2);
    }

    SUBCASE("Operator==") {
        PathSpaceTE space1, space2;
        CHECK(space1==space2);
        space1 = PathSpace{};
        CHECK(space1!=space2);
        space2 = PathSpace{};
        CHECK(space1==space2);
        space1.insert("/test", 123);
        CHECK(space1!=space2);
        space2.insert("/test", 123);
        CHECK(space1==space2);
    }

    SUBCASE("Operator== and Grab Small") {
        space.insert("/test", 1);
        space.insert("/test", 2);
        CHECK(space.grab<int>("/test").value()==1);
        PathSpaceTE space2 = PathSpace{};
        space.insert("/test", 2);
        CHECK(space==space2);
    }

    SUBCASE("Operator== and Grab") {
        for(auto i = 0; i < 100; ++i)
            space.insert("/test", i);
        PathSpaceTE space2 = space;
        CHECK(space==space2);
        for(auto i = 0; i < 50; ++i)
            CHECK(space.grab<int>("/test").value()==i);
        CHECK(space!=space2);
        PathSpaceTE space3 = space;
        CHECK(space==space3);

        PathSpaceTE space4 = PathSpace{};
        for(auto i = 50; i < 100; ++i)
            space4.insert("/test", i);
        CHECK(space==space4);
    }
}