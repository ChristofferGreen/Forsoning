#pragma once
#include "CodexInfo.hpp"
#include "Scroll.hpp"
#include "FSNG/Forge/Forge.hpp"
#include "utils.hpp"

#include <algorithm>
#include <deque>
#include <memory>
#include <string.h>

/* Potential future implementation
    Multi Type:  |Header 1byte|PointerToLastInfoBlock sizeof(CodexInfo*)|InfoBlock sizeof(CodexInfo)|Data
    Single type: |Data|Data|Data
*/

namespace FSNG {

struct PathSpace2;

struct Codex2 {
    Codex2() = default;

    auto insert(InReference const &inref) {
        return this->scrolls[inref.info].insert(inref);
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        for(auto const &[type, scroll] : this->scrolls)
            json += scroll.toJSON(type);
        return json;
    }

    std::unordered_map<std::type_info const *, Scroll> scrolls;
};

}