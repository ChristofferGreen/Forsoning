#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("Path") {
    Path const path{"/test1/test2/test3"};
    auto const rangeOpt = path.range();
    auto range = *rangeOpt;

    SUBCASE("Initial state") {
        CHECK(*range.current=="test1");
        CHECK(*range.end=="test3");
    }

    SUBCASE("Iteration") {
        range = range.next();
        CHECK(*range.current=="test2");
        CHECK(*range.end=="test3");
        range = range.next();
        CHECK(*range.current=="test3");
        CHECK(*range.end=="test3");
        CHECK(*range.current==*range.end);
    }

    SUBCASE("Ill formed ranges") {
        CHECK(not Path{"/"}.range().has_value());
        CHECK(not Path{"/single_value"}.range().has_value());
    }

    SUBCASE("Only spaces") {
        CHECK(*Path{"/test1/test2"}.range().value().end=="test2");
        CHECK(*Path{"/test1/test2/"}.range().value().end=="");
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