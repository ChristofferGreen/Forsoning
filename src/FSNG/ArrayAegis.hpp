#pragma once

#include <shared_mutex>

namespace FSNG {
template<typename T>
struct ArrayAegis {
    ArrayAegis() = default;
    ArrayAegis(T const &t) : array(t) {}
    ArrayAegis(T &&t) : array(std::move(t)) {}
    ArrayAegis(ArrayAegis const &other) : array(other.array) {}

    auto operator=(T const &t) -> ArrayAegis<T> {
        this->array = t;
        return *this;
    }

    auto readMutex() const {
        return std::shared_lock<std::shared_mutex>(this->mutex);
    }

    auto writeMutex() const {
        return std::lock_guard<std::shared_mutex>(this->mutex);
    }

    T array;
    mutable std::shared_mutex mutex;
};
}