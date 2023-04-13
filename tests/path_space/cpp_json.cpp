#include <catch.hpp>

#include "nlohmann/json.hpp"

TEST_CASE("Nlohmann JSON") {
    SECTION("Simple") {
        nlohmann::json j;
        j["test"] = 3;
        REQUIRE(j["test"]==3);
        REQUIRE(j["test2"]==nullptr);
   }
}
