#pragma once
#include "Ticket.hpp"

#include <thread>

namespace FSNG {
struct Thread {
    bool alive;
    Ticket ticket;
    std::thread thread;
};
}