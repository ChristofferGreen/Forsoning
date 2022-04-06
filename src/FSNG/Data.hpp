#pragma once
#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "InReference.hpp"

namespace FSNG {
struct PathSpaceTE;
struct Coroutine;
struct Data {
    Data() = default;
    Data(int i) : data(i) {}
    Data(char const *s) : data(std::string(s)) {}
    Data(std::string const &s) : data(s) {}
    Data(std::unique_ptr<PathSpaceTE> &&up) : data(std::move(up)) {}
    Data(PathSpaceTE const &pste) : data(std::make_unique<PathSpaceTE>(pste)) {}
    Data(auto const &in) {
        using InT = decltype(in);
        using InTRR = typename std::remove_reference<InT>;
        if constexpr(std::is_invocable<InT>())
            data = std::make_unique<std::function<Coroutine()>>(in);
        else if constexpr(std::is_standard_layout<InTRR>()) {// POD
            this->data = InReference{&in, sizeof(InT), &typeid(in)};
            if(InReference::converters.count(&typeid(in))==0) {
                InReference::converters[&typeid(in)] = [](std::byte const *data){
                    nlohmann::json out;
                    to_json(out, reinterpret_cast<InT>(*data));
                    return out;
                };
            }
        }
        else
            assert(false && "Error! Type can not be converted to Data!");
    }

    template<typename T>
    auto is() const {
        return std::holds_alternative<T>(this->data);
    }

    auto isPOD() const {
        return std::holds_alternative<InReference>(this->data);
    }

    template<typename T>
    auto& as() const {
        return std::get<T>(this->data);
    }

private:
    std::variant<int,
                 double,
                 std::string,
                 std::unique_ptr<PathSpaceTE>,
                 std::unique_ptr<std::function<Coroutine()>>,
                 InReference> data;
};
}