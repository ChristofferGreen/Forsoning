#pragma once

namespace FSNG {
struct Eschelon {
    auto add(std::function<Coroutine()> const &coroutineFun, std::function<void(Data const &data)> const &inserter) -> Ticket {
        auto const writeLock = std::lock_guard<std::shared_mutex>(this->mutex);
        auto const ticket = this->currentTicket++;
        this->tasks[ticket] = Task{ticket, coroutineFun, inserter};
        this->condition.notify_one();
        return ticket;
    }

    auto popWait() -> Task {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->alive && this->tasks.size()==0)
            this->condition.wait(writeLock);
        if(!this->alive)
            return Task{};
        auto const task = *this->tasks.begin();
        this->tasks.erase(this->tasks.begin());
        return task.second;
    }
    
private:
    std::map<Ticket, Task> tasks;
    bool alive = true;
    Ticket currentTicket = 0;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}