#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

#include <iostream>

using namespace Forsoning;

TEST_CASE("Insert And Grab") {
    View view(PathSpace{}, Security::Policy::AlwaysAllow);
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
    View view(space, Security::Policy::AlwaysAllow);
    view.insert("/test", 5);
    CHECK(space.size() == view.size());
    View viewCopy(PathSpaceTE{space}, Security::Policy::AlwaysAllow);
    CHECK(viewCopy.size() == 1);
    View viewLink(space, Security::Policy::AlwaysAllow);
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

TEST_CASE("Blocking") {
    PathSpaceTE space(PathSpace{});
    View view(space, Security::Policy::AlwaysAllow);
    auto const path = "/test";
    view.insert(path, 0);
    auto const iterations = 100;
    int firstIter = 0;
    int secondIter = 0;
    auto fun = [&view, &path, &firstIter, &secondIter](std::string const &name, std::string const &other){
        int value = 0;
        while(value < iterations) {
            view.grabBlock<int>("/"+name);
            auto const valueOpt = view.grabBlock<int>(path);
            CHECK(valueOpt.has_value());
            value = valueOpt.value();
            view.insert(path, valueOpt.value()+1);
            view.insert("/"+other, 1);
            if(name=="first") firstIter++;
            else if(name=="second") secondIter++;
        }
    };
    view.insert("/first", 1);
    std::thread first(fun, "first", "second");
    std::thread second(fun, "second", "first");

    first.join();
    second.join();

    auto const valueOpt = view.grabBlock<int>(path);
    CHECK(*valueOpt == iterations+2);
    CHECK(firstIter == secondIter);
    CHECK(firstIter == (iterations/2)+1);
}