#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpContext {
public:
    enum class State { ExpectRequestLine, ExpectHeaders, ExpectBody, Complete };

    HttpContext() = default;

    // Returns true when a complete request has been parsed
    bool parse(std::string_view data);

    State state() const noexcept { return state_; }
    const HttpRequest& request() const noexcept { return request_; }
    void reset();

private:
    bool parseRequestLine(std::string_view line);
    bool parseHeaderLine(std::string_view line);

    State state_ = State::ExpectRequestLine;
    HttpRequest request_;
    std::string pendingLine_;
    size_t contentLength_ = 0;
};
