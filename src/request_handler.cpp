#include "request_handler.hpp"
#include "request.hpp"
#include "response.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace {

std::string to_mime_type(const std::string& extension) {
    struct ExtensionMimeType {
        std::string extension;
        std::string mime_type;
    };

    constexpr static auto map = std::array{
        ExtensionMimeType{"gif", "image/gif"},
        ExtensionMimeType{"htm", "text/html"},
        ExtensionMimeType{"html", "text/html"},
        ExtensionMimeType{"jpg", "image/jpeg"},
        ExtensionMimeType{"jpeg", "image/jpeg"},
        ExtensionMimeType{"png", "image/png"},
    };

    for (const auto& pair : map) {
        if (pair.extension == extension) {
            return pair.mime_type;
        }
    }

    return "text/plain";
}

int to_hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }

    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }

    return -1;
}

} // namespace

namespace http {

std::optional<std::string> decode_url(const std::string& str) {
    std::string ret;
    for (auto i = 0uz; i < str.size(); ++i) {
        if (str[i] == '%') { // replace %xx with
            // missing one or both hex values after %
            if ((i + 2) >= str.size()) {
                return {};
            }

            int hi = to_hex(str[i + 1]);
            int lo = to_hex(str[i + 2]);
            if (!hi || !lo) {
                return {};
            }

            i += 2;
            ret += static_cast<char>((hi << 4) | lo);
        } else if (str[i] == '+') { // replace + with ' '
            ret += ' ';
        } else {
            ret += str[i];
        }
    }

    return ret;
}

RequestHandler::RequestHandler(const std::string& root) : root{root} {}

Response RequestHandler::handle(const Request& req) {
    auto decoded = decode_url(req.uri);

    // require decodable, not empty, absolute path and not containing ".."
    if (!decoded || decoded->empty() || !decoded->starts_with('/') ||
        decoded->contains("..")) {
        return Response::from(Response::StatusCode::BadRequest);
    }

    if (decoded->ends_with('/')) {
        decoded->append("index.html");
    }

    namespace fs = std::filesystem;

    const auto path = decoded.value();
    auto last_dot = path.find_last_of('.');
    auto last_slash = path.find_last_of('/');
    auto extension =
        (last_slash != path.npos) && (last_dot > last_slash)
            ? path.substr(1 + last_dot)
            : "";

    const auto full_path = root + path;
    std::ifstream file{full_path, std::ios::binary};
    if (!file) {
        return Response::from(Response::StatusCode::BadRequest);
    }

    Response response;
    response.status = Response::StatusCode::Ok;
    response.content = std::string(fs::file_size(full_path), '\0');
    if (!file.read(response.content.data(), response.content.size())) {
        return Response::from(Response::StatusCode::InternalServerError);
    }

    response.headers.push_back({
        "Content-Length",
        std::to_string(response.content.size()),
    });
    response.headers.push_back({"Content-Type", to_mime_type(extension)});
    return response;
}

} // namespace http