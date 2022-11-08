#include <catch.hpp>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_read(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template read<InTRRRC>(path);
    REQUIRE(val.has_value());
    REQUIRE(val.value()==testValue);
}

TEST_CASE("PathSpace Read") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Read Simple") {
        REQUIRE(space.insert("/test", 5) == true);
        REQUIRE(space.insert("/test", 6) == true);
        auto const val = space.read<int>("/test");
        REQUIRE(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        REQUIRE(val2.value() == 5);
        auto const val3 = space.read<int>("/test");
        REQUIRE(val3.value() == 6);
    }

    SECTION("Read Empty") {
        auto const val3 = space.read<int>("/test");
        REQUIRE(!val3.has_value());
    }

    SECTION("Read Misc") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.read<int>(rootTestPath) == 5);

        REQUIRE(space.insert(rootTestPath2, "hello") == true);
        auto val = space.read<std::string>(rootTestPath2);
        REQUIRE(val.has_value());
        REQUIRE(std::string(val.value()) == "hello");

        REQUIRE(space.grab<int>(rootTestPath).value_or(0) == 5);
        REQUIRE(space.insert(rootTestPath, 234) == true);
        REQUIRE(space.read<int>(rootTestPath) == 234);
    }

    SECTION("Read POD") {
        POD const pod1;
        REQUIRE(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.read<POD>(rootTestPath);
        REQUIRE(pod2.has_value());
        REQUIRE(pod1 == pod2.value());
    }

    SECTION("Read NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.read<NonTrivial>(rootTestPath);
        REQUIRE(nt2.has_value());
        REQUIRE(nt == nt2.value());
    }

    SECTION("Read NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        REQUIRE(space.insert(rootTestPath, nt) == true);
        auto const val = space.read<NonTrivialJS>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(nt == val.value());
    }

    SECTION("Read Bool") {
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        REQUIRE(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.read<bool>(rootTestPath);
        space.grab<bool>(rootTestPath);
        auto const v2 = space.read<bool>(rootTestPath);
        space.grab<bool>(rootTestPath);
        auto const v3 = space.read<bool>(rootTestPath);
        REQUIRE(v1.has_value());
        REQUIRE(v2.has_value());
        REQUIRE(v3.has_value());
        REQUIRE(v1.value()==true);
        REQUIRE(v2.value()==false);
        REQUIRE(v3.value()==true);
    }

    SECTION("Read Signed Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.read<signed char>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Read Unsigned Char") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.read<unsigned char>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Read wchar_t") {
        REQUIRE(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.read<wchar_t>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()=='C');
    }

    SECTION("Read Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.read<short>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==43);
    }

    SECTION("Read Unsigned Short") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.read<unsigned short>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned short>(-43));
    }

    SECTION("Read Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.read<int>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<int>(43));
    }

    SECTION("Read Unsigned Int") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.read<unsigned int>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned int>(-43));
    }

    SECTION("Read Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.read<long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long>(43));
    }

    SECTION("Read Unsigned Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.read<unsigned long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned long>(-43));
    }

    SECTION("Read Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.read<long long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long long>(43));
    }

    SECTION("Read Unsigned Long Long") {
        REQUIRE(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.read<unsigned long long>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<unsigned long long>(-43));
    }

    SECTION("Read Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.read<double>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<double>(5.45));
    }

    SECTION("Read Long Double") {
        REQUIRE(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.read<long double>(rootTestPath);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<long double>(5.45));
    }

    SECTION("Read Multiple Types") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        REQUIRE(space.insert(rootTestPath, "hello") == true);
        REQUIRE(space.insert(rootTestPath, "hello2") == true);
        REQUIRE(space.insert(rootTestPath, 34) == true);
        check_read(space, rootTestPath, static_cast<int>(5));
        space.grab<int>(rootTestPath);
        check_read(space, rootTestPath, static_cast<std::string>("hello"));
        space.grab<std::string>(rootTestPath);
        check_read(space, rootTestPath, static_cast<std::string>("hello2"));
        space.grab<std::string>(rootTestPath);
        check_read(space, rootTestPath, static_cast<int>(34));
    }

    SECTION("Read deep") {
        REQUIRE(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.read<int>(rootTestTest2Path);
        REQUIRE(val.has_value());
        REQUIRE(val.value()==static_cast<int>(5));
    }

    SECTION("Read Empty Space") {
        REQUIRE(space.insert("/space", PathSpaceTE{PathSpace{}}) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        REQUIRE(valOpt.value() == PathSpaceTE{PathSpace{}});
    }

    SECTION("Read Space") {
        REQUIRE(space.insert("/space/val", 123) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        auto val = valOpt.value();
        REQUIRE(val.read<int>("/val") == 123);
    }

    SECTION("Read Mixed Space/Builtin") {
        REQUIRE(space.insert("/space/val", 123) == true);
        REQUIRE(space.insert("/val", 321) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        REQUIRE(valOpt.has_value());
        auto val = valOpt.value();
        REQUIRE(val.read<int>("/val").value_or(0) == 123);
        REQUIRE(space.read<int>("/val").value_or(0) == 321);
    }
}

TEST_CASE("PathSpace Read Multithreaded") {
    PathSpaceTE space = PathSpace{};

    SECTION("Minimal Insert") {
        space.insert("/coro", []() -> Coroutine {co_return 0;});
    }
    SECTION("Minimal Insert With insert") {
        space.insert("/coro", [&space]() -> Coroutine {space.insert("/finished", 1);co_return 0;});
    }
    SECTION("Read Coroutine Simple") {
        REQUIRE(space.insert("/coro", [&space]() -> Coroutine {
            space.insert("/finished", 1);
            co_return 0;
        }) == true);
        REQUIRE(space.readBlock<int>("/finished")==1);
    }

    SECTION("Check Coro Return") {
        REQUIRE(space.insert("/coro", [&space]() -> Coroutine {
            space.insert("/finished", 1);
            co_return 456;
        }) == true);
        REQUIRE(space.readBlock<int>("/finished")==1);
        REQUIRE(space.readBlock<int>("/coro")==456);
    }

    /*SECTION("Read Coroutine Result") {
        LOG("Read Coroutine Result start");
        REQUIRE(space.insert("/coro", [&space]() -> Coroutine {
            LOG("Starting coro")
            for(auto i = 0; i < 5; ++i) {
                LOG("co_yield {}", i)
                co_yield i;
            }
            LOG("Inserting /finished")
            space.insert("/finished", 1);
            LOG("Inserted /finished")
        }) == true);
        LOG("Trying to Read /finished")
        REQUIRE(space.readBlock<int>("/finished")==1);
        LOG("Readbed /finished")
        for(auto i = 0; i < 5; ++i) {
            REQUIRE(space.read<int>("/coro").value_or(-1)==i);
            space.grab<int>("/coro");
        }
    }*/
}