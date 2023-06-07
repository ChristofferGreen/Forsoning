#include <catch.hpp>

#include "PathSpace.hpp"
#include "PathSpace2.hpp"
#include "test_utils.hpp"

#include <iostream>

using namespace FSNG;

TEST_CASE("PathSpace2 Insert") {
    PathSpace2 space;

    SECTION("Basic") {
        nlohmann::json json;
        REQUIRE(space.insert("/test", 5) == true);
        json["test"] += {5};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test2", "hello") == true);
        json["test2"] += {"hello"};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test", 234) == true);
        json["test"][0] += 234;
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test2", std::string("moo")) == true);
        json["test2"][0] += "moo";
        REQUIRE(space.toJSON() == json);

        int ia[4] = {0, 2, 3, 6};
        REQUIRE(space.insert("/test_int_array", ia) == true);
        json["test_int_array"] += {0, 2, 3, 6};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test3", std::vector<int>({0, 1, 2, 4})) == true);
        json["test3"] += std::vector<int>({0, 1, 2, 4});
        REQUIRE(space.toJSON() == json);

        json["test"][0] += 2345;
        REQUIRE(space.toJSON() != json);
    }

    SECTION("POD") {
        REQUIRE(space.insert("/test", POD()) == true);
        REQUIRE(space.insert("/test", POD()) == true);

        nlohmann::json json;
        json["test"] += nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", 44.0} }) });
        json["test"][0] += nlohmann::json::object({ {"a", 13}, {"b", 44.0} });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert("/test", nt) == true);
        REQUIRE(space.insert("/test", nt) == true);

        nlohmann::json json;
        json["test"] += nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        json["test"][0] += nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert("/test", nt) == true);
        REQUIRE(space.insert("/test", nt) == true);

        nlohmann::json json;
        json["test"] += nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        json["test"][0] += nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Bool") {
        REQUIRE(space.insert("/test", static_cast<bool>(true)) == true);
        REQUIRE(space.insert("/test", static_cast<bool>(false)) == true);
        REQUIRE(space.insert("/test", static_cast<bool>(true)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<bool>(true), static_cast<bool>(false), static_cast<bool>(true)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Char*") {
        REQUIRE(space.insert("/test", "Test String") == true);
        REQUIRE(space.insert("/test", "Test String2") == true);
        REQUIRE(space.insert("/test", "Test String3") == true);
        nlohmann::json json;
        json["test"] += {"Test String", "Test String2", "Test String3"};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Signed Char") {
        REQUIRE(space.insert("/test", static_cast<signed char>('C')) == true);
        nlohmann::json json;
        json["test"] += {static_cast<signed char>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Char") {
        REQUIRE(space.insert("/test", static_cast<unsigned char>('C')) == true);
        nlohmann::json json;
        json["test"] += {static_cast<unsigned char>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("wchar_t") {
        REQUIRE(space.insert("/test", static_cast<wchar_t>('C')) == true);
        nlohmann::json json;
        json["test"] += {static_cast<wchar_t>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Short") {
        REQUIRE(space.insert("/test", static_cast<short>(43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<short>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Short") {
        REQUIRE(space.insert("/test", static_cast<unsigned short>(-43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<unsigned short>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Int") {
        REQUIRE(space.insert("/test", static_cast<int>(43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<int>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Int") {
        REQUIRE(space.insert("/test", static_cast<unsigned int>(-43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<unsigned int>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long") {
        REQUIRE(space.insert("/test", static_cast<long>(43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<long>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Long") {
        REQUIRE(space.insert("/test", static_cast<unsigned long>(-43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<unsigned long>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long Long") {
        REQUIRE(space.insert("/test", static_cast<long long>(43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<long long>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Long Long") {
        REQUIRE(space.insert("/test", static_cast<unsigned long long>(-43)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<unsigned long long>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Double") {
        REQUIRE(space.insert("/test", 5.45) == true);
        nlohmann::json json;
        json["test"] += {5.45};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long Double") {
        REQUIRE(space.insert("/test", static_cast<long double>(5.45)) == true);
        nlohmann::json json;
        json["test"] += {static_cast<long double>(5.45)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Multiple Types") { // implement via get due to random order
        /*nlohmann::json json;
        REQUIRE(space.insert("/test", 5) == true);
        json["test"] += {5};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test", "hello") == true);
        json["test"].push_back("hello");
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/test", "hello2") == true);
        json["test"].push_back("hello2");
        REQUIRE(space.toJSON() == json);    

        REQUIRE(space.insert("/test", 34) == true);
        json["test"].push_back(34);
        REQUIRE(space.toJSON() == json);*/
    }

    SECTION("deep") {
        REQUIRE(space.insert("/test/test2", 5) == true);
        nlohmann::json json;
        json["test"] += nlohmann::json::array({ nlohmann::json::object({ {"test2", {5}} }) });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("space") {
        PathSpace2 space2;
        REQUIRE(space.insert("/space", space2) == true);
        nlohmann::json json;
        json["space"] += nlohmann::json::array({nlohmann::json()});
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/space/val", 34) == true);
        json["space"][0]["val"] = {34};
        REQUIRE(space.toJSON() == json);
    }
}

TEST_CASE("PathSpace Insert") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Basic") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        nlohmann::json json;
        json["test"] = {5};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath2, "hello") == true);
        json["test2"] = {"hello"};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath, 234) == true);
        json["test"].push_back(234);
        REQUIRE(space.toJSON() == json);

        json["test"].push_back(2345);
        REQUIRE(space.toJSON() != json);
    }

    SECTION("POD") {
        REQUIRE(space.insert(rootTestPath, POD()) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", 44.0} }) });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Bool") {
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<bool>(true), static_cast<bool>(false), static_cast<bool>(true)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Char*") {
        REQUIRE(space.insert(rootTestPath, "Test String") == true);
        REQUIRE(space.insert(rootTestPath, "Test String2") == true);
        REQUIRE(space.insert(rootTestPath, "Test String3") == true);
        nlohmann::json json;
        json["test"] = {"Test String", "Test String2", "Test String3"};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Signed Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<signed char>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned char>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("wchar_t") {
        REQUIRE(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<wchar_t>('C')};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<short>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<short>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned short>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<int>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<int>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned int>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned long>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long long>(43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Unsigned Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned long long>(-43)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Double") {
        REQUIRE(space.insert(rootTestPath, 5.45) == true);
        nlohmann::json json;
        json["test"] = {5.45};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Long Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long double>(5.45)};
        REQUIRE(space.toJSON() == json);
    }

    SECTION("Multiple Types") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        nlohmann::json json;
        json["test"] = {5};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath, "hello") == true);
        json["test"].push_back("hello");
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath, "hello2") == true);
        json["test"].push_back("hello2");
        REQUIRE(space.toJSON() == json);    

        REQUIRE(space.insert(rootTestPath, 34) == true);
        json["test"].push_back(34);
        REQUIRE(space.toJSON() == json);
    }

    SECTION("deep") {
        REQUIRE(space.insert(rootTestTest2Path, 5) == true);
        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"test2", {5}} }) });
        REQUIRE(space.toJSON() == json);
    }

    SECTION("space") {
        PathSpaceTE space2 = PathSpace{};
        REQUIRE(space.insert("/space", space2) == true);
        nlohmann::json json;
        json["space"] = nlohmann::json::array({nlohmann::json()});
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/space/val", 34) == true);
        json["space"][0]["val"] = {34};
        REQUIRE(space.toJSON() == json);
    }
}

TEST_CASE("PathSpace Insert Multithreaded") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};
    SECTION("Coroutine") {
        REQUIRE(space.insert(rootTestPath, [&space]() -> CoroutineVoid {
            for(auto i = 0; i < 10; ++i)
                co_yield i;
            space.insert("/finished", 1);
            co_return;
        }) == true);
        space.grabBlock<int>("/finished");
        /*nlohmann::json json;
        json["finished"] = nullptr;
        json["test"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        REQUIRE(space.toJSON() == json);*/
    }

    SECTION("Coroutine Result Path") {
        space.insert("/coro", []()->Coroutine{co_return 123;}, "/res");
        REQUIRE(space.grabBlock<int>("/res")==123);
    }
}