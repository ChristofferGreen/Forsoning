#pragma once
#include "FSNG/ArraysAegis.hpp"
#include "FSNG/CodicesAegis.hpp"
#include "FSNG/Coroutine.hpp"
#include "FSNG/Codex.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/TaskProcessor.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace() : processor(std::make_shared<TaskProcessor>()) {};
    PathSpace(std::shared_ptr<TaskProcessor> const &processor) : processor(processor) {};

    virtual auto insert(Path const &range, Data const &data) -> bool {
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        if(auto const spaceName = range.spaceName()) { // Create space if it does not exist
            auto const mapWriteMutex = this->codices.writeMutex();
            if(this->codices.count(spaceName.value())==0)
                this->codices.push_back(spaceName.value(), PathSpaceTE(PathSpace{this->processor}));
            return this->codices.visitFirst<PathSpaceTE>(spaceName.value(), [&range, &data](auto &space){return space.insert(range.next(), data);});
        }
        return false;
    }

    auto setProcessor(std::shared_ptr<TaskProcessor> const &processor) {
        this->processor = processor;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        auto const mapReadMutex = this->codices.readMutex();
        for(auto const &p : this->codices.codices)
            json[p.first] = p.second.toJSON();
        return json;
    }

private:
    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        if(data.is<int>() || data.is<double>() || data.is<std::string>() || data.is<InReference>()) {
            auto const writeMutex = this->codices.writeMutex();
            this->codices.push_back(dataName, data);
        }
        else if(data.is<std::unique_ptr<PathSpaceTE>>())
            this->insert(dataName, data.as<std::unique_ptr<PathSpaceTE>>());
        else if(data.is<std::unique_ptr<std::function<Coroutine()>>>())
            return this->insert(dataName, *data.as<std::unique_ptr<std::function<Coroutine()>>>());
        else
            return false;
        return true;
    }

    auto insert(std::string const &dataName, std::unique_ptr<PathSpaceTE> const &space) -> void {
        auto const writeMutex = this->codices.writeMutex();
        this->codices.push_back(dataName, *space);
    }

    auto insert(std::string const &dataName, std::function<Coroutine()> const &coroutine) -> bool {
        if(this->processor) {
            this->processor->add(this, coroutine, [this, dataName](Data const &data){this->insert(dataName, data);});
            return true;
        }
        return false;
    }

    CodicesAegis codices;
    std::shared_ptr<TaskProcessor> processor;
};
}