#pragma once
#include "CodexInfo.hpp"
#include "utils.hpp"

namespace FSNG {
struct Codex {
    auto insert(PathSpaceTE const &space) {
        this->addInfo(CodexInfo::Type::Space);
        this->spaces.push_back(space);
    }

    auto insert(Data const &data) {
        if(data.is<int>()) {
            this->addInfo(CodexInfo::Type::Int);
            auto const d = data.as<int>();
            copy_byte_back_insert(&d, sizeof(int), this->codices);
        }
        else if(data.is<double>()) {
            this->addInfo(CodexInfo::Type::Double);
            auto const d = data.as<double>();
            copy_byte_back_insert(&d, sizeof(double), this->codices);
        } else if(data.is<std::string>()) {
            auto const d = data.as<std::string>();
            auto const &info = this->addInfo(CodexInfo::Type::String, d.length());
            auto const dataSizeBytes = info.dataSizeBytes();
            copy_byte_back_insert(d.c_str(), dataSizeBytes, this->codices);
        } else if(data.is<InReference>()) {
            auto const dataRef = data.as<InReference>();
            auto const itemSize = dataRef.size;
            if(dataRef.isTriviallyCopyable) {
                this->addInfo(CodexInfo::Type::TriviallyCopyable, itemSize, dataRef.info);
                copy_byte_back_insert(dataRef.data, dataRef.size, this->codices);
            } else {
                this->addInfo(CodexInfo::Type::NotTriviallyCopyable, itemSize, dataRef.info);
                int const preSize = this->codices.size();
                if(dataRef.toByteArrayConverters.contains(dataRef.info))
                    dataRef.toByteArrayConverters[dataRef.info](this->codices, dataRef.data);
                int const postSize = this->codices.size();
                this->info.rbegin()->items.size = postSize-preSize;
            }
        }
    }

    template<typename T>
    auto visitFirst(auto const &fun) {
        if constexpr(std::is_same<T, PathSpaceTE>::value)
            if(!this->spaces.empty())
                return fun(this->spaces.front());
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
                    case CodexInfo::Type::Double:
                        json.push_back(*reinterpret_cast<double const * const>(&this->codices[currentByte]));
                        break;
                    case CodexInfo::Type::String:
                        ptr = reinterpret_cast<char const * const>(&this->codices[currentByte]);
                        json.push_back(std::string(ptr, info.nbrChars()));
                        break;
                    case CodexInfo::Type::NotTriviallyCopyable:
                    case CodexInfo::Type::TriviallyCopyable:
                        if(InReference::toJSONConverters.contains(info.info))
                            json.push_back(InReference::toJSONConverters[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
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
    auto addInfo(CodexInfo::Type const &type, int const nbrItems=1, std::type_info const *ptr=nullptr) -> CodexInfo {
        if(this->info.size()==0)
            this->info.emplace_back(type, nbrItems, ptr);
        else if(this->info.rbegin()->type==type && type!=CodexInfo::Type::String)
            this->info.rbegin()->items.nbr++;
        else
            this->info.emplace_back(type, nbrItems, ptr);
        return *this->info.rbegin();
    }

    bool singleValueType = true;
    std::vector<std::byte> codices;
    std::vector<CodexInfo> info;
    std::vector<PathSpaceTE> spaces;
};
}