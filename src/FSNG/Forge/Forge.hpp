#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"


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
        while(this->alive) {
            auto task = this->eschelon.popWait();
            /*auto coroutine = task.fun();
            while(coroutine.next())
                task.inserter(coroutine.getValue());*/
        }
    }

private:
    Hearth hearth;
    Eschelon eschelon;
    std::atomic<bool> alive = true;
};
}