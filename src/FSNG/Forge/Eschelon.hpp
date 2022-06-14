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

    auto popWait() -> std::optional<Task> {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        while(this->isAlive && this->tasks.size()==0) {
            this->waiters++;
            this->condition.wait(writeLock);
            this->waiters--;
        }
        if(!this->isAlive)
            return std::nullopt;
        auto const task = *this->tasks.begin();
        this->tasks.erase(this->tasks.begin());
        return task.second;
    }

    auto shutdown() -> void {
        this->isAlive = false;
        this->condition.notify_all();
        while(this->waiters>0) {}
    }
    
private:
    std::map<Ticket, Task> tasks;
    bool isAlive = true;
    std::atomic<int> waiters = 0;
    Ticket currentTicket = 0;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}