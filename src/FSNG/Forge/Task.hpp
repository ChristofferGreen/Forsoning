#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Forge/Ticket.hpp"

#include <functional>

namespace FSNG {
struct Task {
    Ticket ticket;
    std::function<Coroutine()> fun = []()->Coroutine{co_return 0;};
    std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> inserter = [](Data const &data, Ticket const &ticket, PathSpaceTE &space){};
    PathSpaceTE *space = nullptr;
    Path path;
};
}