#pragma once

namespace FSNG {
struct CodexInfo {
    enum struct Type {
        Int = 0,
        String,
        TriviallyCopyable,
        NotTriviallyCopyable,
        Space
    };

    CodexInfo() = default;
    CodexInfo(Type const &type, int const nbrItems, std::type_info const *info) : type(type), items(nbrItems), info(info) {};

    auto nbrItems() const {
        switch(this->type) {
            case Type::String:
            case Type::Space:
            case Type::NotTriviallyCopyable:
            case Type::TriviallyCopyable:
                return 1;
            case Type::Int: 
                return this->items.nbr;
        };
    }

    auto nbrChars() const {
        switch(this->type) {
            case Type::String:
                return this->items.nbr;
            case Type::Space:
            case Type::NotTriviallyCopyable:
            case Type::TriviallyCopyable:
            case Type::Int: 
                return -1;
        };
    }

    auto dataSizeBytes() const -> int {
        switch(this->type) {
            case Type::String:
                return this->dataSizeBytesSingleItem();
            case Type::Space:
            case Type::NotTriviallyCopyable:
            case Type::TriviallyCopyable:
            case Type::Int: 
                return this->dataSizeBytesSingleItem()*this->items.nbr;
        };
    }

    auto dataSizeBytesSingleItem() const -> int {
        switch(this->type) {
            case Type::Int: 
                return sizeof(int);
            case Type::String:
                return sizeof(char)*this->items.nbr;
            case Type::NotTriviallyCopyable:
            case Type::TriviallyCopyable:
                return this->items.size;
            case Type::Space:
                return -1; // Has no size.
        };
        return -1;
    }

    Type type;
    union Items {
        Items() = default;
        Items(int i) : nbr(i) {}
        int nbr;
        int size;
    } items;
    std::type_info const *info = nullptr;
};
}