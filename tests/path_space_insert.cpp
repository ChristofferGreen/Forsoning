#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

#include <iostream>

using namespace FSNG;

TEST_CASE("PathSpace Insert") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Insert") {
        CHECK(space.insert(rootTestPath, 5) == true);
        nlohmann::json json;
        json["test"] = {5};
        CHECK(space.toJSON() == json);

        CHECK(space.insert(rootTestPath2, "hello") == true);
        json["test2"] = {"hello"};
        CHECK(space.toJSON() == json);

        CHECK(space.insert(rootTestPath, 234) == true);
        json["test"].push_back(234);
        CHECK(space.toJSON() == json);

        json["test"].push_back(2345);
        CHECK(space.toJSON() != json);
    }

    SUBCASE("Insert POD") {
        CHECK(space.insert(rootTestPath, POD()) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", 44.0} }) });
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);

        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"a", 13}, {"b", {1, 2, 3}} }) });
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Bool") {
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<bool>(true), static_cast<bool>(false), static_cast<bool>(true)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Char*") {
        CHECK(space.insert(rootTestPath, "Test String") == true);
        CHECK(space.insert(rootTestPath, "Test String2") == true);
        CHECK(space.insert(rootTestPath, "Test String3") == true);
        nlohmann::json json;
        json["test"] = {"Test String", "Test String2", "Test String3"};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Signed Char") {
        CHECK(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<signed char>('C')};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Unsigned Char") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned char>('C')};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert wchar_t") {
        CHECK(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        nlohmann::json json;
        json["test"] = {static_cast<wchar_t>('C')};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Short") {
        CHECK(space.insert(rootTestPath, static_cast<short>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<short>(43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Unsigned Short") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned short>(-43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Int") {
        CHECK(space.insert(rootTestPath, static_cast<int>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<int>(43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Unsigned Int") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned int>(-43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Long") {
        CHECK(space.insert(rootTestPath, static_cast<long>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long>(43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Unsigned Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned long>(-43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long long>(43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Unsigned Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<unsigned long long>(-43)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Double") {
        CHECK(space.insert(rootTestPath, 5.45) == true);
        nlohmann::json json;
        json["test"] = {5.45};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Long Double") {
        CHECK(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        nlohmann::json json;
        json["test"] = {static_cast<long double>(5.45)};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert Multiple Types") {
        CHECK(space.insert(rootTestPath, 5) == true);
        nlohmann::json json;
        json["test"] = {5};
        CHECK(space.toJSON() == json);

        CHECK(space.insert(rootTestPath, "hello") == true);
        json["test"].push_back("hello");
        CHECK(space.toJSON() == json);

        CHECK(space.insert(rootTestPath, "hello2") == true);
        json["test"].push_back("hello2");
        CHECK(space.toJSON() == json);    

        CHECK(space.insert(rootTestPath, 34) == true);
        json["test"].push_back(34);
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert deep") {
        CHECK(space.insert(rootTestTest2Path, 5) == true);
        nlohmann::json json;
        json["test"] = nlohmann::json::array({ nlohmann::json::object({ {"test2", {5}} }) });
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Insert space") {
        PathSpaceTE space2 = PathSpace{};
        CHECK(space.insert("/space", space2) == true);
        nlohmann::json json;
        json["space"] = nlohmann::json::array({nlohmann::json()});
        CHECK(space.toJSON() == json);

        CHECK(space.insert("/space/val", 34) == true);
        json["space"][0]["val"] = {34};
        CHECK(space.toJSON() == json);
    }
}

TEST_CASE("PathSpace Insert Multithreaded") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};
    SUBCASE("Insert coroutine") {
        CHECK(space.insert(rootTestPath, [&space]() -> Coroutine {
            for(auto i = 0; i < 10; ++i)
                co_yield i;
            space.insert("/finished", 1);
        }) == true);
        space.grabBlock<int>("/finished");
        nlohmann::json json;
        json["finished"] = nullptr;
        json["test"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        CHECK(space.toJSON() == json);
    }
}