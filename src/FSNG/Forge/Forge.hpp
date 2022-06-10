#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"


namespace FSNG {
struct Forge {
    Forge() {

    }

    ~Forge() {

    }

    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> Ticket {

        return {};
    }

    auto remove(Ticket const &ticket) -> void {

    }

    auto executor() -> void {

    }

private:

};
}