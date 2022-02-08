#pragma once
#include <experimental/coroutine>

namespace FSNG {
template<typename T>
struct Coro {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;

    Coro(handle_type h): coro(h) {}
    handle_type coro;

    ~Coro() {
        if ( coro ) coro. destroy();
    }
    Coro(const Coro&) = delete;
    Coro& operator= (const Coro&) = delete;
    Coro(Coro&& oth) noexcept : coro(oth.coro){
        oth.coro = nullptr;
    }
    Coro& operator = (Coro&& oth) noexcept {
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
            return Coro {handle_type::from_promise(*this)};
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
}