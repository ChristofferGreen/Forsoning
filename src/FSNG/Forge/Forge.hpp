#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Esprit.hpp"
#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

#ifdef LOG_FORGE
#define LOG_F LOG
#else
#define LOG_F(...)
#define LogRAII(...) 0
#endif

namespace FSNG {
struct Forge {
    Forge() : eschelon(), hearth(&Forge::executor, this) {}

    ~Forge() {
        this->isAlive = false;
        this->eschelon.shutdown();
    }

    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter=[](Data const &data){}) -> Ticket {
        auto const ticket = this->eschelon.newTicket();
        this->esprit.activate(ticket);
        this->eschelon.add(ticket, coroutineFun, inserter);
        return ticket;
    }

    auto remove(Ticket const &ticket) -> void {
        if(!this->eschelon.remove(ticket))
            this->hearth.remove(ticket);
    }

    auto wait(Ticket const &ticket) -> void {
        this->esprit.wait(ticket);
    }

    auto executor(int const id) -> void {
        while(this->isAlive) {
            if(auto task = this->eschelon.popWait()) {
                auto coroutine = task.value().fun();
                bool shouldGoAgain = false;
                do {
                    shouldGoAgain = coroutine.next();
                    if(coroutine.hasValue())
                        task.value().inserter(coroutine.getValue());
                } while(shouldGoAgain);
                this->esprit.deactivate(task.value().ticket);
            }
        }
    }

private:
    std::atomic<bool> isAlive = true;
    Eschelon eschelon;
    Hearth hearth;
    Esprit esprit;
};
}