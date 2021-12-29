#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

TEST_CASE("Standard") {
    Forsoning::PathSpace space;
    Forsoning::View view(space, []{return true;});
    view.insert("/test", 5);
    auto const res = view.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 5);
}