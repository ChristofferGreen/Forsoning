#pragma once
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Esprit.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"

#include <iostream>

#ifdef LOG_FORGE
#define LOG_F(...) LOG("<TAG:Forge>" __VA_ARGS__)
#define LogRAII_F(...) LogRAII("<TAG:Echelon>" __VA_ARGS__)
#else
#define LOG_F(...)
#define LogRAII_F(...) 0
#endif

namespace FSNG {
struct Forge {
    auto static CreateSingleton() -> std::unique_ptr<Forge> {
        auto ptr = std::make_unique<Forge>();
        Forge::instance_ = &(*ptr);
        return ptr;
    }

    auto static instance() -> Forge* {
        return Forge::instance_;
    }

    ~Forge() {
        this->isAlive = false;
        this->eschelon.shutdown();
        auto readLock = std::shared_lock<std::shared_mutex>(this->mutex);
        for(auto &thread : this->threads)
            thread.join();
    }

    auto add(auto const &coroutineFun, std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> const &inserter, PathSpaceTE &space, Path const &path) -> Ticket {
        auto const ticket = this->eschelon.newTicket();
        this->esprit.activate(ticket, space);
        this->eschelon.add(ticket, coroutineFun, inserter, space, path);
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

    auto wait(PathSpaceTE &space) -> void {
        this->esprit.wait(space);
    }
    
    auto removeAndWait(PathSpaceTE &space) -> void {
        this->eschelon.remove(space);
        this->esprit.wait(space);
    }

    auto executor() -> void {
        while(this->isAlive) {
            if(auto task = this->eschelon.popWait()) {
                if(task.value().fun)
                    this->loop(task.value().fun(), task);
                else
                    this->loop(task.value().funv(), task);
                LOG_F("deactivating task");
                task.value().space->grab<void>(task.value().path, task.value().ticket);
                this->esprit.deactivate(task.value().ticket, *task.value().space);
                LOG_F("Task done");
            }
        }
    }

private:
    void loop(auto &&coroutine, auto &task) {
        bool shouldGoAgain = false;
        do {
            LOG_F("Task (re)starting");
            shouldGoAgain = coroutine.next();
            LOG_F("Task finished, restart: {}", shouldGoAgain);
            if(coroutine.hasValue()) {
                LOG_F("Inserting value");
                task.value().inserter(coroutine.getValue(), task.value().ticket, *task.value().space);
                LOG_F("Inserted value");
            }
        } while(shouldGoAgain);
    }
    inline static Forge* instance_;
    std::atomic<bool> isAlive = true;
    mutable std::shared_mutex mutex;
    std::vector<std::thread> threads;
    Eschelon eschelon;
    Esprit esprit;
};
}