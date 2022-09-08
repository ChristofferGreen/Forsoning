#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/file_helper.h"

#include <stack>

#define LOG(...) {if(auto file = spdlog::get("file")) {file->info(__VA_ARGS__);file->flush();}}

struct LogRAII {
    LogRAII(std::string const &message) : message(message) {LOG(this->message+" start")}
    ~LogRAII() {LOG(this->message+" end")}
private:
    std::string message;
};

#define LL(...) spdlog::details::log_msg("", spdlog::level::level_enum::info, __VA_ARGS__)

template<typename Mutex>
class html_file_sink final : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit html_file_sink(const spdlog::filename_t &filename, bool truncate = false, const spdlog::file_event_handlers &event_handlers = {}) : fileHelper{event_handlers} {
        this->fileHelper.open(filename, truncate);
        this->fileJavascriptHelper.open("logs/javascript.html", truncate);
        this->writeToFile("<!DOCTYPE html>\n");
        this->writeToFile("<html>\n");
        this->writeToFile("<head>\n");
        this->writeToFile("<style>\n");
        this->writeToFile("table, th, td {border: 1px solid black; border-collapse: collapse;}\n");
        this->writeToFile("</style>\n");
        this->writeToFile("</head>\n");
        this->writeToFile("<body>\n");
        this->writeToFile("<iframe id=\"frame1\" name=\"frame1\" src=\"javascript.html\" width=\"600\" height=\"50\" frameBorder=\"0\"></iframe>\n");
        this->writeToFile("<table style='font-size:45%' nowrap=\"nowrap\">\n");

        this->writeToJavascriptFile("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js\"></script>\n");
    }

    ~html_file_sink() {
        this->writeToFile("</table>\n");
        this->writeToFile("</html>\n");
        this->writeToFile("</body>\n");
    }

protected:
    auto sink_it_(const spdlog::details::log_msg &msg) -> void override {
        auto const s = std::string_view(msg.payload.data(), msg.payload.size());
        int offset = 0;
        std::string tag;
        if(s.starts_with("<TAG:")) {
            offset = s.find_first_of('>')+1;
            tag = std::string(msg.payload.data()+s.find_first_of(':')+1, offset-6);
            if(!this->tagsSeen.contains(tag)) {
                this->tagToId[tag] = this->tagToId.size();
                std::string const buttonOnClick = "$(\"tr."+tag+"\", parent.document).toggle();";
                std::string const buttonColor = this->colors.size() > this->tagToId.at(tag) ? "style=\"background-color:"+this->tagToColor(tag)+"\"" : "";
                this->writeToJavascriptFile("<button "+buttonColor+" onclick='"+buttonOnClick+"' type=\"button\">"+tag+"</button>\n");
            }
            this->tagsSeen.insert(tag);
        }
        std::string const hclass = tag=="" ? "" : " class=\""+tag+"\"";
        this->writeToFile("<tr"+hclass+">"+this->toRow(this->threadNumber(msg.thread_id), tag, msg.payload, offset)+"</tr>\n");
    }

    auto flush_() -> void  override {
        this->fileHelper.flush();
    }

private:
    auto tagToColor(std::string const &tag) const -> std::string {
        if(this->tagToId.contains(tag))
            if(this->colors.size() > this->tagToId.at(tag))
                return this->colors[this->tagToId.at(tag)];
        return "#808080";
    }

    auto writeToFile(std::string const &str) -> void {
        spdlog::memory_buf_t formatted;
        fmt::format_to(std::back_inserter(formatted), "{}", str);
        this->fileHelper.write(formatted);
    }

    auto writeToJavascriptFile(std::string const &str) -> void {
        spdlog::memory_buf_t formatted;
        fmt::format_to(std::back_inserter(formatted), "{}", str);
        this->fileJavascriptHelper.write(formatted);
        this->fileJavascriptHelper.flush();
    }

    auto toRow(int id, std::string const &tag, spdlog::string_view_t const &str, int offset=0) -> std::string {
        std::string s;
        for(int i = 0; i < id; ++i)
            s += "<td></td>";
        s += std::string("<td style=\"background-color:"+this->tagToColor(tag)+"\">")+std::string(str.data()+offset)+"</td>";
        return s;
    }

    auto threadNumber(int id) -> int {
        if(this->threadIdToNumber.contains(id))
            return this->threadIdToNumber.at(id);
        auto const newId = this->threadIdToNumber.size();
        this->threadIdToNumber[id] = newId;
        return newId;
    }

    spdlog::details::file_helper fileHelper;
    spdlog::details::file_helper fileJavascriptHelper;
    std::map<int, int> threadIdToNumber;
    std::set<std::string> tagsSeen;
    std::vector<std::string> colors = {"#ffccff", "#99ccff", "#66ffff", "#99ff99", "#ffff99", "#ff9966", "#ff99cc", "#ffcc66", "#ff99cc", "#cc66ff", "#33ccff"};
    std::map<std::string, int> tagToId;
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
#define LOG_PATH_SPACE
#define LOG_CODEX
#define LOG_FORGE

