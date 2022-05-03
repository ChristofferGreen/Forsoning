#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

TEST_CASE("PathSpace Grab") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Grab Simple") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        CHECK(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        CHECK(val2.value() == 6);
    }

    SUBCASE("Grab") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.grab<int>(rootTestPath) == 5);

        CHECK(space.insert(rootTestPath2, "hello") == true);
        auto val = space.grab<std::string>(rootTestPath2);
        CHECK(val.has_value());
        CHECK(std::string(val.value()) == "hello");

        CHECK(space.insert(rootTestPath, 234) == true);
        CHECK(space.grab<int>(rootTestPath) == 234);
    }

    SUBCASE("Insert POD") {
        POD const pod1;
        CHECK(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.grab<POD>(rootTestPath);
        CHECK(pod2.has_value());
        CHECK(pod1 == pod2.value());
    }

    /*SUBCASE("Insert NonTrivial Class") {
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

    SUBCASE("Insert coroutine") {
        CHECK(space.insert(rootTestPath, [&space]() -> Coroutine {
            for(auto i = 0; i < 10; ++i)
                co_yield i;
            space.insert("/finished", 1);
        }) == true);
        //space.grabBlock("/finished");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        nlohmann::json json;
        json["finished"] = {1};
        json["test"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        CHECK(space.toJSON() == json);
    }*/
}