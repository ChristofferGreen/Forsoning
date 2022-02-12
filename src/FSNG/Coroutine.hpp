#pragma once
#include "FSNG/Data.hpp"

#include <experimental/coroutine>

namespace FSNG {
struct Coroutine {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;

    Coroutine(handle_type h): coro(h) {}
    handle_type coro;

    ~Coroutine() {
        if ( coro ) coro. destroy();
    }
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator= (const Coroutine&) = delete;
    Coroutine(Coroutine&& oth) noexcept : coro(oth.coro){
        oth.coro = nullptr;
    }
    Coroutine& operator = (Coroutine&& oth) noexcept {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    Data&& getValue() {
        return std::move(coro.promise().current_value);
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
            return Coroutine {handle_type::from_promise(*this)};
        }
        auto return_void() {
            return std::experimental::suspend_never{};
        }
        auto yield_value(Data &&value) {
            current_value = std::move(value);
            return std::experimental::suspend_always{};
        }
        void unhandled_exception() {
            std::exit(1);
        }
        Data current_value;
    };
};
}