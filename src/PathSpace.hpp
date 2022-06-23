#pragma once
#include "FSNG/ArraysAegis.hpp"
#include "FSNG/CodicesAegis.hpp"
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/Forge/Hearth.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace() = default;

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }
    
    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        if(range.isAtRoot())
            LOG("PathSpace::grab {}, isTriviallyCopyable: {}", range.string(), isTriviallyCopyable);
        bool isFound = false;
        if(range.isAtData())
            isFound = this->grab(range.dataName(), info, data, isTriviallyCopyable);
        else if(auto const spaceName = range.spaceName()) {
            this->codices.write(spaceName.value(), [&isFound, &spaceName, &range, &info, &data, &isTriviallyCopyable](auto &codices){
                isFound = codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &info, &data, &isTriviallyCopyable](auto &space){return space.grab(range.next(), info, data, isTriviallyCopyable);});;
            });
        }
        if(range.isAtRoot())
            LOG("PathSpace::grab {}, found: {}", range.string(), isFound);
        return isFound;
    }

    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> void {
        bool const isAtRoot = range.isAtRoot();
        if(isAtRoot)
            LOG("PathSpace::grabBlock {}, isTriviallyCopyable: {}", range.string(), isTriviallyCopyable);
        if(range.isAtData())
            this->grabBlock(range.dataName(), info, data, isTriviallyCopyable);
        else if(auto const spaceName = range.spaceName()) {
            this->codices.write(spaceName.value(), [&spaceName, &range, &info, &data, &isTriviallyCopyable](auto &codices){
                if(codices.contains(spaceName.value()))
                    codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &info, &data, &isTriviallyCopyable](auto &space){space.grabBlock(range.next(), info, data, isTriviallyCopyable);return true;});;
            });
        }
        LOG("PathSpace::grabBlock {}, finished", range.string());
    }

    virtual auto insert(Path const &range, Data const &data) -> bool {
        bool const isAtRoot = range.isAtRoot();
        if(isAtRoot)
            LOG("PathSpace::insert {}", range.string());
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        if(auto const spaceName = range.spaceName()) { // Create space if it does not exist
            bool ret = false;
            this->codices.write(spaceName.value(), [this, &spaceName, &ret, &range, &data](auto &codices){
                if(codices.count(spaceName.value())==0)
                    codices[spaceName.value()].insert(PathSpaceTE(PathSpace{}), [](Data const &data){});
                ret = codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &data](auto &space){return space.insert(range.next(), data);});
            });
            return ret;
        }
        return false;
    }

    virtual auto toJSON() const -> nlohmann::json {
        LOG("PathSpace::toJSON codices size: {}", this->codices.size());
        nlohmann::json json;
        this->codices.read([&json](auto const &codices){
            for(auto const &p : codices)
                json[p.first] = p.second.toJSON();
        });
        LOG("PathSpace::toJSON finished");
        return json;
    }

private:
    virtual auto grab(std::string const &dataName, std::type_info const *info, void *data, bool isFundamentalType) -> bool {
        bool ret = false;
        this->codices.write(dataName, [&dataName, data, info, &ret, isFundamentalType](auto &codices){
            if(codices.contains(dataName))
                ret = codices.at(dataName).grab(info, data, isFundamentalType);
        });
        return ret;
    }

    virtual auto grabBlock(std::string const &dataName, std::type_info const *info, void *data, bool isFundamentalType) -> void {
        if(*info==typeid(Coroutine)) {
           // if(this->processor)
        } else {
            this->codices.writeUntilSucess(dataName, [&dataName, data, info, isFundamentalType](auto &codices){
                if(codices.contains(dataName)) {
                    codices.at(dataName).grab(info, data, isFundamentalType);
                    return true;
                }
                return false;
            });
        }
    }

    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        this->codices.write(dataName, [this, &dataName, &data](auto &codices){
            codices[dataName].insert(data, [this, dataName](Data const &coroData){
                this->codices.write(dataName, [&dataName, &coroData](auto &codices){
                    codices[dataName].insert(coroData, [](Data const &data){});
                });
            });
        });
        return true;
    }

    private:
        CodicesAegis codices;
};
}