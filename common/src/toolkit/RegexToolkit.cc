#include "src/toolkit/RegexToolkit.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>

namespace common::toolkit {
    auto RegexToolkit::is_match(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> bool {
        DLOG(INFO) << fmt::format("RegexToolkit is_match - matching text (length: {}) with pattern: {}", text.length(), pattern);
        try {
            const std::regex re(pattern, flags);
            const auto result = std::regex_match(text, re);
            DLOG(INFO) << fmt::format("RegexToolkit is_match succeeded - match: {}", result ? "true" : "false");
            return result;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit is_match failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::is_match: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::is_search(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> bool {
        DLOG(INFO) << fmt::format("RegexToolkit is_search - searching text (length: {}) for pattern: {}", text.length(), pattern);
        try {
            const std::regex re(pattern, flags);
            const auto result = std::regex_search(text, re);
            DLOG(INFO) << fmt::format("RegexToolkit is_search succeeded - found: {}", result ? "true" : "false");
            return result;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit is_search failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::is_search: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::get_matches(const std::string &text, const std::string &pattern, std::regex_constants::syntax_option_type flags) -> std::vector<std::string> {
        DLOG(INFO) << fmt::format("RegexToolkit get_matches - finding matches in text (length: {}) with pattern: {}", text.length(), pattern);
        try {
            const std::regex re(pattern, flags);
            std::vector<std::string> results;
            const auto it = std::sregex_iterator(text.begin(), text.end(), re);
            const auto end = std::sregex_iterator();

            for (auto i = it; i != end; ++i) {
                results.push_back(i->str());
            }
            DLOG(INFO) << fmt::format("RegexToolkit get_matches succeeded - found {} matches", results.size());
            return results;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit get_matches failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::get_matches: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::get_matches_with_groups(const std::string &text, const std::string &pattern, std::regex_constants::syntax_option_type flags) -> std::vector<std::vector<std::string> > {
        DLOG(INFO) << fmt::format("RegexToolkit get_matches_with_groups - finding grouped matches in text (length: {}) with pattern: {}", text.length(), pattern);
        try {
            const std::regex re(pattern, flags);
            std::vector<std::vector<std::string> > results;
            const auto it = std::sregex_iterator(text.begin(), text.end(), re);
            const auto end = std::sregex_iterator();

            for (auto i = it; i != end; ++i) {
                std::vector<std::string> groups;
                for (size_t j = 0; j < i->size(); ++j) {
                    groups.push_back(i->str(j));
                }
                results.push_back(groups);
            }
            DLOG(INFO) << fmt::format("RegexToolkit get_matches_with_groups succeeded - found {} matches with groups", results.size());
            return results;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit get_matches_with_groups failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::get_matches_with_groups: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::replace_all(const std::string &text, const std::string &pattern, const std::string &replacement, const std::regex_constants::syntax_option_type flags) -> std::string {
        DLOG(INFO) << fmt::format("RegexToolkit replace_all - replacing pattern '{}' with '{}' in text (length: {})", pattern, replacement, text.length());
        try {
            const std::regex re(pattern, flags);
            const auto result = std::regex_replace(text, re, replacement);
            DLOG(INFO) << fmt::format("RegexToolkit replace_all succeeded - result length: {}", result.length());
            return result;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit replace_all failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::replace_all: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::split(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> std::vector<std::string> {
        DLOG(INFO) << fmt::format("RegexToolkit split - splitting text (length: {}) by pattern: {}", text.length(), pattern);
        try {
            const std::regex re(pattern, flags);
            std::sregex_token_iterator it(text.begin(), text.end(), re, -1);
            const std::sregex_token_iterator end;

            std::vector<std::string> results;
            while (it != end) {
                results.push_back(*it++);
            }
            DLOG(INFO) << fmt::format("RegexToolkit split succeeded - parts: {}", results.size());
            return results;
        } catch (const std::regex_error &e) {
            DLOG(ERROR) << fmt::format("RegexToolkit split failed - invalid pattern: {}, error: {}", pattern, e.what());
            throw std::invalid_argument("RegexToolkit::split: Invalid regex pattern: " + std::string(e.what()));
        }
    }
}
