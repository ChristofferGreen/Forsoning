#include <catch.hpp>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_grab_block(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template grab<InTRRRC>(path);
    REQUIRE(val.has_value());
    REQUIRE(val==testValue);
}

TEST_CASE("PathSpace Grab Block") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Grab Block Simple") {
        REQUIRE(space.insert("/test", 5) == true);
        REQUIRE(space.insert("/test", 6) == true);
        auto const val = space.grabBlock<int>("/test");
        REQUIRE(val == 5);
        auto const val2 = space.grabBlock<int>("/test");
        REQUIRE(val2 == 6);
    }

    SECTION("Grab Block") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.grabBlock<int>(rootTestPath) == 5);

        REQUIRE(space.insert(rootTestPath2, "hello") == true);
        auto val = space.grabBlock<std::string>(rootTestPath2);

        REQUIRE(std::string(val) == "hello");

        REQUIRE(space.insert(rootTestPath, 234) == true);
        REQUIRE(space.grabBlock<int>(rootTestPath) == 234);
    }

    SECTION("Grab Block POD") {
        POD const pod1;
        REQUIRE(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.grabBlock<POD>(rootTestPath);
        REQUIRE(pod1 == pod2);
    }

    SECTION("Grab Block NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.grabBlock<NonTrivial>(rootTestPath);
        REQUIRE(nt == nt2);
    }

    SECTION("Grab Block NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);
        auto const val = space.grabBlock<NonTrivialJS>(rootTestPath);

        REQUIRE(nt == val);
    }

    SECTION("Grab Block Bool") {
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.grabBlock<bool>(rootTestPath);
        auto const v2 = space.grabBlock<bool>(rootTestPath);
        auto const v3 = space.grabBlock<bool>(rootTestPath);
        REQUIRE(v1==true);
        REQUIRE(v2==false);
        REQUIRE(v3==true);
    }

    SECTION("Grab Block Signed Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.grabBlock<signed char>(rootTestPath);

        REQUIRE(val=='C');
    }

    SECTION("Grab Block Unsigned Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.grabBlock<unsigned char>(rootTestPath);

        REQUIRE(val=='C');
    }

    SECTION("Grab Block wchar_t") {
        REQUIRE(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.grabBlock<wchar_t>(rootTestPath);

        REQUIRE(val=='C');
    }

    SECTION("Grab Block Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.grabBlock<short>(rootTestPath);

        REQUIRE(val==43);
    }

    SECTION("Grab Block Unsigned Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.grabBlock<unsigned short>(rootTestPath);

        REQUIRE(val==static_cast<unsigned short>(-43));
    }

    SECTION("Grab Block Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.grabBlock<int>(rootTestPath);

        REQUIRE(val==static_cast<int>(43));
    }

    SECTION("Grab Block Unsigned Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.grabBlock<unsigned int>(rootTestPath);

        REQUIRE(val==static_cast<unsigned int>(-43));
    }

    SECTION("Grab Block Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.grabBlock<long>(rootTestPath);

        REQUIRE(val==static_cast<long>(43));
    }

    SECTION("Grab Block Unsigned Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.grabBlock<unsigned long>(rootTestPath);

        REQUIRE(val==static_cast<unsigned long>(-43));
    }

    SECTION("Grab Block Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.grabBlock<long long>(rootTestPath);

        REQUIRE(val==static_cast<long long>(43));
    }

    SECTION("Grab Block Unsigned Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.grabBlock<unsigned long long>(rootTestPath);

        REQUIRE(val==static_cast<unsigned long long>(-43));
    }

    SECTION("Grab Block Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.grabBlock<double>(rootTestPath);

        REQUIRE(val==static_cast<double>(5.45));
    }

    SECTION("Grab Block Long Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.grabBlock<long double>(rootTestPath);

        REQUIRE(val==static_cast<long double>(5.45));
    }

    SECTION("Grab Block Multiple Types") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.insert(rootTestPath, "hello") == true);
        REQUIRE(space.insert(rootTestPath, "hello2") == true);
        REQUIRE(space.insert(rootTestPath, 34) == true);
        check_grab_block(space, rootTestPath, static_cast<int>(5));
        check_grab_block(space, rootTestPath, static_cast<std::string>("hello"));
        check_grab_block(space, rootTestPath, static_cast<std::string>("hello2"));
        check_grab_block(space, rootTestPath, static_cast<int>(34));
    }

    SECTION("Grab Block Deep") {
        REQUIRE(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.grabBlock<int>(rootTestTest2Path);

        REQUIRE(val==static_cast<int>(5));
    }

    SECTION("Grab Block Vector Order") {
        REQUIRE(space.insert("/test", 5) == true);
        REQUIRE(space.insert("/test", 6) == true);
        REQUIRE(space.insert("/test", 7) == true);
        REQUIRE(space.insert("/test", 8) == true);
        REQUIRE(space.grabBlock<int>("/test")==5);
        REQUIRE(space.grabBlock<int>("/test")==6);
        REQUIRE(space.grabBlock<int>("/test")==7);
        REQUIRE(space.grabBlock<int>("/test")==8);
    }

    SECTION("Grab Block space") {
        REQUIRE(space.insert("/space", PathSpaceTE{PathSpace{}}) == true);
        nlohmann::json json;
        json["space"] = nlohmann::json::array({nlohmann::json()});
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert("/space/val", 34) == true);
        json["space"][0]["val"] = {34};
        REQUIRE(space.toJSON() == json);
        auto const s = space.grabBlock<PathSpaceTE>("/space");
        REQUIRE(s.toJSON() == json["space"][0]);
    }
//
    //SECTION("Grab Block coroutine") {
    //    REQUIRE(space.insert(rootTestPath, [&space]() -> Coroutine {
    //        for(auto i = 0; i < 10; ++i)
    //            co_yield i;
    //        space.insert("/finished", 1);
    //    }) == true);
    //    //space.grabBlockBlock("/finished");
    //    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //    nlohmann::json json;
    //    json["finished"] = {1};
    //    json["test"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    //    REQUIRE(space.toJSON() == json);
    //}
}