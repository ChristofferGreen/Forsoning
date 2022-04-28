#pragma once
#include <unordered_map>

#include "nlohmann/json.hpp"

namespace FSNG {
struct InReference {
    void const *data = nullptr;
    int size = 0;
    std::type_info const *info = nullptr;
};
}