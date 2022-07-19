#include <doctest.h>

#include "PathSpace.hpp"
#include "test_utils.hpp"

using namespace FSNG;

auto check_read(auto &space, auto const &path, auto const &testValue) {
    using InT = decltype(testValue);
    using InTRR = typename std::remove_reference<InT>::type;
    using InTRRRC = typename std::remove_const<InTRR>::type;
    auto const val = space.template read<InTRRRC>(path);
    CHECK(val.has_value());
    CHECK(val.value()==testValue);
}

TEST_CASE("PathSpace Read" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Read Simple") {
        CHECK(space.insert("/test", 5) == true);
        CHECK(space.insert("/test", 6) == true);
        auto const val = space.read<int>("/test");
        CHECK(val.value() == 5);
        auto const val2 = space.grab<int>("/test");
        CHECK(val2.value() == 5);
        auto const val3 = space.read<int>("/test");
        CHECK(val3.value() == 6);
    }

    SUBCASE("Read Empty") {
        auto const val3 = space.read<int>("/test");
        CHECK(!val3.has_value());
    }

    SUBCASE("Read Misc") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.read<int>(rootTestPath) == 5);

        CHECK(space.insert(rootTestPath2, "hello") == true);
        auto val = space.read<std::string>(rootTestPath2);
        CHECK(val.has_value());
        CHECK(std::string(val.value()) == "hello");

        CHECK(space.grab<int>(rootTestPath).value_or(0) == 5);
        CHECK(space.insert(rootTestPath, 234) == true);
        CHECK(space.read<int>(rootTestPath) == 234);
    }

    SUBCASE("Read POD") {
        POD const pod1;
        CHECK(space.insert(rootTestPath, pod1) == true);
        auto const pod2 = space.read<POD>(rootTestPath);
        CHECK(pod2.has_value());
        CHECK(pod1 == pod2.value());
    }

    SUBCASE("Read NonTrivial Class") {
        NonTrivial nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);

        auto const nt2 = space.read<NonTrivial>(rootTestPath);
        CHECK(nt2.has_value());
        CHECK(nt == nt2.value());
    }

    SUBCASE("Read NonTrivial Class JS") {
        NonTrivialJS nt;
        nt.b = {1, 2, 3};
        CHECK(space.insert(rootTestPath, nt) == true);
        auto const val = space.read<NonTrivialJS>(rootTestPath);
        CHECK(val.has_value());
        CHECK(nt == val.value());
    }

    SUBCASE("Read Bool") {
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(false)) == true);
        CHECK(space.insert(rootTestPath, static_cast<bool>(true)) == true);
        auto const v1 = space.read<bool>(rootTestPath);
        space.grab<bool>(rootTestPath);
        auto const v2 = space.read<bool>(rootTestPath);
        space.grab<bool>(rootTestPath);
        auto const v3 = space.read<bool>(rootTestPath);
        CHECK(v1.has_value());
        CHECK(v2.has_value());
        CHECK(v3.has_value());
        CHECK(v1.value()==true);
        CHECK(v2.value()==false);
        CHECK(v3.value()==true);
    }

    SUBCASE("Read Signed Char") {
        CHECK(space.insert(rootTestPath, static_cast<signed char>('C')) == true);
        auto const val = space.read<signed char>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Read Unsigned Char") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned char>('C')) == true);
        auto const val = space.read<unsigned char>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Read wchar_t") {
        CHECK(space.insert(rootTestPath, static_cast<wchar_t>('C')) == true);
        auto const val = space.read<wchar_t>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()=='C');
    }

    SUBCASE("Read Short") {
        CHECK(space.insert(rootTestPath, static_cast<short>(43)) == true);
        auto const val = space.read<short>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==43);
    }

    SUBCASE("Read Unsigned Short") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned short>(-43)) == true);
        auto const val = space.read<unsigned short>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned short>(-43));
    }

    SUBCASE("Read Int") {
        CHECK(space.insert(rootTestPath, static_cast<int>(43)) == true);
        auto const val = space.read<int>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<int>(43));
    }

    SUBCASE("Read Unsigned Int") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned int>(-43)) == true);
        auto const val = space.read<unsigned int>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned int>(-43));
    }

    SUBCASE("Read Long") {
        CHECK(space.insert(rootTestPath, static_cast<long>(43)) == true);
        auto const val = space.read<long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long>(43));
    }

    SUBCASE("Read Unsigned Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long>(-43)) == true);
        auto const val = space.read<unsigned long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned long>(-43));
    }

    SUBCASE("Read Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<long long>(43)) == true);
        auto const val = space.read<long long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long long>(43));
    }

    SUBCASE("Read Unsigned Long Long") {
        CHECK(space.insert(rootTestPath, static_cast<unsigned long long>(-43)) == true);
        auto const val = space.read<unsigned long long>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<unsigned long long>(-43));
    }

    SUBCASE("Read Double") {
        CHECK(space.insert(rootTestPath, static_cast<double>(5.45)) == true);
        auto const val = space.read<double>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<double>(5.45));
    }

    SUBCASE("Read Long Double") {
        CHECK(space.insert(rootTestPath, static_cast<long double>(5.45)) == true);
        auto const val = space.read<long double>(rootTestPath);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<long double>(5.45));
    }

    SUBCASE("Read Multiple Types") {
        CHECK(space.insert(rootTestPath, 5) == true);
        CHECK(space.insert(rootTestPath, "hello") == true);
        CHECK(space.insert(rootTestPath, "hello2") == true);
        CHECK(space.insert(rootTestPath, 34) == true);
        check_read(space, rootTestPath, static_cast<int>(5));
        space.grab<int>(rootTestPath);
        check_read(space, rootTestPath, static_cast<std::string>("hello"));
        space.grab<std::string>(rootTestPath);
        check_read(space, rootTestPath, static_cast<std::string>("hello2"));
        space.grab<std::string>(rootTestPath);
        check_read(space, rootTestPath, static_cast<int>(34));
    }

    SUBCASE("Read deep") {
        CHECK(space.insert(rootTestTest2Path, 5) == true);
        auto const val = space.read<int>(rootTestTest2Path);
        CHECK(val.has_value());
        CHECK(val.value()==static_cast<int>(5));
    }

    SUBCASE("Read Empty Space") {
        CHECK(space.insert("/space", PathSpaceTE{PathSpace{}}) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        CHECK(valOpt.value() == PathSpaceTE{PathSpace{}});
    }

    SUBCASE("Read Space") {
        CHECK(space.insert("/space/val", 123) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        auto val = valOpt.value();
        CHECK(val.read<int>("/val") == 123);
    }

    SUBCASE("Read Mixed Space/Builtin") {
        CHECK(space.insert("/space/val", 123) == true);
        CHECK(space.insert("/val", 321) == true);
        auto const valOpt = space.read<PathSpaceTE>("/space");
        CHECK(valOpt.has_value());
        auto val = valOpt.value();
        CHECK(val.read<int>("/val").value_or(0) == 123);
        CHECK(space.read<int>("/val").value_or(0) == 321);
    }
}

TEST_CASE("PathSpace Read Multithreaded" * doctest::timeout(2.0)) {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SUBCASE("Read Coroutine Result") {
        LOG("Read Coroutine Result start");
        CHECK(space.insert("/coro", [&space]() -> Coroutine {
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
        CHECK(space.readBlock<int>("/finished")==1);
        LOG("Readbed /finished")
        for(auto i = 0; i < 5; ++i) {
            CHECK(space.read<int>("/coro").value_or(-1)==i);
            space.grab<int>("/coro");
        }
    }

    /*SUBCASE("Read Coroutine") {
        CHECK(space.insert("/coro", [&space]() -> Coroutine {
            space.readBlock<int>("/exit_coro");
            co_yield 1;
        }) == true);
        auto coroOpt = space.readBlock<int>("/coro");
    }*/
}