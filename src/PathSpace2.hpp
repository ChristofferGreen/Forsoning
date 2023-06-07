#pragma once
#include "FSNG/Path.hpp"
#include "FSNG/utils.hpp"
#include "FSNG/Codex2.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {

struct PathSpace2 {
    PathSpace2() = default;

    PathSpace2(PathSpace2 const &other) {}
    /*PathSpace2(const PathSpace2& other) : member(other.member ? std::make_unique<PathSpace2>(*other.member) : nullptr) {}
 
    PathSpace2& operator=(const PathSpace2& other) {
        this->member = other.member ? std::make_unique<PathSpace2>(*other.member) : nullptr;
        return *this;
    }*/

    virtual auto insert(Path const &range, InReference const &inref) -> bool {
        if(!range.isAtData() && range.spaceName()) {
            if(!this->codices.contains(range.spaceName().value())) {
                this->codices[range.spaceName().value()].insert(PathSpace2{});
            }
        }
        return this->codices[range.dataName()].insert(inref);
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        std::shared_lock<std::shared_mutex> sharedLock(this->mutex);
        for(auto const &p : this->codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

    using IsPathSpace2Type = void;
protected:
    std::unordered_map<std::string, Codex2> codices;
    PathSpace2 *root = nullptr;
    mutable std::shared_mutex mutex;
};

}