#pragma once
#include "CodexInfo.hpp"
#include "utils.hpp"

/* Potential future implementation
    Multi Type:  |Header 1byte|PointerToLastInfoBlock sizeof(CodexInfo*)|InfoBlock sizeof(CodexInfo)|Data
    Single type: |Data|Data|Data
*/

namespace FSNG {
struct Codex {
    auto grab(std::type_info const *info, void *data, bool const isFundamentalType) -> bool {
        if(this->info.empty())
            return false;
        if(this->info.rbegin()->info!=info)
            return false;
        if(isFundamentalType) {
            copy_byte_raw(this->codices.data(), this->info.rbegin()->dataSizeBytesSingleItem(), static_cast<std::byte*>(data));
            return true;
        }
        if(Converters::fromJSONConverters.contains(info))
            return Converters::fromJSONConverters.at(info)(this->codices.data(), static_cast<void*>(this->codices.data()), static_cast<std::byte*>(data));
        return false;
    }
 
    auto insert(Data const &data) {
        if(data.is<bool>())                    this->insertBasic<bool>               (data);
        else if(data.is<signed char>())        this->insertBasic<signed char>        (data);
        else if(data.is<unsigned char>())      this->insertBasic<unsigned char>      (data);
        else if(data.is<wchar_t>())            this->insertBasic<wchar_t>            (data);
        else if(data.is<short>())              this->insertBasic<short>              (data);
        else if(data.is<unsigned short>())     this->insertBasic<unsigned short>     (data);
        else if(data.is<int>())                this->insertBasic<int>                (data);
        else if(data.is<unsigned int>())       this->insertBasic<unsigned int>       (data);
        else if(data.is<long>())               this->insertBasic<long>               (data);
        else if(data.is<unsigned long>())      this->insertBasic<unsigned long>      (data);
        else if(data.is<long long>())          this->insertBasic<long long>          (data);
        else if(data.is<unsigned long long>()) this->insertBasic<unsigned long long> (data);
        else if(data.is<double>())             this->insertBasic<double>             (data);
        else if(data.is<long double>())        this->insertBasic<long double>             (data);
        else if(data.is<std::string>()) {
            auto const d = data.as<std::string>();
            auto const &info = this->addInfo(d.length(), &typeid(std::string));
            auto const dataSizeBytes = info.dataSizeBytes();
            copy_byte_back_insert(d.c_str(), dataSizeBytes, this->codices);
        } else if(data.is<std::unique_ptr<PathSpaceTE>>()) {
            this->addInfo(1, &typeid(PathSpaceTE));
            auto const &p = data.as<std::unique_ptr<PathSpaceTE>>();
            this->spaces.push_back(*p);
        } else if(data.is<InReference>()) {
            auto const dataRef = data.as<InReference>();
            auto const itemSize = dataRef.size;
            this->addInfo(itemSize, dataRef.info);
            if(Converters::toByteArrayConverters.contains(dataRef.info)) {
                int const preSize = this->codices.size();
                Converters::toByteArrayConverters[dataRef.info](this->codices, dataRef.data);
                int const postSize = this->codices.size();
                this->lastInfo().items.size = postSize-preSize;
            } else {
                copy_byte_back_insert(dataRef.data, dataRef.size, this->codices);
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
                if     (*info.info==typeid(bool))               this->jsonPushBack<bool               const * const>(json, currentByte);
                else if(*info.info==typeid(signed char))        this->jsonPushBack<signed char        const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned char))      this->jsonPushBack<unsigned char      const * const>(json, currentByte);
                else if(*info.info==typeid(wchar_t))            this->jsonPushBack<wchar_t            const * const>(json, currentByte);
                else if(*info.info==typeid(short))              this->jsonPushBack<short              const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned short))     this->jsonPushBack<unsigned short     const * const>(json, currentByte);
                else if(*info.info==typeid(int))                this->jsonPushBack<int                const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned int))       this->jsonPushBack<unsigned int       const * const>(json, currentByte);
                else if(*info.info==typeid(long))               this->jsonPushBack<long               const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned long))      this->jsonPushBack<unsigned long      const * const>(json, currentByte);
                else if(*info.info==typeid(long long))          this->jsonPushBack<long long          const * const>(json, currentByte);
                else if(*info.info==typeid(unsigned long long)) this->jsonPushBack<unsigned long long const * const>(json, currentByte);
                else if(*info.info==typeid(double))             this->jsonPushBack<double             const * const>(json, currentByte);
                else if(*info.info==typeid(long double))        this->jsonPushBack<long double        const * const>(json, currentByte);
                else if(*info.info==typeid(std::string))                  json.push_back(std::string(reinterpret_cast<char const * const>(&this->codices[currentByte]), info.nbrChars()));
                else if(*info.info==typeid(PathSpaceTE))                  json.push_back(this->spaces[currentSpace++].toJSON());
                else if(Converters::toJSONConverters.contains(info.info)) json.push_back(Converters::toJSONConverters[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
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
    auto insertBasic(auto const &data) -> void {
        this->addInfo(1, &typeid(T));
        auto const d = data.template as<T>();
        copy_byte_back_insert(&d, sizeof(T), this->codices);
    }

    auto addInfo(int const nbrItems, std::type_info const *ptr) -> CodexInfo {
        if(this->info.size()==0)
            this->info.emplace_back(nbrItems, ptr);
        else if(*this->info.rbegin()->info==*ptr && *ptr!=typeid(std::string))
            this->info.rbegin()->items.nbr++;
        else
            this->info.emplace_back(nbrItems, ptr);

        return *this->info.rbegin();
    }

    CodexInfo& lastInfo() {
        return *this->info.rbegin();
    }

    uint32_t currentByte = 0;
    std::vector<std::byte> codices;
    std::vector<CodexInfo> info;
    std::vector<PathSpaceTE> spaces;
};
}