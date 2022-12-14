#include <catch.hpp>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_grab(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template grab<InTRRRC>(path);
    REQUIRE(val.has_value());
    REQUIRE(val.value()==testValue);
}

TEST_CASE("PathSpace Grab") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Grab Simple") {
        REQUIRE(space.insert("/test", 5) == true);
        REQUIRE(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        REQUIRE(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        REQUIRE(val2.value() == 6);
    }

    SECTION("Grab Empty") {
        REQUIRE(space.insert("/test", 5) == true);
        REQUIRE(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        REQUIRE(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        REQUIRE(val2.value() == 6);
        auto const val3 = space.grab<int>("/test");
        REQUIRE(!val3.has_value());
    }

    SECTION("Grab") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.grab<int>(rootTestPath) == 5);

        REQUIRE(space.insert(rootTestPath2, "hello") == true);
        auto val = space.grab<std::string>(rootTestPath2);
        REQUIRE(val.has_value());
        REQUIRE(std::string(val.value()) == "hello");

        REQUIRE(space.insert(rootTestPath, 234) == true);
        REQUIRE(space.grab<int>(rootTestPath) == 234);
    }

    SECTION("Grab POD") {
        POD const pod1;
        REQUIRE(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.grab<POD>(rootTestPath);
        REQUIRE(pod2.has_value());
        REQUIRE(pod1 == pod2.value());
    }

    SECTION("Grab NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.grab<NonTrivial>(rootTestPath);
        REQUIRE(nt2.has_value());
        REQUIRE(nt == nt2.value());
    }

    SECTION("Grab NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);
        auto const val = space.grab<NonTrivialJS>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(nt == val.value());
    }

    SECTION("Grab Bool") {
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.grab<bool>(rootTestPath);
        auto const v2 = space.grab<bool>(rootTestPath);
        auto const v3 = space.grab<bool>(rootTestPath);
        REQUIRE(v1.has_value());
        REQUIRE(v2.has_value());
        REQUIRE(v3.has_value());
        REQUIRE(v1.value()==true);
        REQUIRE(v2.value()==false);
        REQUIRE(v3.value()==true);
    }

    SECTION("Grab Signed Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.grab<signed char>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Grab Unsigned Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.grab<unsigned char>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Grab wchar_t") {
        REQUIRE(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.grab<wchar_t>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Grab Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.grab<short>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==43);
    }

    SECTION("Grab Unsigned Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.grab<unsigned short>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned short>(-43));
    }

    SECTION("Grab Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.grab<int>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<int>(43));
    }

    SECTION("Grab Unsigned Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.grab<unsigned int>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned int>(-43));
    }

    SECTION("Grab Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.grab<long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long>(43));
    }

    SECTION("Grab Unsigned Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.grab<unsigned long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned long>(-43));
    }

    SECTION("Grab Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.grab<long long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long long>(43));
    }

    SECTION("Grab Unsigned Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.grab<unsigned long long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned long long>(-43));
    }

    SECTION("Grab Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.grab<double>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<double>(5.45));
    }

    SECTION("Grab Long Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.grab<long double>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long double>(5.45));
    }

    SECTION("Grab Multiple Types") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.insert(rootTestPath, "hello") == true);
        REQUIRE(space.insert(rootTestPath, "hello2") == true);
        REQUIRE(space.insert(rootTestPath, 34) == true);
        check_grab(space, rootTestPath, static_cast<int>(5));
        check_grab(space, rootTestPath, static_cast<std::string>("hello"));
        check_grab(space, rootTestPath, static_cast<std::string>("hello2"));
        check_grab(space, rootTestPath, static_cast<int>(34));
    }

    SECTION("Grab deep") {
        REQUIRE(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.grab<int>(rootTestTest2Path);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<int>(5));
    }

    SECTION("Grab Empty Space") {
        REQUIRE(space.insert("/space", PathSpaceTE{PathSpace{}}) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        REQUIRE(valOpt.value() == PathSpaceTE{PathSpace{}});
    }

    SECTION("Grab Space") {
        REQUIRE(space.insert("/space/val", 123) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        auto val = valOpt.value();
        REQUIRE(val.grab<int>("/val") == 123);
    }

    SECTION("Grab Mixed Space/Builtin") {
        REQUIRE(space.insert("/space/val", 123) == true);
        REQUIRE(space.insert("/val", 321) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        auto val = valOpt.value();
        REQUIRE(val.grab<int>("/val").value_or(0) == 123);
        REQUIRE(space.grab<int>("/val").value_or(0) == 321);
    }
}

TEST_CASE("PathSpace Grab Multithreaded") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Grab Coroutine Result") {
        LOG("Grab Coroutine Result start");
        REQUIRE(space.insert("/coro", [&space]() -> Coroutine {
            LOG("Starting coro")
            for(auto i = 0; i < 5; ++i) {
                LOG("co_yield {}", i)
                co_yield i;
            }
            LOG("Inserting /finished")
            space.insert("/finished", 1);
            LOG("Inserted /finished")
            co_return 0;
        }) == true);
        LOG("Trying to grab /finished")
        REQUIRE(space.grabBlock<int>("/finished")==1);
        LOG("Grabbed /finished")
        for(auto i = 0; i < 5; ++i)
            REQUIRE(space.grabBlock<int>("/coro")==i);
    }

    //SECTION("Grab Coroutine") {
    //    REQUIRE(space.insert("/coro", [&space]() -> Coroutine {
    //        space.grabBlock<int>("/exit_coro");
    //        co_yield 1;
    //    }) == true);
    //    auto coroOpt = space.grabBlock<experimental/coroutine>("/coro");
    //}
}