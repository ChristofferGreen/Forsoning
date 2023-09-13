#pragma once
#include "FSNG/Path.hpp"
#include "FSNG/utils.hpp"
#include "FSNG/Codex2.hpp"

#include "nlohmann/json.hpp"

#include <any>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {

struct PathSpace2 {
    PathSpace2() = default;

    PathSpace2(PathSpace2 const &other) {
        *this = other;
    }
 
    PathSpace2& operator=(const PathSpace2& other) {
        this->codices = other.codices;
        this->root = other.root;
        return *this;
    }

    template<typename T>
    inline auto grab(Path const &range) -> std::optional<T> {
        T obj;
        if(this->grabImpl(range, &obj, &typeid(T), sizeof(T)))
            return obj;
        return std::nullopt;
    }

    virtual auto insert(Path const &range, InReference const &inref) -> bool {
        if(range.isAtSpace()) {
            if(!this->codices.contains(range.spaceName().value())) {
                if(!this->codices[range.spaceName().value()].insert(PathSpace2{}))
                    return false;
            }
            return this->codices[range.spaceName().value()].insert(range.next(), inref);
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

private:
    virtual auto grabImpl(Path const &range, void *obj, std::type_info const *info, std::size_t const size) -> bool {
        if(range.isAtSpace())
            if(!this->codices.contains(range.spaceName().value()))
                return false;
        return this->codices.at(range.spaceName().value()).grab(range, obj, info, size);
    }
};

#include "FSNG/Codex2Impl.hpp"
#include "FSNG/ScrollImpl.hpp"

}