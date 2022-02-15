#pragma once
#include "FSNG/Data.hpp"

#include <functional>

namespace FSNG {
struct Task {
    std::function<void(Data const &data)> inserter;
};
}