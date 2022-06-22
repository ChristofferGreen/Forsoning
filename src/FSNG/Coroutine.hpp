#pragma once
#include "FSNG/Data.hpp"

#include <experimental/coroutine>

namespace FSNG {
struct Coroutine {
    struct promise_type;
    using handle_type = std::experimental::coroutine_handle<promise_type>;

    Coroutine() = default;
    Coroutine(handle_type h) : coro(h) {}
    Coroutine(const Coroutine&) = delete;
    Coroutine& operator= (const Coroutine&) = delete;
    Coroutine(Coroutine&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
    }
    Coroutine& operator = (Coroutine&& other) noexcept {
        coro = other.coro;
        other.coro = nullptr;
        return *this;
    }
    ~Coroutine() {
        if(coro)
            coro.destroy();
    }

    auto getValue() {
        return std::move(coro.promise().current_value);
    }

    auto done() -> bool {
        return coro.done();
    }

    auto next() {
        coro.resume();
        return !coro.done();
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

        auto yield_value(Data &&value) {
            current_value = std::move(value);
            return std::experimental::suspend_always{};
        }

        auto return_void() {
            return std::experimental::suspend_always{};
        }

        void unhandled_exception() {
            std::exit(1);
        }

        Data current_value;
    };
    handle_type coro;
};
}