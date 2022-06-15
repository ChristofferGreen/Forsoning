#pragma once
#include <set>

namespace FSNG {
struct Hearth {
    template<typename T, typename U>
    Hearth(T t, U u) {
        auto const nbrThreads = std::max(static_cast<unsigned int>(32), std::thread::hardware_concurrency()*2);
        for(auto i = 0; i < nbrThreads; ++i)
            this->threads.emplace_back(t, u, i);
    }

    ~Hearth() {
        for(auto &thread : this->threads)
            thread.join();
    }

    auto starting(Ticket const &ticket) {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->tickets.insert(ticket);
    }

    auto finished(Ticket const &ticket) {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        this->tickets.erase(ticket);
        this->condition.notify_all();
    }

    auto wait(Ticket const &ticket) -> void {
        auto readLock = std::shared_lock(this->mutex);
        while(this->tickets.contains(ticket))
            this->condition.wait(readLock);
    }

    auto remove(Ticket const &ticket) -> void {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        // ??????
    }

private:
    std::vector<std::thread> threads;
    std::set<Ticket> tickets;
    mutable std::shared_mutex mutex;
    mutable std::condition_variable_any condition;
};
}