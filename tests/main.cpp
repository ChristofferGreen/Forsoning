#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

#include <iostream>
#include <map>

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
    View view(PathSpace{}, Security::Policy::AlwaysAllow);
    auto const iterations = 100;
    view.insert("/baton_first", 0);
    auto fun = [&view](std::string const &name){
        std::string const batonSelfPath = "/baton_"+name;
        std::string const batonOtherPath = "/baton_"+std::string(name=="first" ? "second" : "first");
        while(auto const batonOpt = view.grabBlock<int>(batonSelfPath)) {
            view.insert(batonOtherPath, batonOpt.value()+1);
            if(batonOpt.value()>=iterations) {
                view.insert("/result_"+name, batonOpt.value());
                break;
            }
        };
    };
    std::thread first(fun, "first");
    std::thread second(fun, "second");

    first.join();
    second.join();

    CHECK(*view.grabBlock<int>("/result_first")  == iterations);
    CHECK(*view.grabBlock<int>("/result_second")  == iterations+1);
}

TEST_CASE("Sub Paths") {
    View view(PathSpace{}, Security::Policy::AlwaysAllow);
    view.insert("/test1/test2", 5);
    auto res = view.grab<int>("/test1/test2");
    CHECK(res.has_value());
    CHECK(*res == 5);
}