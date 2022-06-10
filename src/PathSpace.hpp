#pragma once
#include "FSNG/ArraysAegis.hpp"
#include "FSNG/CodicesAegis.hpp"
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/Forge/TaskProcessor.hpp"
#include "FSNG/Forge/Eschelon.hpp"
#include "FSNG/utils.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace()                                                : processor(std::make_shared<TaskProcessor>()) {};
    PathSpace(std::shared_ptr<TaskProcessor> const &processor) : processor(processor) {};

    auto operator==(PathSpace const &rhs) const -> bool { return this->codices==rhs.codices; }
    
    auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        if(range.isAtData())
            return this->grab(range.dataName(), info, data, isTriviallyCopyable);
        if(auto const spaceName = range.spaceName()) {
            bool ret = false;
            this->codices.write(spaceName.value(), [&ret, &spaceName, &range, &info, &data, &isTriviallyCopyable](auto &codices){
                ret = codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &info, &data, &isTriviallyCopyable](auto &space){return space.grab(range.next(), info, data, isTriviallyCopyable);});;
            });
            return ret;
        }
        return false;
    }

    auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        if(range.isAtData())
            return this->grabBlock(range.dataName(), info, data, isTriviallyCopyable);
        if(auto const spaceName = range.spaceName()) {
            bool ret = false;
            this->codices.writeWaitForExistance(spaceName.value(), [&ret, &spaceName, &range, &info, &data, &isTriviallyCopyable](auto &codices){
                ret = codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &info, &data, &isTriviallyCopyable](auto &space){return space.grabBlock(range.next(), info, data, isTriviallyCopyable);});;
            });
            return ret;
        }
        return false;
    }

    virtual auto insert(Path const &range, Data const &data) -> bool {
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        if(auto const spaceName = range.spaceName()) { // Create space if it does not exist
            bool ret = false;
            this->codices.write(spaceName.value(), [this, &spaceName, &ret, &range, &data](auto &codices){
                if(codices.count(spaceName.value())==0)
                    codices[spaceName.value()].insert(PathSpaceTE(PathSpace{this->processor}));
                ret = codices[spaceName.value()].template visitFirst<PathSpaceTE>([&range, &data](auto &space){return space.insert(range.next(), data);});
            });
            return ret;
        }
        return false;
    }

    auto setProcessor(std::shared_ptr<TaskProcessor> const &processor) {
        this->processor = processor;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        this->codices.read([&json](auto const &codices){
            for(auto const &p : codices)
                json[p.first] = p.second.toJSON();
        });
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

    virtual auto grabBlock(std::string const &dataName, std::type_info const *info, void *data, bool isFundamentalType) -> bool {
        bool ret = false;
        if(*info==typeid(Coroutine)) {
           // if(this->processor)
        } else {
            this->codices.writeWaitForExistance(dataName, [&dataName, data, info, &ret, isFundamentalType](auto &codices){
                if(codices.contains(dataName))
                    ret = codices.at(dataName).grab(info, data, isFundamentalType);
            });
        }
        return ret;
    }

    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        if(data.isDirectlyInsertable()) {
            this->codices.write(dataName, [&dataName, &data](auto &codices){
                codices[dataName].insert(data);
            });
        }
        else if(data.is<std::unique_ptr<std::function<Coroutine()>>>())
            return this->insert(dataName, *data.as<std::unique_ptr<std::function<Coroutine()>>>());
        else
            return false;
        return true;
    }
    
    auto insert(std::string const &dataName, std::function<Coroutine()> const &coroutine) -> bool {
        if(this->processor) {
            this->processor->add(this, coroutine, [this, dataName](Data const &data){this->insert(dataName, data);});
            return true;
        }
        return false;
    }
    private:
        CodicesAegis codices;
        std::shared_ptr<TaskProcessor> processor;
};
}