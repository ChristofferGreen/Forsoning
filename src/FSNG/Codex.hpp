#pragma once
#include "CodexInfo.hpp"
#include "utils.hpp"

namespace FSNG {
struct Codex {
    auto insert(Data const &data) {
        if(data.is<short>())                   this->insertBasic<short>              (CodexInfo::Type::Short,            data);
        else if(data.is<unsigned short>())     this->insertBasic<unsigned short>     (CodexInfo::Type::UnsignedShort,    data);
        else if(data.is<int>())                this->insertBasic<int>                (CodexInfo::Type::Int,              data);
        else if(data.is<unsigned int>())       this->insertBasic<unsigned int>       (CodexInfo::Type::UnsignedInt,      data);
        else if(data.is<long>())               this->insertBasic<long>               (CodexInfo::Type::Long,             data);
        else if(data.is<unsigned long>())      this->insertBasic<unsigned long>      (CodexInfo::Type::UnsignedLong,     data);
        else if(data.is<long long>())          this->insertBasic<long long>          (CodexInfo::Type::LongLong,         data);
        else if(data.is<unsigned long long>()) this->insertBasic<unsigned long long> (CodexInfo::Type::UnsignedLongLong, data);
        else if(data.is<double>())             this->insertBasic<double>             (CodexInfo::Type::Double,           data);
        else if(data.is<std::string>()) {
            auto const d = data.as<std::string>();
            auto const &info = this->addInfo(CodexInfo::Type::String, d.length(), &typeid(std::string));
            auto const dataSizeBytes = info.dataSizeBytes();
            copy_byte_back_insert(d.c_str(), dataSizeBytes, this->codices);
        } else if(data.is<std::unique_ptr<PathSpaceTE>>()) {
            this->addInfo(CodexInfo::Type::Space, 1, &typeid(PathSpaceTE));
            auto const &p = data.as<std::unique_ptr<PathSpaceTE>>();
            this->spaces.push_back(*p);
        } else if(data.is<InReferenceTriviallyCopyable>()) {
            auto const dataRef = data.as<InReferenceTriviallyCopyable>();
            auto const itemSize = dataRef.size;
            this->addInfo(CodexInfo::Type::TriviallyCopyable, itemSize, dataRef.info);
            copy_byte_back_insert(dataRef.data, dataRef.size, this->codices);
        } else if(data.is<InReferenceNonTriviallyCopyable>()) {
            auto const dataRef = data.as<InReferenceNonTriviallyCopyable>();
            auto const itemSize = dataRef.size;
            this->addInfo(CodexInfo::Type::NotTriviallyCopyable, itemSize, dataRef.info);
            int const preSize = this->codices.size();
            if(Converters::toByteArrayConverters.contains(dataRef.info))
                Converters::toByteArrayConverters[dataRef.info](this->codices, dataRef.data);
            int const postSize = this->codices.size();
            this->info.rbegin()->items.size = postSize-preSize;
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
                if     (*info.info==typeid(short))              this->jsonPushBack<short              const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned short))     this->jsonPushBack<unsigned short     const * const>(json, currentByte);
                else if(*info.info==typeid(int))                this->jsonPushBack<int                const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned int))       this->jsonPushBack<unsigned int       const * const>(json, currentByte);
                else if(*info.info==typeid(long))               this->jsonPushBack<long               const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned long))      this->jsonPushBack<unsigned long      const * const>(json, currentByte);
                else if(*info.info==typeid(long long))          this->jsonPushBack<long long          const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned long long)) this->jsonPushBack<unsigned long long const * const>(json, currentByte);
                else if(*info.info==typeid(double))             this->jsonPushBack<double             const * const>(json, currentByte);
                else if(*info.info==typeid(std::string)) {
                    ptr = reinterpret_cast<char const * const>(&this->codices[currentByte]);
                    json.push_back(std::string(ptr, info.nbrChars()));
                }
                else if(*info.info==typeid(InReferenceTriviallyCopyable)) {
                    //if(InReference::toJSONConverters.contains(info.info))
                      //  json.push_back(InReference::toJSONConverters[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
                }
                switch(info.type) {
                    case CodexInfo::Type::NotTriviallyCopyable:
                        if(Converters::toJSONConverters.contains(info.info))
                            json.push_back(Converters::toJSONConverters[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
                        break;
                    case CodexInfo::Type::TriviallyCopyable:
                        if(Converters::toJSONConverters.contains(info.info))
                            json.push_back(Converters::toJSONConverters[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
                        break;
                    case CodexInfo::Type::Space:
                        json.push_back(this->spaces[currentSpace++].toJSON());
                        break;
                    default: break;
                };
                currentByte += info.dataSizeBytesSingleItem();
            }
        }
        return json;
    }

private:
    template<typename T>
    auto jsonPushBack(auto &json, auto const &currentByte) const -> void {
        json.push_back(*reinterpret_cast<T>(&this->codices[currentByte]));
    }

    template<typename T>
    auto insertBasic(auto const &type, auto const &data) -> void {
        this->addInfo(type, 1, &typeid(T));
        auto const d = data.template as<T>();
        copy_byte_back_insert(&d, sizeof(T), this->codices);
    }

    auto addInfo(CodexInfo::Type const &type, int const nbrItems, std::type_info const *ptr) -> CodexInfo {
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