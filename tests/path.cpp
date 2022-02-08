#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Path") {
    Path const path{"/test1/test2/test3"};
    auto const rangeOpt = path.range();
    auto range = *rangeOpt;

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
        CHECK(not Path{"/"}.range().has_value());
    }

    SUBCASE("Only spaces") {
        CHECK(Path{"/test1/test2"}.range().value().dataName()=="test2");
        CHECK(Path{"/test1/test2/"}.range().value().dataName()=="");
    }

    SUBCASE("Data name") {
        CHECK(path.dataName().has_value());
        CHECK(path.dataName().value()=="test3");

        CHECK(Path{"/test1"}.dataName().value()=="test1");
        CHECK(Path{"/test1/test2/test3"}.dataName().value()=="test3");
    }

    SUBCASE("Space name") {
        CHECK(Path{"/test1/test2"}.range().value().spaceName().value()=="test1");
        CHECK(Path{"/test1/test2"}.range().value().next().spaceName().value()=="test2");
    }
}