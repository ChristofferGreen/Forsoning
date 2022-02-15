#pragma once
#include "FSNG/ArraysAegis.hpp"
#include "FSNG/Coroutine.hpp"
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

    virtual auto insert(Path const &path, Data const &data) -> bool {
        if(auto range = path.range())
            return this->insert(range.value(), data);
        return false;
    };

    virtual auto insert(Path::Range const &range, Data const &data) -> bool {
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        if(auto const spaceName = range.spaceName()) {
            int count = 0;
            {
                auto const mapReadMutex = this->arrays.readMutex();
                count = this->arrays.map.count(spaceName.value());
            }
            if(count>0) {
                auto const mapReadMutex = this->arrays.readMutex();
                auto &array = this->arrays.map.at(spaceName.value());
                auto arrayWriteMutex = array.writeMutex();
                if(std::holds_alternative<std::deque<PathSpaceTE>>(array.array)) {
                    if(auto &arrayPS = std::get<std::deque<PathSpaceTE>>(array.array); arrayPS.size()>0) {
                        return arrayPS.at(0).insert(range.next(), data);
                    }
                }
            } else {
                auto const mapWriteMutex = this->arrays.writeMutex();
                this->arrays.map[spaceName.value()] = std::deque<PathSpaceTE>{PathSpaceTE(PathSpace{this->processor})};
                auto &arrayAegis = this->arrays.map.at(spaceName.value());
                auto arrayWriteMutex = arrayAegis.writeMutex();
                auto &array = std::get<std::deque<PathSpaceTE>>(arrayAegis.array).at(0);
                return array.insert(range.next(), data);
            }
        }
        return false;
    }

    auto setProcessor(std::shared_ptr<TaskProcessor> const &processor) {
        this->processor = processor;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        auto const readMutex = this->arrays.readMutex();
        for(auto const &p : this->arrays.map) {
            nlohmann::json array;
            auto arrayReadMutex = this->arrays.map.at(p.first).readMutex();
            if(std::holds_alternative<std::deque<int>>(this->arrays.map.at(p.first).array))
                for(auto &v : std::get<std::deque<int>>(p.second.array))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<double>>(this->arrays.map.at(p.first).array))
                for(auto &v : std::get<std::deque<double>>(p.second.array))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<std::string>>(this->arrays.map.at(p.first).array))
                for(auto &v : std::get<std::deque<std::string>>(p.second.array))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<PathSpaceTE>>(this->arrays.map.at(p.first).array))
                for(auto &v : std::get<std::deque<PathSpaceTE>>(p.second.array))
                    array.push_back(v.toJSON());
            json[p.first] = std::move(array);
        }
        return json;
    }

private:
    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        if(data.is<int>())
            this->insert<int>(dataName, data);
        else if(data.is<double>())
            this->insert<double>(dataName, data);
        else if(data.is<std::string>())
            this->insert<std::string>(dataName, data);
        else if(data.is<std::unique_ptr<PathSpaceTE>>())
            this->insert(dataName, data.as<std::unique_ptr<PathSpaceTE>>());
        else if(data.is<std::unique_ptr<std::function<Coroutine()>>>())
            return this->insert(dataName, *data.as<std::unique_ptr<std::function<Coroutine()>>>());
        else
            return false;
        return true;
    }

    template<typename T>
    auto insert(std::string const &dataName, Data const &data) -> void {
        auto const writeMutex = this->arrays.writeMutex();
        if(this->arrays.map.count(dataName)) {
            auto arraysWriteMutex = this->arrays.map[dataName].writeMutex();
            std::get<std::deque<T>>(this->arrays.map[dataName].array).push_back(data.as<T>());
        }
        else
            this->arrays.map.insert(std::make_pair(dataName, std::deque<T>{data.as<T>()}));
    }

    auto insert(std::string const &dataName, std::unique_ptr<PathSpaceTE> const &space) -> void {
        auto const writeMutex = this->arrays.writeMutex();
        if(this->arrays.map.count(dataName)) {
            auto arraysWriteMutex = this->arrays.map[dataName].writeMutex();
            std::get<std::deque<PathSpaceTE>>(this->arrays.map[dataName].array).push_back(*space);
        }
        else {
            auto array = std::deque<PathSpaceTE>{*space};
            array.at(0).setProcessor(this->processor);
            this->arrays.map.insert(std::make_pair(dataName, std::move(array)));
        }
    }

    auto insert(std::string const &dataName, std::function<Coroutine()> const &coroutine) -> bool {
        if(this->processor) {
            this->processor->add(this, coroutine, [this, dataName](Data const &data){this->insert(dataName, data);});
            return true;
        }
        return false;
    }

    ArraysAegis arrays;
    std::shared_ptr<TaskProcessor> processor;
};
}