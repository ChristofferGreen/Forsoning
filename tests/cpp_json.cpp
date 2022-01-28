#include <doctest.h>

#include "nlohmann/json.hpp"

TEST_CASE("Nlohmann JSON") {
    SUBCASE("Simple") {
        nlohmann::json j;
        j["test"] = 3;
        CHECK(j["test"]==3);
        CHECK(j["test2"]==nullptr);
   }
}
