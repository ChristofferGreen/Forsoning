#pragma once
#include "FSNG/Task.hpp"

#include <unordered_map>

namespace FSNG {
struct TaskProcessor {
    auto add(void *id, std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) {
        auto coroutine = coroutineFun();
        while(coroutine.next())
            inserter(coroutine.getValue());
    }
private:
    std::unordered_map<void*, Task> tasks;
};
}