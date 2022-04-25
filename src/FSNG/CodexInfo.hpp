#pragma once

namespace FSNG {
struct CodexInfo {
    enum struct Type {
        Short = 0,
        UnsignedShort,
        Int,
        UnsignedInt,
        Long,
        UnsignedLong,
        LongLong,
        UnsignedLongLong,
        Double,
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
            case Type::Short:
            case Type::UnsignedShort:
            case Type::Int:
            case Type::UnsignedInt:
            case Type::Long:
            case Type::UnsignedLong:
            case Type::LongLong:
            case Type::UnsignedLongLong:
            case Type::Double:
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
            case Type::Short:
            case Type::UnsignedShort:
            case Type::Int:
            case Type::UnsignedInt:
            case Type::Long:
            case Type::UnsignedLong:
            case Type::LongLong:
            case Type::UnsignedLongLong:
            case Type::Double:
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
            case Type::Short:
            case Type::UnsignedShort:
            case Type::Int:
            case Type::UnsignedInt:
            case Type::Long:
            case Type::UnsignedLong:
            case Type::LongLong:
            case Type::UnsignedLongLong:
            case Type::Double:
                return this->dataSizeBytesSingleItem()*this->items.nbr;
        };
    }

    auto dataSizeBytesSingleItem() const -> int {
        switch(this->type) {
            case Type::Short:
                return sizeof(short);
            case Type::UnsignedShort:
                return sizeof(unsigned short);
            case Type::Int:
                return sizeof(int);
            case Type::UnsignedInt:
                return sizeof(unsigned int);
            case Type::Long:
                return sizeof(long);
            case Type::UnsignedLong:
                return sizeof(unsigned long);
            case Type::LongLong:
                return sizeof(long long);
            case Type::UnsignedLongLong:
                return sizeof(unsigned long long);
            case Type::Double: 
                return sizeof(double);
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