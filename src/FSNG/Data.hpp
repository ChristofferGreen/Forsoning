#pragma once
#include <cassert>
#include <optional>
#include <string>
#include <variant>
#include <memory>

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
        using InT = typename std::remove_reference<decltype(in)>;
        if constexpr(std::is_invocable<InT>())
            data = std::make_unique<std::function<Coroutine()>>(in);
        else if constexpr(std::is_standard_layout<InT>()) // POD
            int a = 5;
        else
            assert(false && "Error! Type can not be converted to Data!");
    }

    template<typename T>
    auto is() const {
        return std::holds_alternative<T>(this->data);
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
                 std::unique_ptr<std::function<Coroutine()>>> data;
};
}