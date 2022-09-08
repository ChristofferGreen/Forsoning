#include <catch.hpp>

#include "FSNG/utils.hpp"
#include "FSNG/Forge/Forge.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    auto const forge = FSNG::Forge::CreateSingleton();
    try {
        HTMLLoggerMT("file", "logs/basic-log.html", true)->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        LOG("This is the main thread");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
    //for(auto i = 0; i < argc; ++i)
        //LOG("{}", argv[i]);
    
    return Catch::Session().run(argc, argv);
}