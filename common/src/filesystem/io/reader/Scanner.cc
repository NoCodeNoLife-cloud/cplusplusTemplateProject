#include "src/filesystem/io/reader/Scanner.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <istream>
#include <stdexcept>
#include <limits>

namespace common::filesystem {
    Scanner::Scanner(std::istream &input_stream) : input_(input_stream) {
    }

    auto Scanner::nextInt() const -> int32_t {
        std::string token;
        if (!getNextToken(token)) {
            DLOG(ERROR) << "Scanner nextInt failed - no more tokens available";
            throw std::runtime_error("Scanner::nextInt: No more integers available.");
        }

        try {
            // Check for potential overflow before conversion
            size_t pos = 0;
            const long long_value = std::stol(token, &pos);

            if (pos != token.length()) {
                DLOG(ERROR) << fmt::format("Scanner nextInt failed - non-numeric characters in token: '{}'", token);
                throw std::invalid_argument("Scanner::nextInt: Non-numeric characters in token: '" + token + "'");
            }

            if (long_value < std::numeric_limits<int32_t>::min() || long_value > std::numeric_limits<int32_t>::max()) {
                DLOG(ERROR) << fmt::format("Scanner nextInt failed - value out of range: '{}' (value={})", token, long_value);
                throw std::out_of_range("Scanner::nextInt: Value out of range for int32_t: '" + token + "'");
            }

            DLOG(INFO) << fmt::format("Scanner nextInt - successfully parsed: {}", long_value);
            return static_cast<int32_t>(long_value);
        } catch (const std::invalid_argument &e) {
            DLOG(ERROR) << fmt::format("Scanner nextInt failed - invalid argument: '{}', error: {}", token, e.what());
            throw std::invalid_argument("Scanner::nextInt: Invalid argument - '" + token + "'. Error: " + e.what());
        } catch (const std::out_of_range &e) {
            DLOG(ERROR) << fmt::format("Scanner nextInt failed - out of range: '{}', error: {}", token, e.what());
            throw std::out_of_range("Scanner::nextInt: Out of range - '" + token + "'. Error: " + e.what());
        }
    }

    auto Scanner::nextDouble() const -> double {
        std::string token;
        if (!getNextToken(token)) {
            DLOG(ERROR) << "Scanner nextDouble failed - no more tokens available";
            throw std::runtime_error("Scanner::nextDouble: No more doubles available.");
        }

        try {
            size_t pos = 0;
            const double result = std::stod(token, &pos);

            if (pos != token.length()) {
                DLOG(ERROR) << fmt::format("Scanner nextDouble failed - non-numeric characters in token: '{}'", token);
                throw std::invalid_argument("Scanner::nextDouble: Non-numeric characters in token: '" + token + "'");
            }

            DLOG(INFO) << fmt::format("Scanner nextDouble - successfully parsed: {}", result);
            return result;
        } catch (const std::invalid_argument &e) {
            DLOG(ERROR) << fmt::format("Scanner nextDouble failed - invalid argument: '{}', error: {}", token, e.what());
            throw std::invalid_argument("Scanner::nextDouble: Invalid argument - '" + token + "'. Error: " + e.what());
        } catch (const std::out_of_range &e) {
            DLOG(ERROR) << fmt::format("Scanner nextDouble failed - out of range: '{}', error: {}", token, e.what());
            throw std::out_of_range("Scanner::nextDouble: Out of range - '" + token + "'. Error: " + e.what());
        }
    }

    auto Scanner::nextLine() const -> std::string {
        std::string line;
        std::getline(input_, line);
        return line;
    }

    auto Scanner::getNextToken(std::string &token) const -> bool {
        // Skip leading whitespace
        while (input_.peek() != EOF && std::isspace(input_.peek())) {
            input_.get();
        }

        if (input_.peek() == EOF) {
            return false;
        }

        // Read until next whitespace
        token.clear();
        while (input_.peek() != EOF && !std::isspace(input_.peek())) {
            token += static_cast<char>(input_.get());
        }
        return true;
    }

    auto Scanner::nextTokens(const char delimiter) const -> std::vector<std::string> {
        std::string line;
        std::getline(input_, line);
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(std::string(1, delimiter)));

        // Remove empty tokens
        std::erase_if(tokens, [](const std::string &s) { return s.empty(); });
        return tokens;
    }
}
