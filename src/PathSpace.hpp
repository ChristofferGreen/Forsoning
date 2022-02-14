#pragma once
#include "FSNG/Coroutine.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Security.hpp"
#include "FSNG/ArraysAegis.hpp"
#include "FSNG/ArrayAegis.hpp"

#include "nlohmann/json.hpp"

#include <deque>
#include <unordered_map>
#include <string>
#include <variant>

namespace FSNG {
struct PathSpace {
    PathSpace() = default;

    virtual auto insert(Path const &path, Data const &data) -> bool {
        if(auto range = path.range())
            return this->insert(range.value(), data);
        return false;
    };

    virtual auto insert(Path const &path, std::function<Coroutine()> const &fun) -> bool {
        return false;
    }

    virtual auto insert(Path::Range const &range, Data const &data) -> bool {
        if(range.isAtData())
            return this->insert(range.dataName(), data);
        if(auto const spaceName = range.spaceName()) {
            auto const writeMutex = this->arrays.writeMutex();
            if(this->arrays.data.count(spaceName.value())>0) {
                auto &deque = this->arrays.data.at(spaceName.value());
                if(std::holds_alternative<std::deque<PathSpaceTE>>(deque)) {
                    auto &dequePS = std::get<std::deque<PathSpaceTE>>(deque);
                    if(dequePS.size()>0) {
                        return dequePS.at(0).insert(range.next(), data);
                    }
                }
            } else {
                this->arrays.data[spaceName.value()] = std::deque<PathSpaceTE>{PathSpaceTE(PathSpace{})};
                return std::get<std::deque<PathSpaceTE>>(this->arrays.data.at(spaceName.value())).at(0).insert(range.next(), data);
            }
        }
        return false;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        auto const readMutex = this->arrays.readMutex();
        for(auto const &p : this->arrays.data) {
            nlohmann::json array;
            if(std::holds_alternative<std::deque<int>>(this->arrays.data.at(p.first)))
                for(auto &v : std::get<std::deque<int>>(p.second))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<double>>(this->arrays.data.at(p.first)))
                for(auto &v : std::get<std::deque<double>>(p.second))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<std::string>>(this->arrays.data.at(p.first)))
                for(auto &v : std::get<std::deque<std::string>>(p.second))
                    array.push_back(v);
            else if(std::holds_alternative<std::deque<PathSpaceTE>>(this->arrays.data.at(p.first)))
                for(auto &v : std::get<std::deque<PathSpaceTE>>(p.second))
                    array.push_back(v.toJSON());
            json[p.first] = std::move(array);
        }
        return json;
    }

private:
    virtual auto insert(std::string const &dataName, Data const &data) -> bool {
        if(data.is<int>())
            this->insertT<int>(dataName, data);
        else if(data.is<double>())
            this->insertT<double>(dataName, data);
        else if(data.is<std::string>())
            this->insertT<std::string>(dataName, data);
        else if(data.is<std::unique_ptr<PathSpaceTE>>())
            this->insertSpaceT(dataName, data);
        else
            return false;
        return true;
    }

    template<typename T>
    auto insertT(std::string const &dataName, Data const &data) -> void {
        auto const writeMutex = this->arrays.writeMutex();
        if(this->arrays.data.count(dataName))
            std::get<std::deque<T>>(this->arrays.data[dataName]).push_back(data.as<T>());
        else
            this->arrays.data.insert(std::make_pair(dataName, std::deque<T>{data.as<T>()}));
    }

    auto insertSpaceT(std::string const &dataName, Data const &data) -> void {
        auto const writeMutex = this->arrays.writeMutex();
        if(this->arrays.data.count(dataName))
            std::get<std::deque<PathSpaceTE>>(this->arrays.data[dataName]).push_back(*data.as<std::unique_ptr<PathSpaceTE>>());
        else
            this->arrays.data.insert(std::make_pair(dataName, std::deque<PathSpaceTE>{*data.as<std::unique_ptr<PathSpaceTE>>()}));
    }

    ArraysAegis arrays;
};
}