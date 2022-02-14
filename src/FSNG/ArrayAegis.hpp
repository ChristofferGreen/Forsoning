#pragma once

#include <shared_mutex>
#include <condition_variable>

namespace FSNG {
template<typneame T>
struct ArrayAegis {
    ArrayAegis() = default;
    ArrayAegis(ArrayAegis const &other) : data(other.data) {}

    auto readMutex() const {
        return std::shared_lock<std::shared_mutex>(this->mutex);
    }

    auto writeMutex() const {
        return std::lock_guard<std::shared_mutex>(this->mutex);
    }

    T data;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}