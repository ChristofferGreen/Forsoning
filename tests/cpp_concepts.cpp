#include <catch.hpp>

#include <concepts>
#include <iostream>

struct CTest {
    explicit CTest(std::integral auto const &i)       : mode(1) {}
    explicit CTest(std::floating_point auto const &f) : mode(2) {}

    int mode = 0;
};


TEST_CASE("Concepts Syntax Check") {
    SECTION("CTest int/float") {
        REQUIRE(CTest(1).mode==1);
        REQUIRE(CTest(1.5).mode==2);
   }
}
