#pragma once
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Esprit.hpp"
#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

#ifdef LOG_FORGE
#define LOG_F LOG
#define LogRAII_F LogRAII
#else
#define LOG_F(...)
#define LogRAII_F(...) 0
#endif

namespace FSNG {
struct Forge {
    ~Forge() {
        this->isAlive = false;
        this->eschelon.shutdown();
        auto readLock = std::shared_lock<std::shared_mutex>(this->mutex);
        for(auto &thread : this->threads)
            thread.join();
    }

    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter=[](Data const &data){}) -> Ticket {
        auto const ticket = this->eschelon.newTicket();
        this->esprit.activate(ticket);
        this->eschelon.add(ticket, coroutineFun, inserter);
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        auto const hasFreeThreads = this->esprit.nbrActive()>this->threads.size();
        auto const lessAllocatedThreadsThanMaximum = this->threads.size()<(std::thread::hardware_concurrency()*2);
        if(hasFreeThreads && lessAllocatedThreadsThanMaximum)
            this->threads.emplace_back(std::thread(&Forge::executor, this));
        return ticket;
    }

    auto remove(Ticket const &ticket) -> void {
        this->eschelon.remove(ticket);
    }

    auto wait(Ticket const &ticket) -> void {
        this->esprit.wait(ticket);
    }

    auto executor() -> void {
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
    Esprit esprit;
    std::vector<std::thread> threads;
    mutable std::shared_mutex mutex;
};
}