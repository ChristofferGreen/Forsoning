#pragma once
#include <algorithm>

namespace FSNG {
struct Hearth {
    template<typename T, typename U>
    Hearth(T t, U u) {
        auto const nbrThreads = std::max(static_cast<unsigned int>(32), std::thread::hardware_concurrency()*2);
        for(auto i = 0; i < nbrThreads; ++i)
            this->threads.emplace_back(t, u);
    }

    ~Hearth() {
        for(auto &thread : this->threads)
            thread.join();
    }

private:
    std::vector<std::thread> threads;
};
}