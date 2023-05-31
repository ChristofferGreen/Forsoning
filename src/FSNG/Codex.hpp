#pragma once
#include "CodexInfo.hpp"
#include "FSNG/Forge/Forge.hpp"
#include "utils.hpp"

#include <algorithm>
#include <deque>
#include <string.h>

/* Potential future implementation
    Multi Type:  |Header 1byte|PointerToLastInfoBlock sizeof(CodexInfo*)|InfoBlock sizeof(CodexInfo)|Data
    Single type: |Data|Data|Data
*/

namespace FSNG {
struct Codex {
    Codex() = default;

    Codex(const Codex &other) {
        this->currentByte = other.currentByte;
        this->codices = other.codices;
        for(auto const &info : other.info)
            if(info.info!=&typeid(Coroutine) && info.info!=&typeid(CoroutineVoid))
                this->info.push_back(info);
        this->spaces = other.spaces;
    }

    auto operator==(Codex const &rhs) const -> bool { 
        return (std::equal(this->codices.begin()+this->currentByte, this->codices.end(), rhs.codices.begin()+rhs.currentByte)) &&
               (this->info==rhs.info) &&
               (this->spaces==rhs.spaces); 
    }

    auto grab(std::type_info const *info, void *data, bool const isTriviallyCopyable) -> bool {
        if(this->info.empty())
            return false;
        if(this->info.begin()->info!=info)
            return false;
        bool ret = false;
        if(isTriviallyCopyable) {
            copy_byte_raw(this->codices.data()+this->currentByte, this->info.begin()->dataSizeBytesSingleItem(), static_cast<std::byte*>(data));
            ret = true;
        } else if(*info==typeid(std::string)) {
            std::string &str = *static_cast<std::string*>(data);
            str = std::string(reinterpret_cast<char*>(this->codices.data()+this->currentByte), this->info.begin()->nbrChars());
            ret = true;
        } else if(*info==typeid(PathSpaceTE)) {
            if(this->spaces.size()==0)
                return false;
            *reinterpret_cast<PathSpaceTE*>(data) = this->spaces.front();
            this->spaces.erase(this->spaces.begin());
            ret = true;
        } else if(Converters::fromByteArray.contains(info))
            ret = Converters::fromByteArray.at(info)(this->codices.data()+this->currentByte, data);
        else if(Converters::fromJSON.contains(info))
            ret = Converters::fromJSON.at(info)(this->codices.data()+this->currentByte, this->info.begin()->dataSizeBytesSingleItem(), data);
        this->currentByte += this->info.begin()->dataSizeBytesSingleItem();
        this->popInfo();
        return ret;
    }

    auto read(std::type_info const *info, void *data, bool const isTriviallyCopyable) -> bool {
        if(this->info.empty())
            return false;
        if(this->info.begin()->info!=info)
            return false;
        bool ret = false;
        if(isTriviallyCopyable) {
            copy_byte_raw(this->codices.data()+this->currentByte, this->info.begin()->dataSizeBytesSingleItem(), static_cast<std::byte*>(data));
            ret = true;
        } else if(*info==typeid(std::string)) {
            std::string &str = *static_cast<std::string*>(data);
            str = std::string(reinterpret_cast<char*>(this->codices.data()+this->currentByte), this->info.begin()->nbrChars());
            ret = true;
        } else if(*info==typeid(PathSpaceTE)) {
            if(this->spaces.size()==0)
                return false;
            *reinterpret_cast<PathSpaceTE*>(data) = this->spaces.front();
            ret = true;
        } else if(Converters::fromByteArray.contains(info))
            ret = Converters::fromByteArray.at(info)(this->codices.data()+this->currentByte, data);
        else if(Converters::fromJSON.contains(info))
            ret = Converters::fromJSON.at(info)(this->codices.data()+this->currentByte, this->info.begin()->dataSizeBytesSingleItem(), data);
        return ret;
    }
 
    template<typename T>
    auto insertSpace(T &&space) -> void {
        this->spaces.emplace_back(std::move(space));
        this->addInfo(1, &typeid(PathSpaceTE));
    }

    auto insert(Path const &path, Path const &coroResultPath, Data const &data, PathSpaceTE &space) -> void {
        if(data.isTriviallyCopyable()) {
            this->addInfo(1, data.info);
            copy_byte_back_insert(data.ptr, data.size, this->codices);
        }
        else if(data.is<bool>())               this->insertBasic<bool>               (data);
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
        else if(data.is<long double>())        this->insertBasic<long double>        (data);
        else if(data.is<char const *>()) {
            auto const length = strlen(data.as<char const*>());
            auto const &info = this->addInfo(length, &typeid(std::string));
            copy_byte_back_insert(data.as<char const*>(), length, this->codices);
        }
        else if(data.is<std::string>()) {
            auto const d = data.as<std::string>();
            auto const &info = this->addInfo(d.length(), &typeid(std::string));
            auto const dataSizeBytes = info.dataSizeBytes();
            copy_byte_back_insert(d.c_str(), dataSizeBytes, this->codices);
        } else if(data.is<std::unique_ptr<std::function<Coroutine()>>>()) {
            auto const ticket = Forge::instance()->add(*data.as<std::unique_ptr<std::function<Coroutine()>>>(), space, path, coroResultPath);
            this->addInfo(1, &typeid(Coroutine), ticket);
        } else if(data.is<std::unique_ptr<std::function<CoroutineVoid()>>>()) {
            auto const ticket = Forge::instance()->add(*data.as<std::unique_ptr<std::function<CoroutineVoid()>>>(), space, path, coroResultPath);
            this->addInfo(1, &typeid(CoroutineVoid), ticket);
        } else if(data.is<std::unique_ptr<PathSpaceTE>>()) {
            this->addInfo(1, &typeid(PathSpaceTE));
            auto const &p = data.as<std::unique_ptr<PathSpaceTE>>();
            this->spaces.push_back(*p);
        } else if(data.is<InReference>()) {
            auto const dataRef = data.as<InReference>();
            auto const itemSize = dataRef.size;
            this->addInfo(itemSize, dataRef.info);
            if(Converters::toByteArray.contains(dataRef.info)) {
                int const preSize = this->codices.size();
                Converters::toByteArray[dataRef.info](this->codices, dataRef.data);
                int const postSize = this->codices.size();
                this->lastInfo().items.size = postSize-preSize;
            } else {
                copy_byte_back_insert(dataRef.data, dataRef.size, this->codices);
            }
        }
    }

    auto removeCoroutine(Ticket const &ticket) -> void {
        auto const iter = std::find_if(this->info.begin(), this->info.end(), [&ticket](CodexInfo const &info){
            if(info.info == &typeid(Coroutine) || info.info == &typeid(CoroutineVoid))
                return ticket==info.items.ticket;
            return false;
        });
        if(iter == this->info.end())
            return;
        this->info.erase(iter);
    }

    template<typename T>
    auto visitFirst(auto const &fun) -> bool {
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
                else if(*info.info==typeid(char const*))                  
                    json.push_back(std::string(reinterpret_cast<char const * const>(&this->codices[currentByte]), info.nbrChars()));
                else if(*info.info==typeid(std::string))                  json.push_back(std::string(reinterpret_cast<char const * const>(&this->codices[currentByte]), info.nbrChars()));
                else if(*info.info==typeid(PathSpaceTE))                  json.push_back(this->spaces[currentSpace++].toJSON());
                else if(Converters::toJSON.contains(info.info)) json.push_back(Converters::toJSON[info.info](reinterpret_cast<std::byte const *>(&this->codices[currentByte]), info.dataSizeBytesSingleItem()));
                currentByte += info.dataSizeBytesSingleItem();
            }
        }
        return json;
    }

    auto setRoot(PathSpaceTE *root) -> void {
        for(auto &space : this->spaces)
            space.insert("", root);
    }

    auto empty() const -> bool {
        return this->info.size()==0;
    }
    
private:
    auto popInfo() -> void {
        if(this->info.begin()->nbrItems()>1) {
            this->info.begin()->items.nbr--;
            return;
        }
        assert(this->info.begin()->nbrItems()!=0);
        this->info.erase(this->info.begin());
    }

    template<typename T>
    auto jsonPushBack(auto &json, auto const &currentByte) const -> void {
        json.push_back(*reinterpret_cast<T>(&this->codices[currentByte]));
    }

    template<typename T>
    auto insertBasic(auto const &data) -> void {
        this->addInfo(1, data.info);
        auto const d = data.template as<T>();
        copy_byte_back_insert(&d, data.size, this->codices);
    }

    auto addInfo(int const nbrItems, std::type_info const *ptr) -> CodexInfo {
        if(this->info.size()==0) {
            this->info.emplace_back(nbrItems, ptr);
        }
        else if(*this->info.rbegin()->info==*ptr && (*ptr!=typeid(std::string) && *ptr!=typeid(char const*)))
            this->info.rbegin()->items.nbr++;
        else {
            this->info.emplace_back(nbrItems, ptr);
        }

        return *this->info.rbegin();
    }

    auto addInfo(int const nbrItems, std::type_info const *ptr, Ticket const &ticket) -> CodexInfo {
        this->info.emplace_back(ticket, ptr);
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

struct PathSpace2;
struct Scroll {
    auto insert(InReference const &inref) {
        if(inref.isTriviallyCopyable) {
            std::copy(static_cast<std::byte const*>(inref.data), static_cast<std::byte const*>(inref.data)+inref.size, std::back_inserter(this->data));
            if(*inref.info==typeid(char*))
                this->itemSizes.push_back(inref.size);
            if(!inref.isFundamental && this->itemSizes.empty())
                this->itemSizes.push_back(inref.size);
            return true;
        } else if(Converters::toCompressedByteArray.contains(inref.info)) {
            auto const pre = this->data.size();
            Converters::toCompressedByteArray.at(inref.info)(this->data, inref.data);
            this->itemSizes.push_back(this->data.size()-pre);
            return true;
        }else if(Converters::toByteArray.contains(inref.info)) {
            auto const pre = this->data.size();
            Converters::toByteArray.at(inref.info)(this->data, inref.data);
            //if(this->itemSizes.empty()) // for uncompressed we only need one item size
            this->itemSizes.push_back(this->data.size()-pre);
            return true;
        }
        return false;
    }

    auto toJSON(std::type_info const *type) const -> nlohmann::json {
        // Arithmetic Types
        if(*type==typeid(char))                      return this->scalarToJSON<char>                ();
        else if(*type==typeid(signed char))          return this->scalarToJSON<signed char>         ();
        else if(*type==typeid(unsigned char))        return this->scalarToJSON<unsigned char>       ();
        else if(*type==typeid(short))                return this->scalarToJSON<short>               ();
        else if(*type==typeid(short int))            return this->scalarToJSON<short int>           ();
        else if(*type==typeid(signed short))         return this->scalarToJSON<signed short>        ();
        else if(*type==typeid(unsigned short))       return this->scalarToJSON<unsigned short>      ();
        else if(*type==typeid(signed short int))     return this->scalarToJSON<signed short int>    ();
        else if(*type==typeid(int))                  return this->scalarToJSON<int>                 ();
        else if(*type==typeid(signed))               return this->scalarToJSON<signed>              ();
        else if(*type==typeid(signed int))           return this->scalarToJSON<signed int>          ();
        else if(*type==typeid(unsigned))             return this->scalarToJSON<unsigned>            ();
        else if(*type==typeid(unsigned int))         return this->scalarToJSON<unsigned int>        ();
        else if(*type==typeid(long))                 return this->scalarToJSON<long>                ();
        else if(*type==typeid(long int))             return this->scalarToJSON<long int>            ();
        else if(*type==typeid(signed long))          return this->scalarToJSON<signed long>         ();
        else if(*type==typeid(signed long int))      return this->scalarToJSON<signed long int>     ();
        else if(*type==typeid(unsigned long))        return this->scalarToJSON<unsigned long>       ();
        else if(*type==typeid(unsigned long int))    return this->scalarToJSON<unsigned long int>   ();
        else if(*type==typeid(long long))            return this->scalarToJSON<long long>           ();
        else if(*type==typeid(long long int))        return this->scalarToJSON<long long int>       ();
        else if(*type==typeid(signed long long int)) return this->scalarToJSON<signed long long int>();
        else if(*type==typeid(unsigned long long))   return this->scalarToJSON<unsigned long long>  ();
        else if(*type==typeid(double))               return this->scalarToJSON<double>              ();
        else if(*type==typeid(long double))          return this->scalarToJSON<long double>         ();
        else if(*type==typeid(bool))                 return this->scalarToJSON<bool>                ();
        else if(*type==typeid(wchar_t))              return this->scalarToJSON<wchar_t>             ();
        // Containers
        else if(*type==typeid(char*))                return this->charArrayToJSON                   (); // Also used for std::string
        // To JSON
        else if(Converters::triviallyCopyableToJSON.contains(type)) return this->triviallyCopyableToJSON(type);
        else if(Converters::toJSON.contains(type))                  return this->nonTriviallToJSON(type);
        return {};
    }

    std::vector<std::byte> data; // Could be Ticket if type is Coroutine
    std::vector<unsigned int> itemSizes;
    std::vector<std::unique_ptr<PathSpace2>> spaces;

private:
    auto triviallyCopyableToJSON(std::type_info const *type) const -> nlohmann::json {
        nlohmann::json json;
        std::byte const *ptr = reinterpret_cast<std::byte const*>(this->data.data());
        unsigned int itemSize = this->itemSizes[0];
        unsigned int items = this->data.size()/itemSize;
        for(unsigned int i = 0; i < items; ++i)
            json += Converters::triviallyCopyableToJSON.at(type)(ptr+i*itemSize);
        return json;
    }
    
    auto nonTriviallToJSON(std::type_info const *type) const -> nlohmann::json {
        nlohmann::json json;
        std::byte const *ptr = this->data.data();
        unsigned int itemSize = this->itemSizes[0];
        unsigned int items = this->data.size()/itemSize;
        for(unsigned int i = 0; i < items; ++i)
            json += Converters::toJSON.at(type)(ptr+i*itemSize, itemSize);
        return json;
    }

    auto charArrayToJSON() const -> nlohmann::json {
        nlohmann::json json;
        char const *p = reinterpret_cast<char const*>(this->data.data());
        for(auto const i : this->itemSizes) {
            json += std::string(p, i);
            p += i;
        }
        return json;
    }

    template<typename T>
    auto scalarToJSON() const -> nlohmann::json {
        nlohmann::json json;
        T const *ptr = reinterpret_cast<T const*>(this->data.data());
        unsigned int amount = this->data.size()/sizeof(T);
        for(unsigned int i = 0; i < amount; ++i)
            json += ptr[i];
        return json;
    }
};

struct Codex2 {
    Codex2() = default;

    auto insert(InReference const &inref) {
        return this->scrolls[inref.info].insert(inref);
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        for(auto const &[type, scroll] : this->scrolls)
            json += scroll.toJSON(type);
        return json;
    }

    std::unordered_map<std::type_info const *, Scroll> scrolls;
};

}