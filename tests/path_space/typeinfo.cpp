#include <catch.hpp>

#include "FSNG/InReference.hpp"

using namespace FSNG;

TEST_CASE("TypeInfo") {

    SECTION("Types") {
        auto const tinfoInt = FSNG::TypeInfo::Create<int>();
        auto const tinfoString = FSNG::TypeInfo::Create<std::string>();
        REQUIRE(tinfoInt.element_size==sizeof(int));
        REQUIRE(tinfoString.element_size==sizeof(std::string::value_type));
    }
}
