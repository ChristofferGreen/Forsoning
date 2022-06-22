#pragma once
#include "FSNG/Forge/Ticket.hpp"

#include <set>
#include <condition_variable>
#include <thread>

namespace FSNG {
struct Hearth {
    template<typename T, typename U>
    Hearth(T t, U u) {
        auto const nbrThreads = 4;
        //auto const nbrThreads = std::max(static_cast<unsigned int>(32), std::thread::hardware_concurrency()*2);
        for(auto i = 0; i < nbrThreads; ++i)
            this->threads.push_back(std::thread(t, u, i));
    }

    ~Hearth() {
        for(auto &thread : this->threads)
            thread.join();
    }

    auto remove(Ticket const &ticket) -> void {
        auto writeLock = std::unique_lock<std::shared_mutex>(this->mutex);
        // ??????
    }

private:
    std::vector<std::thread> threads;
    mutable std::shared_mutex mutex;
};
}