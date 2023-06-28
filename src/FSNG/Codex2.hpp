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

    Codex2& operator=(const Codex2& other) {
        this->scrolls = other.scrolls;
        return *this;
    }

    Codex2(Codex2 const &other) {
        *this = other;
    }

    auto insert(Path const &range, InReference const &inref) -> bool;

    auto insert(InReference const &inref) {
        return this->scrolls[inref.info].insert(inref);
    }
    
    auto grab(Path const &range, void *obj, std::type_info const *info) -> bool;
    
    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        for(auto const &[type, scroll] : this->scrolls)
            json += scroll.toJSON(type);
        return json;
    }

    std::unordered_map<std::type_info const *, Scroll> scrolls;
};

}