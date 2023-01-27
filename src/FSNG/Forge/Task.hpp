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
    Task(Ticket const &ticket, auto const &fun, PathSpaceTE *space, Path const &path, Path const &coroResultPath="") :
            ticket(ticket), fun(fun), space(space), path(path), coroResultPath(coroResultPath) {}

    Ticket ticket;
    std::variant<std::function<Coroutine()>, std::function<CoroutineVoid()>> fun;
    PathSpaceTE *space = nullptr;
    Path path, coroResultPath;
};
}