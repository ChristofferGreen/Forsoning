#pragma once
#include "CodexInfo.hpp"
#include "FSNG/Forge/Forge.hpp"
#include "utils.hpp"

#include <algorithm>
#include <deque>
#include <memory>
#include <string.h>

/* Potential future implementation
    Multi Type:  |Header 1byte|PointerToLastInfoBlock sizeof(CodexInfo*)|InfoBlock sizeof(CodexInfo)|Data
    Single type: |Data|Data|Data
*/

namespace FSNG {

struct PathSpace2;
struct Scroll {
    Scroll() = default;

    Scroll& operator=(const Scroll& other) {
        this->data = other.data;
        this->itemSizes = other.itemSizes;
        return *this;
    }

    Scroll(Scroll const &other) {
        this->data = other.data;
        this->itemSizes = other.itemSizes;
    }

    auto grab(Path const &range, void *obj, std::type_info const *info) -> bool;
    auto insert(Path const &range, InReference const &inref) -> bool;

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
        } else if(Converters::toByteArray.contains(inref.info)) {
            auto const pre = this->data.size();
            Converters::toByteArray.at(inref.info)(this->data, inref.data);
            //if(this->itemSizes.empty()) // for uncompressed we only need one item size
            this->itemSizes.push_back(this->data.size()-pre);
            return true;
        } else if(inref.isPathSpace) {
            this->spaces.push_back(std::make_unique<PathSpace2>(*static_cast<PathSpace2 const*>(inref.data)));
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
        else if(!this->spaces.empty())               return this->spacesToJSON                      ();
        // To JSON
        else if(Converters::triviallyCopyableToJSON.contains(type)) return this->triviallyCopyableToJSON(type);
        else if(Converters::toJSON.contains(type))                  return this->nonTriviallToJSON(type);
        return {};
    }

    std::vector<std::byte> data; // Could be Ticket if type is Coroutine
    std::vector<unsigned int> itemSizes;
    std::vector<std::unique_ptr<PathSpace2>> spaces;

private:
    auto spacesToJSON() const -> nlohmann::json;

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

}