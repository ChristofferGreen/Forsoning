#include <catch.hpp>

#include "FSNG/utils.hpp"
#include "FSNG/Forge/Forge.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    FSNG::Forge::CreateSingleton();
    SetupHTMLLog();
    //for(auto i = 0; i < argc; ++i)
        //LOG("{}", argv[i]);
    
    auto const ret = Catch::Session().run(argc, argv);
    FSNG::Forge::DestroySingleton();
    return ret;
}