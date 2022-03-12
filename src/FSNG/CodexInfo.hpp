#pragma once

namespace FSNG {
struct CodexInfo {
    enum struct Type {
        Int = 0,
        String,
        Space
    };

    CodexInfo() = default;
    CodexInfo(Type const &type, int const nbrItems) : type(type), nbrItems_(nbrItems) {};

    auto nbrItems() const {
        return this->type == Type::String ? 1 : this->nbrItems_;
    }

    auto nbrChars() const {
        return this->nbrItems_;
    }

    auto dataSizeBytes() const -> int {
        return this->dataSizeBytesSingleItem()*this->nbrItems_;
    }

    auto dataSizeBytesSingleItem() const -> int {
        switch(this->type) {
            case Type::Int: 
                return sizeof(int);
            case Type::String:
                return sizeof(char);
            case Type::Space:
                return -1; // Has no size.
        };
    }

    Type type;
    int nbrItems_ = 0;
};
}