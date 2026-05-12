#include "HttpContext.h"
#include "../util/Logger.h"
#include <algorithm>
#include <cctype>

void HttpContext::reset() {
    state_ = State::ExpectRequestLine;
    request_ = HttpRequest{};
    pendingLine_.clear();
    contentLength_ = 0;
}

bool HttpContext::parse(std::string_view data) {
    while (!data.empty()) {
        if (state_ == State::ExpectRequestLine) {
            auto pos = data.find("\r\n");
            if (pos == std::string_view::npos) {
                pendingLine_.append(data);
                return false;
            }
            std::string line = pendingLine_ + std::string(data.substr(0, pos));
            pendingLine_.clear();
            data.remove_prefix(pos + 2);
            if (!parseRequestLine(line)) {
                Logger::warn() << "Malformed request line: " << line;
                return false;
            }
            state_ = State::ExpectHeaders;
        }
        else if (state_ == State::ExpectHeaders) {
            auto pos = data.find("\r\n");
            if (pos == std::string_view::npos) {
                pendingLine_.append(data);
                return false;
            }
            std::string line = pendingLine_ + std::string(data.substr(0, pos));
            pendingLine_.clear();
            data.remove_prefix(pos + 2);

            if (line.empty()) {
                // Blank line = end of headers
                auto it = request_.headers.find("content-length");
                if (it != request_.headers.end()) {
                    contentLength_ = std::stoul(it->second);
                    state_ = State::ExpectBody;
                } else {
                    state_ = State::Complete;
                    return true;
                }
            } else {
                if (!parseHeaderLine(line)) {
                    Logger::warn() << "Malformed header: " << line;
                    return false;
                }
            }
        }
        else if (state_ == State::ExpectBody) {
            size_t needed = contentLength_ - request_.body.size();
            size_t take = std::min(needed, data.size());
            request_.body.append(data.data(), take);
            data.remove_prefix(take);
            if (request_.body.size() == contentLength_) {
                state_ = State::Complete;
                return true;
            }
        }
        else {
            // Already Complete — caller should reset first
            return true;
        }
    }
    return state_ == State::Complete;
}

bool HttpContext::parseRequestLine(std::string_view line) {
    // METHOD SP PATH SP VERSION
    size_t sp1 = line.find(' ');
    if (sp1 == std::string_view::npos) return false;
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos) return false;

    request_.method = line.substr(0, sp1);
    request_.path = line.substr(sp1 + 1, sp2 - sp1 - 1);
    request_.version = line.substr(sp2 + 1);
    return true;
}

bool HttpContext::parseHeaderLine(std::string_view line) {
    auto colon = line.find(':');
    if (colon == std::string_view::npos) return false;

    std::string key(line.substr(0, colon));
    auto valStart = line.find_first_not_of(' ', colon + 1);
    std::string_view val = (valStart != std::string_view::npos)
        ? line.substr(valStart)
        : line.substr(colon + 1);

    // Normalize key to lowercase
    for (auto& c : key) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    request_.headers[std::move(key)] = val;
    return true;
}
