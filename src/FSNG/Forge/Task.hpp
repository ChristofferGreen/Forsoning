#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Forge/Ticket.hpp"

#include <functional>
#include <variant>

namespace FSNG {
struct Task {
    Task() = default;
    Task(Ticket const &ticket, auto const &fun, 
         std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> const &inserter, PathSpaceTE *space, Path const &path) :
            ticket(ticket), fun(fun), inserter(inserter), space(space), path(path) {}

    Ticket ticket;
    std::variant<std::function<Coroutine()>, std::function<CoroutineVoid()>> fun;
    std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> inserter = [](Data const &data, Ticket const &ticket, PathSpaceTE &space){};
    PathSpaceTE *space = nullptr;
    Path path;
    bool isRunning = false;
};
}