#include <catch.hpp>

#include "PathSpace.hpp"
#include "PathSpace2.hpp"
#include "FSNG/InReference.hpp"

using namespace FSNG;

TEST_CASE("TypeInfo") {
    SECTION("Int") {
        auto const tinfo = FSNG::TypeInfo::Create<int>();
        REQUIRE(tinfo.element_size==sizeof(int));
        REQUIRE(tinfo.nbr_elements.has_value()==false);
        REQUIRE(tinfo.type==&typeid(int));
        REQUIRE(tinfo.arrayElementType==nullptr);
        REQUIRE(tinfo.isTriviallyCopyable==true);
        REQUIRE(tinfo.isInternalDataTriviallyCopyable==false);
        REQUIRE(tinfo.isFundamental==true);
        REQUIRE(tinfo.isPathSpace==false);
        REQUIRE(tinfo.isArray==false);
    }

    SECTION("std::string") {
        auto const tinfo = FSNG::TypeInfo::Create<std::string>();
        REQUIRE(tinfo.element_size==sizeof(std::string::value_type));
        REQUIRE(tinfo.nbr_elements.has_value()==false);
        REQUIRE(tinfo.type==&typeid(std::string));
        REQUIRE(tinfo.arrayElementType==&typeid(std::string::value_type));
        REQUIRE(tinfo.isTriviallyCopyable==false);
        REQUIRE(tinfo.isInternalDataTriviallyCopyable==true);
        REQUIRE(tinfo.isFundamental==false);
        REQUIRE(tinfo.isPathSpace==false);
        REQUIRE(tinfo.isArray==false);
    }

    SECTION("PathSpace2") {
        auto const tinfo = FSNG::TypeInfo::Create<PathSpace2>();
        REQUIRE(tinfo.element_size==sizeof(PathSpace2));
        REQUIRE(tinfo.nbr_elements.has_value()==false);
        REQUIRE(tinfo.type==&typeid(PathSpace2));
        REQUIRE(tinfo.arrayElementType==nullptr);
        REQUIRE(tinfo.isTriviallyCopyable==false);
        REQUIRE(tinfo.isInternalDataTriviallyCopyable==false);
        REQUIRE(tinfo.isFundamental==false);
        REQUIRE(tinfo.isPathSpace==true);
        REQUIRE(tinfo.isArray==false);
    }

    SECTION("std::vector<int>") {
        auto const tinfo = FSNG::TypeInfo::Create<std::vector<int>>();
        REQUIRE(tinfo.element_size==sizeof(int));
        REQUIRE(tinfo.nbr_elements.has_value()==false);
        REQUIRE(tinfo.type==&typeid(std::vector<int>));
        REQUIRE(tinfo.arrayElementType==&typeid(int));
        REQUIRE(tinfo.isTriviallyCopyable==false);
        REQUIRE(tinfo.isInternalDataTriviallyCopyable==true);
        REQUIRE(tinfo.isFundamental==false);
        REQUIRE(tinfo.isPathSpace==false);
        REQUIRE(tinfo.isArray==false);
    }

    SECTION("std::vector<std::string>") {
        auto const tinfo = FSNG::TypeInfo::Create<std::vector<std::string>>();
        REQUIRE(tinfo.element_size==sizeof(std::string::value_type));
        REQUIRE(tinfo.nbr_elements.has_value()==false);
        REQUIRE(tinfo.type==&typeid(std::vector<std::string>));
        REQUIRE(tinfo.arrayElementType==&typeid(std::string::value_type));
        REQUIRE(tinfo.isTriviallyCopyable==false);
        REQUIRE(tinfo.isInternalDataTriviallyCopyable==true);
        REQUIRE(tinfo.isFundamental==false);
        REQUIRE(tinfo.isPathSpace==false);
        REQUIRE(tinfo.isArray==false);
    }
}
