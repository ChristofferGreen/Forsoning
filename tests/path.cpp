#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Path") {
    Path const path{"/test1/test2/test3"};
    auto range = path;

    SUBCASE("Initial state") {
        CHECK(*range.spaceName()=="test1");
        CHECK(range.dataName()=="test3");
    }

    SUBCASE("Iteration") {
        range = range.next();
        CHECK(*range.spaceName()=="test2");
        CHECK(range.dataName()=="test3");
        range = range.next();
        CHECK(*range.spaceName()=="test3");
        CHECK(range.dataName()=="test3");
        CHECK(*range.spaceName()==range.dataName());
    }

    SUBCASE("Ill formed ranges") {
        CHECK(not Path{"/"}.isValid());
    }

    SUBCASE("Only spaces") {
        CHECK(Path{"/test1/test2"}.dataName()=="test2");
        CHECK(Path{"/test1/test2/"}.dataName()=="");
    }

    SUBCASE("Data name") {
        CHECK(path.dataName()=="test3");

        CHECK(Path{"/test1"}.dataName()=="test1");
        CHECK(Path{"/test1/test2/test3"}.dataName()=="test3");
    }

    SUBCASE("Space name") {
        CHECK(Path{"/test1/test2"}.spaceName().value()=="test1");
        CHECK(Path{"/test1/test2"}.next().spaceName().value()=="test2");
    }

    SUBCASE("IsAtRoot") {
        CHECK(Path{"/test1"}.isAtRoot()==true);
        CHECK(Path{"/test1/"}.isAtRoot()==true);
        CHECK(Path{"/test1/test2"}.isAtRoot()==true);
        CHECK(Path{"/test1/test2/"}.isAtRoot()==true);
        CHECK(Path{"/test1/test2/test3"}.isAtRoot()==true);
    }
}