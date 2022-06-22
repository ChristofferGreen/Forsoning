#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Forge/Ticket.hpp"

#include <functional>

namespace FSNG {
struct Task {
    Ticket ticket;
    std::function<Coroutine()> fun = []()->Coroutine{co_yield 0;};
    std::function<void(Data const &data)> inserter = [](Data const &data){};
};
}