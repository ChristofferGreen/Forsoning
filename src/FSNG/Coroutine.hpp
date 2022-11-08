#pragma once
#include "FSNG/Data.hpp"
#include "FSNG/utils.hpp"

namespace FSNG {
struct Coroutine {
    struct promise_type;
    using handle_type = STD_EXPERIMENTAL::coroutine_handle<promise_type>;

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

    auto hasValue() {
        return coro.promise().currentValue.has_value();
    }

    auto getValue() {
        auto val = std::move(coro.promise().currentValue.value());
        coro.promise().currentValue = std::nullopt;
        return val;
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
            return STD_EXPERIMENTAL::suspend_always{};
        }

        auto final_suspend() noexcept {
            return STD_EXPERIMENTAL::suspend_always{};
        }

        auto get_return_object() {
            return Coroutine {handle_type::from_promise(*this)};
        }

        auto yield_value(Data &&value) {
            currentValue = std::move(value);
            return STD_EXPERIMENTAL::suspend_always{};
        }

        auto return_value(Data &&value) {
            currentValue = std::move(value);
            return STD_EXPERIMENTAL::suspend_always{};
        }

        void unhandled_exception() {
            std::exit(1);
        }

        std::optional<Data> currentValue;
    };
    handle_type coro;
};
}