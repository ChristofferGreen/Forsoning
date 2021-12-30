#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

using namespace Forsoning;

TEST_CASE("Insert And Grab") {
    View view(PathSpace{}, SecurityAlwaysAllow);
    view.insert("/test", 5);
    view.insert("/test", 4);
    view.insert("/test", 3);
    auto res = view.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 5);
    res = view.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 4);
    res = view.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 3);
    res = view.grab<int>("/test");
    CHECK(!res.has_value());
}

TEST_CASE("Space Linking") {
    PathSpaceTE space(PathSpace{});
    View view(space, SecurityAlwaysAllow);
    view.insert("/test", 5);
    CHECK(space.size() == view.size());
    View viewCopy(PathSpaceTE{space}, SecurityAlwaysAllow);
    CHECK(viewCopy.size() == 1);
    View viewLink(space, SecurityAlwaysAllow);
    CHECK(viewLink.size() == 1);

    auto res = view.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 5);

    auto resCopy = viewCopy.grab<int>("/test");
    CHECK(resCopy.has_value());
    CHECK(*resCopy == 5);

    auto resLink = viewLink.grab<int>("/test");
    CHECK(!resLink.has_value());
}