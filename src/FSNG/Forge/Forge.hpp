#pragma once
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/Forge/Eschelon.hpp"
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
        return this->eschelon.add(coroutineFun, inserter);
    }

    auto remove(Ticket const &ticket) -> void {
        if(!this->eschelon.remove(ticket))
            this->hearth.remove(ticket);
    }

    auto wait(Ticket const &ticket) -> void {
        this->eschelon.wait(ticket);
        this->hearth.wait(ticket);
    }

    auto executor(int const id) -> void {
        while(!spdlog::get("file")) {}
        LOG("Thread: {} started executing.", id);
        while(this->isAlive) {
            LOG("Thread: {} waiting for task, tasks available: {}", id, this->eschelon.size());
            if(auto task = this->eschelon.popWait()) {
                LOG("Thread: {} got task", id);
                this->hearth.starting(task.value().ticket);
                auto coroutine = task.value().fun();
                while(!coroutine.done()) {
                    coroutine.next();
                    task.value().inserter(coroutine.getValue());
                    LOG("Thread: {} inserting coroutine value", id);
                }
                this->hearth.finished(task.value().ticket);
                LOG("Thread: {} finished task", id);
            }
        }
    }

private:
    std::atomic<bool> isAlive = true;
    Eschelon eschelon;
    Hearth hearth;
};
}