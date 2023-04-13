#include <catch.hpp>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Path") {
    Path const path{"/test1/test2/test3"};
    auto range = path;

    SECTION("Initial state") {
        REQUIRE(*range.spaceName()=="test1");
        REQUIRE(range.dataName()=="test3");
    }

    SECTION("Iteration") {
        range = range.next();
        REQUIRE(*range.spaceName()=="test2");
        REQUIRE(range.dataName()=="test3");
        range = range.next();
        REQUIRE(*range.spaceName()=="test3");
        REQUIRE(range.dataName()=="test3");
        REQUIRE(*range.spaceName()==range.dataName());
    }

    SECTION("Ill formed ranges") {
        REQUIRE(not Path{"/"}.isValid());
    }

    SECTION("Only spaces") {
        REQUIRE(Path{"/test1/test2"}.dataName()=="test2");
        REQUIRE(Path{"/test1/test2/"}.dataName()=="");
    }

    SECTION("Data name") {
        REQUIRE(path.dataName()=="test3");

        REQUIRE(Path{"/test1"}.dataName()=="test1");
        REQUIRE(Path{"/test1/test2/test3"}.dataName()=="test3");
    }

    SECTION("Space name") {
        REQUIRE(Path{"/test1/test2"}.spaceName().value()=="test1");
        REQUIRE(Path{"/test1/test2"}.next().spaceName().value()=="test2");
    }

    SECTION("IsAtRoot") {
        REQUIRE(Path{"/test1"}.isAtRoot()==true);
        REQUIRE(Path{"/test1/"}.isAtRoot()==true);
        REQUIRE(Path{"/test1/test2"}.isAtRoot()==true);
        REQUIRE(Path{"/test1/test2/"}.isAtRoot()==true);
        REQUIRE(Path{"/test1/test2/test3"}.isAtRoot()==true);
    }
}