#include <doctest.h>

#include <iostream>
#include <map>
#include <experimental/coroutine>
#include <memory>

namespace CoRet {

template <typename T>
struct MyFuture {
    std::shared_ptr<T> value;
    MyFuture(std::shared_ptr<T> p): value(p) {}
    ~MyFuture() {}
    T get () {
        return *value;
    }

    struct promise_type {
        std::shared_ptr<T> ptr = std::make_shared<T>();
        ~promise_type() {}
        MyFuture<T> get_return_object() {
            return ptr;
        }
        void return_value(T v) {
            *ptr = v;
        }
        std::experimental::suspend_never initial_suspend() {
            return {};
        }
        std::experimental::suspend_never final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
            std::exit(1);
        }
    };
};

MyFuture<int> createFuture() {
    co_return 2021;
}
}


namespace CoYield {
template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;

    Generator(handle_type h): coro(h) {}
    handle_type coro;

    ~Generator() {
        if ( coro ) coro. destroy();
    }
    Generator(const Generator&) = delete;
    Generator& operator= (const Generator&) = delete;
    Generator (Generator&& oth) noexcept : coro(oth.coro){
        oth.coro = nullptr;
    }
    Generator& operator = (Generator&& oth) noexcept {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    T getValue() {
        return coro.promise().current_value;
    }
    bool next() {
        coro.resume();
        return not coro.done();
    }
    struct promise_type {
        promise_type() = default;
        ~promise_type() = default;

        auto initial_suspend() {
            return std::experimental::suspend_always{};
        }
        auto final_suspend() noexcept {
            return std::experimental::suspend_always{};
        }
        auto get_return_object() {
            return Generator {handle_type::from_promise(*this)};
        }
        auto return_void() {
            return std::experimental::suspend_never{};
        }
        auto yield_value(const T value) {
            current_value = value;
            return std::experimental::suspend_always{};
        }
        void unhandled_exception() {
            std::exit(1);
        }
        T current_value;
    };
};

Generator<int> getNext(int start = 0, int step = 1) {
    auto value = start;
    while (true) {
        co_yield value;
        value += step;
    }
}
}

namespace CoAwait {
struct Job {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>; handle_type coro;
    Job(handle_type h): coro(h){}
    ~Job() {
        if ( coro ) coro.destroy(); 
    }
    void start() { 
        coro.resume();
    }
    struct promise_type {
        auto get_return_object() {
            return Job{handle_type::from_promise(*this)}; 
        }
        std::experimental::suspend_always initial_suspend() { 
            std::cout << " Preparing job" << '\n';
            return {};
        }
        std::experimental::suspend_always final_suspend() noexcept {
            std::cout << " Performing job" << '\n';
            return {}; 
        }
        void return_void() {}
        void unhandled_exception() {}
    };
};
Job prepareJob() {
    co_await std::experimental::suspend_never();
}
}

TEST_CASE("Coroutine Syntax Test") {
    SUBCASE("Single co_return") {
        auto fut = CoRet::createFuture();
        CHECK(fut.get()==2021);
   }
    SUBCASE("co_yield") {
        auto gen = CoYield::getNext();
        for (int i = 0; i<= 10; ++i) {
            gen.next();
            std::cout << gen.getValue() << std::endl;
        }
   }
   SUBCASE("Awaitable") {
       std::cout <<  "Before job" << '\n';
       auto job = CoAwait::prepareJob();
       job.start();
       std::cout <<  "After job" <<  '\n';
   }
}
