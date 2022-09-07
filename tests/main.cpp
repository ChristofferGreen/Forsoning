#include <catch.hpp>

#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    try {
        auto logger = html_logger_mt("file", "logs/basic-log.html", true);
        logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        LOG("This is the main thread");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
    //for(auto i = 0; i < argc; ++i)
        //LOG("{}", argv[i]);
    
    return Catch::Session().run(argc, argv);
}