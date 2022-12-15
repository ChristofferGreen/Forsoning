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

        auto return_value(Data &&value) -> void {
            currentValue = std::move(value);
        }

        void unhandled_exception() {
            std::exit(1);
        }

        std::optional<Data> currentValue;
    };
    handle_type coro;
};

struct CoroutineVoid {
    struct promise_type;
    using handle_type = STD_EXPERIMENTAL::coroutine_handle<promise_type>;

    CoroutineVoid() = default;
    CoroutineVoid(handle_type h) : coro(h) {}
    CoroutineVoid(const Coroutine&) = delete;
    CoroutineVoid& operator= (const Coroutine&) = delete;
    CoroutineVoid(CoroutineVoid&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
    }
    CoroutineVoid& operator = (CoroutineVoid&& other) noexcept {
        coro = other.coro;
        other.coro = nullptr;
        return *this;
    }
    ~CoroutineVoid() {
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
            return CoroutineVoid {handle_type::from_promise(*this)};
        }

        auto yield_value(Data &&value) {
            currentValue = std::move(value);
            return STD_EXPERIMENTAL::suspend_always{};
        }

        auto return_void() -> void {
            currentValue = std::nullopt;
        }

        void unhandled_exception() {
            std::exit(1);
        }

        std::optional<Data> currentValue;
    };
    handle_type coro;
};
}