#pragma once
#include "FSNG/PathSpaceTE.hpp"
#include "FSNG/Forge/Ticket.hpp"

namespace FSNG {
struct CodexInfo {
    CodexInfo() = default;
    CodexInfo(int const nbrItems, std::type_info const *info) : items(nbrItems), info(info) {};
    CodexInfo(Ticket const ticket, std::type_info const *info) : items(ticket, 0), info(info) {};
    auto operator==(CodexInfo const &rhs) const -> bool { return (this->items.nbr&&rhs.items.nbr) && (this->info==rhs.info); }

    auto nbrItems() const -> uint32_t {
        if(*this->info==typeid(std::string) ||
           *this->info==typeid(char const*) ||
           *this->info==typeid(PathSpaceTE) ||
           Converters::toJSONConverters.contains(this->info))
           return 1;
        return this->items.nbr;
    }

    auto nbrChars() const -> uint32_t {
        return (*this->info==typeid(std::string) ||
                *this->info==typeid(char const*)) ? this->items.nbr : 0;
    }

    auto dataSizeBytes() const -> int {
        return (*this->info==typeid(std::string) || 
                *this->info==typeid(char const*)) ? this->dataSizeBytesSingleItem() : this->dataSizeBytesSingleItem()*this->items.nbr;
    }

    auto dataSizeBytesSingleItem() const -> int {
        if     (*this->info==typeid(bool))                         return sizeof(bool);
        else if(*this->info==typeid(signed char))                  return sizeof(signed char);
        else if(*this->info==typeid(unsigned char))                return sizeof(unsigned char);
        else if(*this->info==typeid(wchar_t))                      return sizeof(wchar_t);
        else if(*this->info==typeid(short))                        return sizeof(short);
        else if(*this->info==typeid(unsigned short))               return sizeof(unsigned short);
        else if(*this->info==typeid(int))                          return sizeof(int);
        else if(*this->info==typeid(unsigned int))                 return sizeof(unsigned int);
        else if(*this->info==typeid(long))                         return sizeof(long);
        else if(*this->info==typeid(unsigned long))                return sizeof(unsigned long);
        else if(*this->info==typeid(long long))                    return sizeof(long long);
        else if(*this->info==typeid(unsigned long long))           return sizeof(unsigned long long);
        else if(*this->info==typeid(double))                       return sizeof(double);
        else if(*this->info==typeid(long double))                  return sizeof(long double);
        else if(*this->info==typeid(char const*))                  return sizeof(char)*this->items.nbr;
        else if(*this->info==typeid(std::string))                  return sizeof(char)*this->items.nbr;
        else if(*this->info==typeid(PathSpaceTE))                  return 0;
        else if(*this->info==typeid(Coroutine))                    return 0;
        else if(Converters::toJSONConverters.contains(this->info)) return this->items.size;
        return -1;
    }

    union Items {
        Items() = default;
        explicit Items(uint32_t i) : nbr(i) {}
        explicit Items(Ticket t, int) : ticket(t) {}
        uint32_t nbr;
        uint32_t size;
        Ticket ticket;
    } items;
    std::type_info const *info = nullptr;
};
}