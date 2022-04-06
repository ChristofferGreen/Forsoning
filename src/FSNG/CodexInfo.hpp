#pragma once

namespace FSNG {
struct CodexInfo {
    enum struct Type {
        Int = 0,
        String,
        POD,
        Space
    };

    CodexInfo() = default;
    CodexInfo(Type const &type, int const nbrItems, std::type_info const *info) : type(type), nbrItems_(nbrItems), info(info) {};

    auto nbrItems() const {
        return this->type == Type::String ? 1 : this->nbrItems_;
    }

    auto nbrChars() const {
        return this->nbrItems_;
    }

    auto dataSizeBytes() const -> int {
        return this->type == Type::String ? this->dataSizeBytesSingleItem() : this->dataSizeBytesSingleItem()*this->nbrItems_;
    }

    auto dataSizeBytesSingleItem() const -> int {
        switch(this->type) {
            case Type::Int: 
                return sizeof(int);
            case Type::String:
                return sizeof(char)*this->nbrItems_;
            case Type::Space:
                return -1; // Has no size.
            case Type::POD:
                return -1; // Has no size.
            };
    }

    Type type;
    int nbrItems_ = 0;
    std::type_info const *info = nullptr;
};
}