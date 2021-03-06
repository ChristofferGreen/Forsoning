#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_grab(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template grab<InTRRRC>(path);
    CHECK(val.has_value());
    CHECK(val.value()==testValue);
}

TEST_CASE("PathSpace Grab" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Grab Simple") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        CHECK(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        CHECK(val2.value() == 6);
    }

    SUBCASE("Grab Empty") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.grab<int>("/test");
        CHECK(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        CHECK(val2.value() == 6);
        auto const val3 = space.grab<int>("/test");
        CHECK(!val3.has_value());
    }

    SUBCASE("Grab") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.grab<int>(rootTestPath) == 5);

        CHECK(space.insert(rootTestPath2, "hello") == true);
        auto val = space.grab<std::string>(rootTestPath2);
        CHECK(val.has_value());
        CHECK(std::string(val.value()) == "hello");

        CHECK(space.insert(rootTestPath, 234) == true);
        CHECK(space.grab<int>(rootTestPath) == 234);
    }

    SUBCASE("Grab POD") {
        POD const pod1;
        CHECK(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.grab<POD>(rootTestPath);
        CHECK(pod2.has_value());
        CHECK(pod1 == pod2.value());
    }

    SUBCASE("Grab NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.grab<NonTrivial>(rootTestPath);
        CHECK(nt2.has_value());
        CHECK(nt == nt2.value());
    }

    SUBCASE("Grab NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);
        auto const val = space.grab<NonTrivialJS>(rootTestPath);
        CHECK(val.has_value());
        CHECK(nt == val.value());
    }

    SUBCASE("Grab Bool") {
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.grab<bool>(rootTestPath);
        auto const v2 = space.grab<bool>(rootTestPath);
        auto const v3 = space.grab<bool>(rootTestPath);
        CHECK(v1.has_value());
        CHECK(v2.has_value());
        CHECK(v3.has_value());
        CHECK(v1.value()==true);
        CHECK(v2.value()==false);
        CHECK(v3.value()==true);
    }

    SUBCASE("Grab Signed Char") {
        CHECK(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.grab<signed char>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Grab Unsigned Char") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.grab<unsigned char>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Grab wchar_t") {
        CHECK(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.grab<wchar_t>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Grab Short") {
        CHECK(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.grab<short>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==43);
    }

    SUBCASE("Grab Unsigned Short") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.grab<unsigned short>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned short>(-43));
    }

    SUBCASE("Grab Int") {
        CHECK(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.grab<int>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<int>(43));
    }

    SUBCASE("Grab Unsigned Int") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.grab<unsigned int>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned int>(-43));
    }

    SUBCASE("Grab Long") {
        CHECK(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.grab<long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long>(43));
    }

    SUBCASE("Grab Unsigned Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.grab<unsigned long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned long>(-43));
    }

    SUBCASE("Grab Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.grab<long long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long long>(43));
    }

    SUBCASE("Grab Unsigned Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.grab<unsigned long long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned long long>(-43));
    }

    SUBCASE("Grab Double") {
        CHECK(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.grab<double>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<double>(5.45));
    }

    SUBCASE("Grab Long Double") {
        CHECK(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.grab<long double>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long double>(5.45));
    }

    SUBCASE("Grab Multiple Types") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.insert(rootTestPath, "hello") == true);
        CHECK(space.insert(rootTestPath, "hello2") == true);
        CHECK(space.insert(rootTestPath, 34) == true);
        check_grab(space, rootTestPath, static_cast<int>(5));
        check_grab(space, rootTestPath, static_cast<std::string>("hello"));
        check_grab(space, rootTestPath, static_cast<std::string>("hello2"));
        check_grab(space, rootTestPath, static_cast<int>(34));
    }

    SUBCASE("Grab deep") {
        CHECK(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.grab<int>(rootTestTest2Path);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<int>(5));
    }

    SUBCASE("Grab Empty Space") {
        CHECK(space.insert("/space", PathSpaceTE{PathSpace{}}) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        CHECK(valOpt.value() == PathSpaceTE{PathSpace{}});
    }

    SUBCASE("Grab Space") {
        CHECK(space.insert("/space/val", 123) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        auto val = valOpt.value();
        CHECK(val.grab<int>("/val") == 123);
    }

    SUBCASE("Grab Mixed Space/Builtin") {
        CHECK(space.insert("/space/val", 123) == true);
        CHECK(space.insert("/val", 321) == true);
        auto const valOpt = space.grab<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        auto val = valOpt.value();
        CHECK(val.grab<int>("/val").value_or(0) == 123);
        CHECK(space.grab<int>("/val").value_or(0) == 321);
    }
}

TEST_CASE("PathSpace Grab Multithreaded" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Grab Coroutine Result") {
        LOG("Grab Coroutine Result start");
        CHECK(space.insert("/coro", [&space]() -> Coroutine {
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
        CHECK(space.grabBlock<int>("/finished")==1);
        LOG("Grabbed /finished")
        for(auto i = 0; i < 5; ++i)
            CHECK(space.grab<int>("/coro").value_or(-1)==i);
    }

    /*SUBCASE("Grab Coroutine") {
        CHECK(space.insert("/coro", [&space]() -> Coroutine {
            space.grabBlock<int>("/exit_coro");
            co_yield 1;
        }) == true);
        auto coroOpt = space.grabBlock<experimental/coroutine>("/coro");
    }*/
}