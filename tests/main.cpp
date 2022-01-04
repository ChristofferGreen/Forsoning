#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

#include <iostream>
#include <map>

using namespace Forsoning;

TEST_CASE("Insert And Grab") {
    PathSpaceTE space = PathSpace{};
    space.insert("/test", 5);
    space.insert("/test", 4);
    space.insert("/test", 3);
    auto res = space.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(res.value() == 5);
    res = space.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 4);
    res = space.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 3);
    res = space.grab<int>("/test");
    CHECK(!res.has_value());
}

TEST_CASE("Raw Linking") {
    PathSpaceTE space = PathSpace{};
    space.insert("/test", 5);
    PathSpaceTE spaceCopy = space;
    CHECK(spaceCopy.size() == 1);
    PathSpaceTE spaceLink = space.link();
    CHECK(spaceLink.size() == 1);

    auto res = space.grab<int>("/test");
    CHECK(res.has_value());
    CHECK(*res == 5);

    auto resCopy = spaceCopy.grab<int>("/test");
    CHECK(resCopy.has_value());
    CHECK(*resCopy == 5);

    auto resLink = spaceLink.grab<int>("/test");
    CHECK(!resLink.has_value());
}

TEST_CASE("View Linking") {
    PathSpaceTE space = PathSpace{};
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
    PathSpaceTE space = PathSpace{};
    auto const iterations = 20;
    CHECK(!(iterations%2));
    space.insert("/baton_first", 0);
    auto fun = [&space, iterations](std::string const &name){
        std::string const batonSelfPath = "/baton_"+name;
        std::string const batonOtherPath = "/baton_"+std::string(name=="first" ? "second" : "first");
        while(auto const batonOpt = space.grabBlock<int>(batonSelfPath)) {
            space.insert(batonOtherPath, batonOpt.value()+1);
            if(batonOpt.value()>=iterations) {
                space.insert("/result_"+name, batonOpt.value());
                return;
            }
        };
        CHECK(false);
    };
    std::thread first(fun, "first");
    std::thread second(fun, "second");

    first.join();
    second.join();

    CHECK(*space.grabBlock<int>("/result_first") == iterations);
    CHECK(*space.grabBlock<int>("/result_second") == iterations+1);
}

TEST_CASE("Sub Paths") {
    PathSpaceTE space = PathSpace{};
    space.insert("/test1/test2", 5);

    auto const parent = space.grab<std::unique_ptr<PathSpaceTE>>("/test1");
    CHECK(parent.has_value());
    CHECK(parent.value()->size()==1);

    auto const value = parent.value()->grab<int>("/test2");
    CHECK(value.has_value());
    CHECK(*value == 5);
}
