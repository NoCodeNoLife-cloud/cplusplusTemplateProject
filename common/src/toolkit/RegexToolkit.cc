#include "src/toolkit/RegexToolkit.hpp"


#include <fmt/format.h>
#include <regex>
#include <string>
#include <vector>
#include <stdexcept>

namespace common::toolkit {
    auto RegexToolkit::is_match(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> bool {
        try {
            const std::regex re(pattern, flags);
            return std::regex_match(text, re);
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::is_match: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::is_search(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> bool {
        try {
            const std::regex re(pattern, flags);
            return std::regex_search(text, re);
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::is_search: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::get_matches(const std::string &text, const std::string &pattern, std::regex_constants::syntax_option_type flags) -> std::vector<std::string> {
        try {
            const std::regex re(pattern, flags);
            std::vector<std::string> results;
            const auto it = std::sregex_iterator(text.begin(), text.end(), re);
            const auto end = std::sregex_iterator();

            for (auto i = it; i != end; ++i) {
                results.push_back(i->str());
            }
            return results;
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::get_matches: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::get_matches_with_groups(const std::string &text, const std::string &pattern, std::regex_constants::syntax_option_type flags) -> std::vector<std::vector<std::string> > {
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
            return results;
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::get_matches_with_groups: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::replace_all(const std::string &text, const std::string &pattern, const std::string &replacement, const std::regex_constants::syntax_option_type flags) -> std::string {
        try {
            const std::regex re(pattern, flags);
            return std::regex_replace(text, re, replacement);
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::replace_all: Invalid regex pattern: " + std::string(e.what()));
        }
    }

    auto RegexToolkit::split(const std::string &text, const std::string &pattern, const std::regex_constants::syntax_option_type flags) -> std::vector<std::string> {
        try {
            const std::regex re(pattern, flags);
            std::sregex_token_iterator it(text.begin(), text.end(), re, -1);
            const std::sregex_token_iterator end;

            std::vector<std::string> results;
            while (it != end) {
                results.push_back(*it++);
            }
            return results;
        } catch (const std::regex_error &e) {
            throw std::invalid_argument("RegexToolkit::split: Invalid regex pattern: " + std::string(e.what()));
        }
    }
}
