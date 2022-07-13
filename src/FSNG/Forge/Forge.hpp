#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Esprit.hpp"
#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

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
        LOG("eschelon.wait for ticket {}", ticket);
        this->esprit.wait(ticket);
        LOG("Forge::wait done for ticket {}", ticket);
    }

    auto executor(int const id) -> void {
        while(!spdlog::get("file")) {}
        LOG("Thread: {} started executing.", id);
        while(this->isAlive) {
            LOG("Thread: {} waiting for task, tasks available: {}", id, this->eschelon.size());
            if(auto task = this->eschelon.popWait()) {
                LOG("Thread: {} got task", id);
                auto coroutine = task.value().fun();
                while(coroutine.next()) {
                    task.value().inserter(coroutine.getValue());
                    LOG("Thread: {} inserting coroutine value", id);
                }
                LOG("Thread: {} finished task", id);
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