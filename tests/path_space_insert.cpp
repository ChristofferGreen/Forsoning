#include <doctest.h>

#include "PathSpace.hpp"
#include <iostream>

using namespace FSNG;

struct POD {
    int a = 13;
    float b = 44.0;
};

void to_json(nlohmann::json& j, const POD& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

struct NonTrivial {
    int a = 13;
    std::vector<int> b;
};

void to_json(nlohmann::json& j, const NonTrivial& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

void to_bytevec(std::vector<std::byte> &vec, NonTrivial const &obj) {
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(&obj.a)),
              static_cast<std::byte const * const>(static_cast<void const * const>(&obj.a)) + sizeof(int),
              std::back_inserter(vec));
    int const elements = obj.b.size();
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(&elements)),
              static_cast<std::byte const * const>(static_cast<void const * const>(&elements)) + sizeof(int),
              std::back_inserter(vec));
    std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(obj.b.data())),
              static_cast<std::byte const * const>(static_cast<void const * const>(obj.b.data())) + sizeof(int) * obj.b.size(),
              std::back_inserter(vec));
}

void from_bytevec(std::byte const *vec, NonTrivial &ret) {
    ret.a = *reinterpret_cast<int const*>(vec);
    vec += sizeof(int);
    int const elements = *reinterpret_cast<int const*>(vec);
    vec += sizeof(int);
    for(auto i = 0; i < elements; ++i) {
        auto const val = *reinterpret_cast<int const *>(vec);
        ret.b.push_back(val);
        vec += sizeof(int);
    }
}

struct NonTrivialJS {
    int a = 13;
    std::vector<int> b;
};

void to_json(nlohmann::json& j, const NonTrivialJS& p) {
    j = nlohmann::json{{"a", p.a}, {"b", p.b}};
}

TEST_CASE("PathSpace") {
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
    }
}