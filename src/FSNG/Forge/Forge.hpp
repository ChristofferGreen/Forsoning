#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"

#include <iostream>

namespace FSNG {
struct Forge {
    Forge() : hearth(&Forge::executor, this) {

    }

    ~Forge() {
        this->alive = false;
    }

    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> Ticket {

        return {};
    }

    auto remove(Ticket const &ticket) -> void {

    }

    auto executor() -> void {
         std::cout << "blah" << std::endl;
        while(this->alive) {
            std::cout << "hello" << std::endl;
            auto task = this->Eschelon.popWait();
            auto coroutine = task.fun();
            while(coroutine.next())
                task.inserter(coroutine.getValue());
        }
    }

private:
    Hearth hearth;
    Eschelon Eschelon;
    std::atomic<bool> alive = true;
};
}