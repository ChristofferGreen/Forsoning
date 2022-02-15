#pragma once

namespace FSNG {
struct TaskProcessor {
    auto add(void *id, std::unique_ptr<std::function<Coroutine()>> const &coroutinePtr, std::function<void(Data const &data)> const &inserter) {
        if(!coroutinePtr)
            return;
        auto coroutine = (*coroutinePtr)();
        while(coroutine.next())
            inserter(coroutine.getValue());
    }
};
}