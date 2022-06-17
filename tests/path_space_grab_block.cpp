#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_grab_block(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template grab<InTRRRC>(path);
    CHECK(val.has_value());
    CHECK(val==testValue);
}

TEST_CASE("PathSpace Grab Block") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Grab Block Simple") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.grabBlock<int>("/test");
        CHECK(val == 5);
        auto const val2 = space.grabBlock<int>("/test");
        CHECK(val2 == 6);
    }

    SUBCASE("Grab Block") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.grabBlock<int>(rootTestPath) == 5);

        CHECK(space.insert(rootTestPath2, "hello") == true);
        auto val = space.grabBlock<std::string>(rootTestPath2);

        CHECK(std::string(val) == "hello");

        CHECK(space.insert(rootTestPath, 234) == true);
        CHECK(space.grabBlock<int>(rootTestPath) == 234);
    }

    SUBCASE("Grab Block POD") {
        POD const pod1;
        CHECK(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.grabBlock<POD>(rootTestPath);
        CHECK(pod1 == pod2);
    }

    SUBCASE("Grab Block NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.grabBlock<NonTrivial>(rootTestPath);
        CHECK(nt == nt2);
    }

    SUBCASE("Grab Block NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);
        auto const val = space.grabBlock<NonTrivialJS>(rootTestPath);

        CHECK(nt == val);
    }

    SUBCASE("Grab Block Bool") {
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.grabBlock<bool>(rootTestPath);
        auto const v2 = space.grabBlock<bool>(rootTestPath);
        auto const v3 = space.grabBlock<bool>(rootTestPath);
        CHECK(v1==true);
        CHECK(v2==false);
        CHECK(v3==true);
    }

    SUBCASE("Grab Block Signed Char") {
        CHECK(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.grabBlock<signed char>(rootTestPath);

        CHECK(val=='C');
    }

    SUBCASE("Grab Block Unsigned Char") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.grabBlock<unsigned char>(rootTestPath);

        CHECK(val=='C');
    }

    SUBCASE("Grab Block wchar_t") {
        CHECK(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.grabBlock<wchar_t>(rootTestPath);

        CHECK(val=='C');
    }

    SUBCASE("Grab Block Short") {
        CHECK(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.grabBlock<short>(rootTestPath);

        CHECK(val==43);
    }

    SUBCASE("Grab Block Unsigned Short") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.grabBlock<unsigned short>(rootTestPath);

        CHECK(val==static_cast<unsigned short>(-43));
    }

    SUBCASE("Grab Block Int") {
        CHECK(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.grabBlock<int>(rootTestPath);

        CHECK(val==static_cast<int>(43));
    }

    SUBCASE("Grab Block Unsigned Int") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.grabBlock<unsigned int>(rootTestPath);

        CHECK(val==static_cast<unsigned int>(-43));
    }

    SUBCASE("Grab Block Long") {
        CHECK(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.grabBlock<long>(rootTestPath);

        CHECK(val==static_cast<long>(43));
    }

    SUBCASE("Grab Block Unsigned Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.grabBlock<unsigned long>(rootTestPath);

        CHECK(val==static_cast<unsigned long>(-43));
    }

    SUBCASE("Grab Block Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.grabBlock<long long>(rootTestPath);

        CHECK(val==static_cast<long long>(43));
    }

    SUBCASE("Grab Block Unsigned Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.grabBlock<unsigned long long>(rootTestPath);

        CHECK(val==static_cast<unsigned long long>(-43));
    }

    SUBCASE("Grab Block Double") {
        CHECK(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.grabBlock<double>(rootTestPath);

        CHECK(val==static_cast<double>(5.45));
    }

    SUBCASE("Grab Block Long Double") {
        CHECK(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.grabBlock<long double>(rootTestPath);

        CHECK(val==static_cast<long double>(5.45));
    }

    SUBCASE("Grab Block Multiple Types") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.insert(rootTestPath, "hello") == true);
        CHECK(space.insert(rootTestPath, "hello2") == true);
        CHECK(space.insert(rootTestPath, 34) == true);
        check_grab_block(space, rootTestPath, static_cast<int>(5));
        check_grab_block(space, rootTestPath, static_cast<std::string>("hello"));
        check_grab_block(space, rootTestPath, static_cast<std::string>("hello2"));
        check_grab_block(space, rootTestPath, static_cast<int>(34));
    }

    SUBCASE("Grab Block Deep") {
        CHECK(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.grabBlock<int>(rootTestTest2Path);

        CHECK(val==static_cast<int>(5));
    }

    SUBCASE("Grab Block Vector Order") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        CHECK(space.insert("/test", 7) == true);
        CHECK(space.insert("/test", 8) == true);
        CHECK(space.grabBlock<int>("/test")==5);
        CHECK(space.grabBlock<int>("/test")==6);
        CHECK(space.grabBlock<int>("/test")==7);
        CHECK(space.grabBlock<int>("/test")==8);
    }

    /*SUBCASE("Grab Block space") {
        PathSpaceTE space2 = PathSpace{};
        CHECK(space.insert("/space", space2) == true);
        nlohmann::json json;
        json["space"] = nlohmann::json::array({nlohmann::json()});
        CHECK(space.toJSON() == json);

        CHECK(space.insert("/space/val", 34) == true);
        json["space"][0]["val"] = {34};
        CHECK(space.toJSON() == json);
    }

    SUBCASE("Grab Block coroutine") {
        CHECK(space.insert(rootTestPath, [&space]() -> Coroutine {
            for(auto i = 0; i < 10; ++i)
                co_yield i;
            space.insert("/finished", 1);
        }) == true);
        //space.grabBlockBlock("/finished");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        nlohmann::json json;
        json["finished"] = {1};
        json["test"] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        CHECK(space.toJSON() == json);
    }*/
}