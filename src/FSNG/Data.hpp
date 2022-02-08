#pragma once
#include <optional>
#include <string>
#include <variant>
#include <memory>

namespace FSNG {
struct PathSpaceTE;
struct Data {
    Data(int i) : data(i) {}
    template<typename T>
    auto is() const {
        return std::holds_alternative<T>(this->data);
    }
    template<typename T>
    auto as() const {
        return std::get<T>(this->data);
    }
private:
    std::variant<int, double, std::string, std::unique_ptr<PathSpaceTE>> data;
};
}