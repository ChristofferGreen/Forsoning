#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "backward.hpp"

#include <chrono>
#include <thread>

namespace backward {
backward::SignalHandling sh;
}

int main(int argc, char** argv) {
    try {
        auto logger = spdlog::basic_logger_mt("file", "logs/basic-log.txt", true);
        logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
    spdlog::get("file")->info("main");
    using namespace std::chrono_literals;
    
    doctest::Context context(argc, argv);
    int res = context.run();

    if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return res;          // propagate the result of the tests

    // client code goes here

    return res;
}