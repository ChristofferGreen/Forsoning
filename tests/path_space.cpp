#include <doctest.h>

#include "PathSpace.hpp"

using namespace FSNG;

TEST_CASE("PathSpace") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

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
}