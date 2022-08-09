#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/file_helper.h"

#define LOG(...) {while(!spdlog::get("file")) {};if(auto file = spdlog::get("file")) {file->info(__VA_ARGS__);file->flush();}}

struct LogRAII {
    LogRAII(std::string const &message) : message(message) {LOG(this->message+" start")}
    ~LogRAII() {LOG(this->message+" end")}
private:
    std::string message;
};

template<typename Mutex>
class html_file_sink final : public spdlog::sinks::base_sink<Mutex>
{
public:
    explicit html_file_sink(const spdlog::filename_t &filename, bool truncate = false, const spdlog::file_event_handlers &event_handlers = {}) : file_helper_{event_handlers} {
        file_helper_.open(filename, truncate);
    }
    const spdlog::filename_t &filename() const {
        return file_helper_.filename();
    }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        file_helper_.write(formatted);
    }

    void flush_() override {
        file_helper_.flush();
    }

private:
    spdlog::details::file_helper file_helper_;
};

using html_file_sink_mt = html_file_sink<std::mutex>;
using html_file_sink_st = html_file_sink<spdlog::details::null_mutex>;

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> html_logger_mt(
    const std::string &logger_name, const spdlog::filename_t &filename, bool truncate = false, const spdlog::file_event_handlers &event_handlers = {})
{
    return Factory::template create<html_file_sink_mt>(logger_name, filename, truncate, event_handlers);
}

#define LOG_MUTEX
//#define LOG_PATH_SPACE
//#define LOG_CODEX
//#define LOG_FORGE

