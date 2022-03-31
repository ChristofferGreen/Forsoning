#pragma once
#include "CodexInfo.hpp"

/*  Codex Docs - Future improvement for space efficeiency

Vector: Consists of bytes, the codex has a bool field named singleValueType, if true then the entire
vector consists only of that type and the number of elements can be gotten by vector.size()/sizeof(type).
If singleValueType is false then vector looks like so:
|FinalInfoLocation|Info|Data1Type1|Data2Type1|Info|Data3Type2|
FinalInfoLocation: An integer showing where in the vector the last/most recent info is located.
Info: Information about the following items, their type, element size and number of elements
DataXTypeY: Simply data of a certain type, for example int or string

Normally there is an info block whenever the data type changes but strings are an exception and have an
info block after every string.
*/

namespace FSNG {
struct Codex {
    auto insert(PathSpaceTE const &space) {
        this->addInfo(CodexInfo::Type::Space);
        this->spaces.push_back(space);
    }

    auto insert(Data const &data) {
        if(data.is<int>()) {
            auto const &info = this->addInfo(CodexInfo::Type::Int);
            auto const d = data.as<int>();
            std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(&d)),
                      static_cast<std::byte const * const>(static_cast<void const * const>(&d)) + sizeof(int),
                      std::back_inserter(this->codices));
        } else if(data.is<std::string>()) {
            auto const d = data.as<std::string>();
            auto const &info = this->addInfo(CodexInfo::Type::String, d.length());
            auto const dataSizeBytes = info.dataSizeBytes();
            std::copy(static_cast<std::byte const * const>(static_cast<void const * const>(d.c_str())),
                      static_cast<std::byte const * const>(static_cast<void const * const>(d.c_str())) + dataSizeBytes,
                      std::back_inserter(this->codices));
        }
    }

    template<typename T>
    auto visitFirst(auto const &fun) {
        if constexpr(std::is_same<T, PathSpaceTE>::value) {
            if(this->spaces.size()>0) {
                return fun(this->spaces[0]);
            }
        } else {
            
        }
        return false;
    }

    auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        int currentByte = 0;
        int currentSpace = 0;
        char const *ptr = nullptr;
        for(auto const &info : this->info) {
            for(auto i = 0; i < info.nbrItems(); ++i) {
                switch(info.type) {
                    case CodexInfo::Type::Int:
                        json.push_back(*reinterpret_cast<int const * const>(&this->codices[currentByte]));
                        break;
                    case CodexInfo::Type::String:
                        ptr = reinterpret_cast<char const * const>(&this->codices[currentByte]);
                        json.push_back(std::string(ptr, info.nbrChars()));
                        break;
                    case CodexInfo::Type::Space:
                        json.push_back(this->spaces[currentSpace++].toJSON());
                        break;
                };
                currentByte += info.dataSizeBytesSingleItem();
            }
        }
        return json;
    }

private:
    auto addInfo(CodexInfo::Type const &type, int const nbrItems=1) -> CodexInfo {
        if(this->info.size()==0)
            this->info.emplace_back(type, nbrItems);
        else if(this->info.rbegin()->type==type && type!=CodexInfo::Type::String)
            this->info.rbegin()->nbrItems_++;
        else
            this->info.emplace_back(type, nbrItems);
        return *this->info.rbegin();
    }

    bool singleValueType = true;
    std::vector<std::byte> codices;
    std::vector<CodexInfo> info;
    std::vector<PathSpaceTE> spaces;
};
}