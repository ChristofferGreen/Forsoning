#pragma once
#include <string>
#include "nlohmann/json.hpp"

namespace FSNG {
struct CreateWindow {
    bool operator==(CreateWindow const&) const = default;
    const char *title = "";
    bool fullscreen = false;
};
}

inline void to_json(nlohmann::json& j, const CreateWindow& c) {
    j = nlohmann::json{{"title", c.title}, {"fullscreen", c.fullscreen}};
}