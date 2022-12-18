#include <catch.hpp>

#include "PathSpace.hpp"

#include <mutex>

using namespace FSNG;

TEST_CASE("Forge") {
    PathSpaceTE space = PathSpace{};

    SECTION("Coroutine") {
        for(int i = 0; i < 1; ++i) {
            Forge forge;
            auto const ticket = forge.add([]()->Coroutine{co_return 345;}, space, "/test");
            forge.wait(ticket);
            nlohmann::json json;
            json["test"] = {345};
            REQUIRE(space.toJSON() == json);
        }
    }

    SECTION("Multiple Tasks") {
        Forge forge;
        std::set<int> s;
        std::vector<int> tickets;
        std::shared_mutex mutex;
        for(auto i = 0; i < 128; ++i) {
            tickets.push_back(forge.add([i]()->Coroutine{co_return i;}, space, "/test"));
        }
        for(auto const &ticket : tickets)
            forge.wait(ticket);
        std::set<int> res;
        for(auto i = 0; i < 128; ++i)
            res.insert(space.grabBlock<int>("/test"));
        REQUIRE(res.size()==128);
        REQUIRE(*res.begin()==0);
        REQUIRE(*res.rbegin()==127);
    }
}