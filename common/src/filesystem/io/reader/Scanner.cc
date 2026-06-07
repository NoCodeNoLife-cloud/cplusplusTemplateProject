/**
 * @file Scanner.cc
 * @brief Scanner class implementation
 * @details This file contains the implementation of the Scanner class methods for Common library utilities.
 */

#include "filesystem/io/reader/Scanner.hpp"

#include <istream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <fmt/format.h>

namespace common::filesystem::io::reader
{
    Scanner::Scanner(std::istream& input_stream) : input_(input_stream)
    {
    }

    int32_t Scanner::nextInt()
    {
        std::string token;
        if (!getNextToken(token))
        {
            throw std::runtime_error("Scanner::nextInt: No more integers available.");
        }

        try
        {
            // Check for potential overflow before conversion
            size_t pos = 0;
            const long long_value = std::stol(token, &pos);

            if (pos != token.length())
            {
                throw std::invalid_argument("Scanner::nextInt: Non-numeric characters in token: '" + token + "'");
            }

            if (long_value < std::numeric_limits<int32_t>::min() || long_value > std::numeric_limits<int32_t>::max())
            {
                throw std::out_of_range("Scanner::nextInt: Value out of range for int32_t: '" + token + "'");
            }
            return static_cast<int32_t>(long_value);
        }
        catch (const std::invalid_argument& e)
        {
            throw std::invalid_argument("Scanner::nextInt: Invalid argument - '" + token + "'. Error: " + e.what());
        }
        catch (const std::out_of_range& e)
        {
            throw std::out_of_range("Scanner::nextInt: Out of range - '" + token + "'. Error: " + e.what());
        }
    }

    double Scanner::nextDouble()
    {
        std::string token;
        if (!getNextToken(token))
        {
            throw std::runtime_error("Scanner::nextDouble: No more doubles available.");
        }

        try
        {
            size_t pos = 0;
            const double result = std::stod(token, &pos);

            if (pos != token.length())
            {
                throw std::invalid_argument("Scanner::nextDouble: Non-numeric characters in token: '" + token + "'");
            }
            return result;
        }
        catch (const std::invalid_argument& e)
        {
            throw std::invalid_argument("Scanner::nextDouble: Invalid argument - '" + token + "'. Error: " + e.what());
        }
        catch (const std::out_of_range& e)
        {
            throw std::out_of_range("Scanner::nextDouble: Out of range - '" + token + "'. Error: " + e.what());
        }
    }

    std::string Scanner::nextLine()
    {
        std::string line;
        std::getline(input_, line);
        return line;
    }

    bool Scanner::getNextToken(std::string& token)
    {
        // Skip leading whitespace
        while (input_.peek() != EOF && std::isspace(input_.peek()))
        {
            input_.get();
        }

        if (input_.peek() == EOF)
        {
            return false;
        }

        // Read until next whitespace
        token.clear();
        while (input_.peek() != EOF && !std::isspace(input_.peek()))
        {
            token += static_cast<char>(input_.get());
        }
        return true;
    }

    std::vector<std::string> Scanner::nextTokens(const char delimiter)
    {
        std::string line;
        std::getline(input_, line);
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, delimiter))
        {
            if (!token.empty())
            {
                tokens.push_back(std::move(token));
            }
        }
        return tokens;
    }
}