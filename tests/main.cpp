#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "PathSpace.hpp"

#include <iostream>
#include <map>

using namespace Forsoning;

TEST_CASE("PathUtils") {
    SUBCASE("Path Range") {
        std::filesystem::path path = "/test1/test2";
        auto range = PathUtils::path_range(path);
        CHECK(range.has_value());
        CHECK(*range.value().first=="test1");
        CHECK(*range.value().second=="test2");
        range.value().first++;
        CHECK(range.value().first->filename()=="test2");

        path = "/";
        range = PathUtils::path_range(path);
        CHECK(!range.has_value());

        path = "/test1";
        range = PathUtils::path_range(path);
        CHECK(!range.has_value());

        path = "/test1/test2/test3";
        range = PathUtils::path_range(path);
        CHECK(range.has_value());
        CHECK(*range.value().first=="test1");
        CHECK(*range.value().second=="test3");
        range.value().first++;
        CHECK(range.value().first->filename()=="test2");
        range.value().first++;
        CHECK(range.value().first->filename()=="test3");

        path = "/test1/test2/";
        range = PathUtils::path_range(path);
        CHECK(range.has_value());
        CHECK(*range.value().first=="test1");
        CHECK(*range.value().second=="");
        range.value().first++;
        CHECK(range.value().first->filename()=="test2");
    }

    SUBCASE("Path iters not equals") { // Make sure position matters for equals
        std::filesystem::path const path = "/test/test";
        auto iter1 = path.begin();
        iter1++;
        auto iter2 = iter1;
        iter2++;
        CHECK(iter1!=iter2);
        CHECK(*iter1=="test");
        CHECK(*iter2=="test");
    }

    SUBCASE("Remove filename") {
        std::filesystem::path path = "/test1/test2";
        path = PathUtils::remove_filename(path);
        CHECK(path=="/test1/");
    }

    SUBCASE("Data name") {
        std::filesystem::path path = "/test1/test2";
        CHECK(PathUtils::data_name(path)=="test2");

        path = "/test1";
        CHECK(PathUtils::data_name(path).value()=="test1");

        path = "/test1/test2/test3";
        CHECK(PathUtils::data_name(path).value()=="test3");

        path = "/test1/";
        CHECK(!PathUtils::data_name(path).has_value());

        path = "/";
        CHECK(!PathUtils::data_name(path).has_value());
    }

    SUBCASE("Data name iters") {
        std::filesystem::path path = "/test1/test2/test3";
        auto const iters = PathUtils::path_range(path);
        CHECK(iters.has_value());
        CHECK(*iters->first=="test1");
        CHECK(*iters->second=="test3");
        auto name = PathUtils::data_name(*iters);
        CHECK(name.has_value());
        CHECK(name.value()=="test3");
    }
}


TEST_CASE_FIXTURE(PathSpace, "Insert: Iterator Version") {
    PathSpace space;
    auto const key = Security::Policy::AlwaysAllow;
    std::filesystem::path const path = "/test1/test2/test3";
    auto const optPair = PathUtils::path_range(path);
    CHECK(optPair.has_value());
    space.insert(path, *optPair, 5, key);
}

TEST_CASE("PathSpace") {
    SUBCASE("Insert And Grab") {
        PathSpaceTE space = PathSpace{};
        auto const key = Security::Policy::AlwaysAllow;

        // Insert
        CHECK(space.size()==0);
        CHECK(space.insert("/test", 5, key));
        CHECK(space.size()==1);
        CHECK(space.insert("/test", 4, key));
        CHECK(space.size()==2);
        CHECK(space.insert("/test", 3, key));
        CHECK(space.size()==3);

        CHECK(!space.insert("/", 2, key));
        CHECK(space.size()==3);

        // Grab
        auto res = space.grab<int>("/test", key);
        CHECK(res);
        CHECK(res.value() == 5);
        CHECK(space.size()==2);
        res = space.grab<int>("/test", key);
        CHECK(res);
        CHECK(res.value() == 4);
        CHECK(space.size()==1);
        res = space.grab<int>("/test", key);
        CHECK(res);
        CHECK(res.value() == 3);

        // Grab empty
        CHECK(space.size()==0);
        res = space.grab<int>("/test", key);
        CHECK(!res);
    }

    SUBCASE("Insert And Grab Spaces") {
        PathSpace space;
    /* CHECK(space.size()==0);
        space.insert("/test1/test2/test3", 5);
        CHECK(space.size()==1);
        auto res = space.grab("/test1/test2/3");
        CHECK(res);
        CHECK(std::get<int>(*res) == 5);
        CHECK(space.size()==1);*/
    }

    /*
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
    */
}