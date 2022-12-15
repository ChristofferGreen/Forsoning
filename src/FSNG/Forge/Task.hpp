#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Forge/Ticket.hpp"

#include <functional>

namespace FSNG {
struct Task {
    Task() = default;
    Task(Ticket const &ticket, ReturnsCoroutine auto const &fun, 
         std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> const &inserter, PathSpaceTE *space, Path const &path) :
            ticket(ticket), fun(fun), inserter(inserter), space(space), path(path) {}
    Task(Ticket const &ticket, ReturnsCoroutineVoid auto const &funv, 
         std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> const &inserter, PathSpaceTE *space, Path const &path) :
            ticket(ticket), funv(funv), inserter(inserter), space(space), path(path) {}

    Ticket ticket;
    std::function<Coroutine()> fun;
    std::function<CoroutineVoid()> funv;
    std::function<void(Data const &data, Ticket const &ticket, PathSpaceTE &space)> inserter = [](Data const &data, Ticket const &ticket, PathSpaceTE &space){};
    PathSpaceTE *space = nullptr;
    Path path;
};
}