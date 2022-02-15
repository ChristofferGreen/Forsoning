#pragma once
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
    template<typename T>
    Data(T const &coro) : data(std::make_unique<std::function<Coroutine()>>(coro)) {}

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