#include <doctest.h>

#include <concepts>
#include <iostream>

struct CTest {
    explicit CTest(std::integral auto const &i)       : mode(1) {}
    explicit CTest(std::floating_point auto const &f) : mode(2) {}

    int mode = 0;
};


TEST_CASE("Concepts Syntax Check") {
    SUBCASE("CTest int/float") {
        CHECK(CTest(1).mode==1);
        CHECK(CTest(1.5).mode==2);
   }
}
