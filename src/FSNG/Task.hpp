#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"

#include <functional>

namespace FSNG {
struct Task {
    void *id = nullptr;
    std::function<Coroutine()> fun;
    std::function<void(Data const &data)> inserter;
};
}