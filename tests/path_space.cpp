#include <doctest.h>

#include "PathSpace.hpp"
#include <iostream>

using namespace FSNG;

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
        std::cout << space.toJSON() << std::endl;
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
    }
}