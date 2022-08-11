#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "FSNG/utils.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

int main(int argc, char** argv) {
    try {
        auto logger = html_logger_mt("file", "logs/basic-log.html", true);
        logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
        spdlog::get("file")->info("main");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
    
    doctest::Context context(argc, argv);
    return context.run();
}