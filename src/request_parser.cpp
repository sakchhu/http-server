#include <request_parser.hpp>

#include <request.hpp>
#include <string_view>

using http::Request;
using http::RequestParser;

namespace {

bool is_control_character(int c) { return (c >= 0 && c <= 32) || (c == 127); }

bool is_special(char c) {
    static constexpr std::string_view specials_table{"(){}[]<>@,\";:\\/?= \t"};
    return specials_table.find(c) != specials_table.npos;
}

bool is_alpha(int c) { return c >= 0 && c <= 127; }
bool is_digit(int c) { return c >= '0' && c <= '9'; }
bool is_invalid(int c) {
    return !is_alpha(c) || is_control_character(c) || is_special(c);
}

} // namespace

auto RequestParser::parse(Request& req, std::string_view input) noexcept
    -> Result {
    for (auto it = input.begin(); it != input.end(); ++it) {
        if (auto err = consume(req, *it); err != Result::Indeterminate) {
            return err;
        }
    }
    return Result::Indeterminate;
}

auto RequestParser::consume(Request& req, char input) -> Result {
    switch (state) {
    case State::MethodStart:
        // First character has to be alphabetical
        if (!is_alpha(input) || is_invalid(input)) {
            return Result::Invalid;
        }
        state = State::Method;
        req.method.push_back(input);
        return Result::Indeterminate;

    case State::Method:
        if (input == ' ') {
            state = State::Uri;
            return Result::Indeterminate;
        } else if (is_invalid(input)) {
            return Result::Invalid;
        }
        req.method.push_back(input);
        return Result::Indeterminate;

    case State::Uri:
        if (input == ' ') {
            state = State::HttpVersionH;
            return Result::Indeterminate;
        } else if (is_control_character(input)) {
            return Result::Invalid;
        }
        req.uri.push_back(input);
        return Result::Indeterminate;

    case State::HttpVersionH:
        if (input == 'H') {
            state = State::HttpVersionT1;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionT1:
        if (input == 'T') {
            state = State::HttpVersionT2;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionT2:
        if (input == 'T') {
            state = State::HttpVersionP;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionP:
        if (input == 'P') {
            state = State::HttpVersionSlash;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionSlash:
        if (input == '/') {
            state = State::HttpVersionMajorStart;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionMajorStart:
        if (is_digit(input)) {
            state = State::HttpVersionMajor;
            req.version.major = input - '0';
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionMajor:
        if (is_digit(input)) {
            req.version.major = req.version.major * 10 + input - '0';
            return Result::Indeterminate;
        } else if (input == '.') {
            state = State::HttpVersionMinorStart;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionMinorStart:
        if (is_digit(input)) {
            state = State::HttpVersionMinor;
            req.version.minor = input - '0';
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionMinor:
        if (is_digit(input)) {
            req.version.minor = req.version.minor * 10 + input - '0';
            return Result::Indeterminate;
        } else if (input == '\r') {
            state = State::HttpVersionEnd;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HttpVersionEnd:
        if (input == '\n') {
            state = State::HeaderLineStart;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::HeaderLineStart:
        if (input == '\r') {
            // HeaderName: Value\r\n
            // \r\n
            state = State::RequestEnd;
            return Result::Indeterminate;
        } else if (is_invalid(input)) {
            return Result::Invalid;
        }

        state = State::HeaderName;
        req.headers.emplace_back();
        req.headers.back().name.push_back(input);
        return Result::Indeterminate;

    case State::HeaderName:
        if (input == ':') {
            state = State::HeaderNameEnd;
            return Result::Indeterminate;
        } else if (is_invalid(input)) {
            return Result::Invalid;
        }
        req.headers.back().name.push_back(input);
        return Result::Indeterminate;

    case State::HeaderNameEnd:
        if (input == ' ') {
            state = State::HeaderValue;
            return Result::Indeterminate;
        } else {
            return Result::Invalid;
        }

    case State::HeaderValue:
        if (input == '\r') {
            state = State::HeaderValueEnd;
            return Result::Indeterminate;
        } else if (is_control_character(input)) {
            return Result::Invalid;
        }
        req.headers.back().value.push_back(input);
        return Result::Indeterminate;

    case State::HeaderValueEnd:
        if (input == '\n') {
            state = State::HeaderLineStart;
            return Result::Indeterminate;
        }
        return Result::Invalid;

    case State::RequestEnd:
        return (input == '\n') ? Result::Complete : Result::Invalid;
    default:
        return Result::Invalid;
    }
}