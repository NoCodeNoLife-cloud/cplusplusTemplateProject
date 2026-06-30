/**
 * @file JwtToken.cc
 * @brief JWT token encoder/decoder implementation using OpenSSL HMAC-SHA256
 */

#include <cppforge/starter/security/JwtToken.hpp>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace cppforge::starter::security
{
    JwtToken::JwtToken(std::string secret) : secret_(std::move(secret))
    {
    }

    std::string JwtToken::encode(const std::map<std::string, std::string>& payload) const
    {
        // Header: {"alg":"HS256","typ":"JWT"}
        std::string header = R"({"alg":"HS256","typ":"JWT"})";
        std::string encoded_header = base64UrlEncode(header);

        // Build payload JSON using RapidJSON for proper escaping
        rapidjson::Document doc;
        doc.SetObject();
        for (const auto& [key, value] : payload)
        {
            doc.AddMember(rapidjson::Value(key.c_str(), doc.GetAllocator()).Move(),
                          rapidjson::Value(value.c_str(), doc.GetAllocator()).Move(),
                          doc.GetAllocator());
        }
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string encoded_payload = base64UrlEncode(buffer.GetString());

        // Signature: HMAC-SHA256(header.payload, secret)
        std::string signing_input = encoded_header + "." + encoded_payload;
        std::string signature = hmacSha256(signing_input, secret_);
        std::string encoded_signature = base64UrlEncode(signature);

        return signing_input + "." + encoded_signature;
    }

    std::optional<std::map<std::string, std::string>> JwtToken::decode(const std::string& token) const
    {
        // Split token into parts
        std::vector<std::string> parts;
        std::istringstream stream(token);
        std::string part;
        while (std::getline(stream, part, '.'))
        {
            parts.push_back(part);
        }

        if (parts.size() != 3)
        {
            return std::nullopt;
        }

        // Verify signature using constant-time comparison
        std::string signing_input = parts[0] + "." + parts[1];
        std::string expected_signature = hmacSha256(signing_input, secret_);
        std::string encoded_expected = base64UrlEncode(expected_signature);

        if (encoded_expected.size() != parts[2].size() ||
            CRYPTO_memcmp(encoded_expected.data(), parts[2].data(), encoded_expected.size()) != 0)
        {
            return std::nullopt; // Invalid signature
        }

        // Decode payload using RapidJSON
        std::string payload_json = base64UrlDecode(parts[1]);

        std::map<std::string, std::string> result;
        rapidjson::Document doc;
        doc.Parse(payload_json.c_str());
        if (doc.HasParseError() || !doc.IsObject())
        {
            return std::nullopt;
        }
        for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
        {
            if (it->value.IsString())
            {
                result[it->name.GetString()] = it->value.GetString();
            }
        }

        return result;
    }

    std::string JwtToken::base64UrlEncode(const std::string& input)
    {
        // Standard base64 alphabet
        static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string output;
        output.reserve(((input.size() + 2) / 3) * 4);

        int val = 0;
        int bits = -6;
        const unsigned int mask = 0x3F;

        for (unsigned char c : input)
        {
            val = (val << 8) + c;
            bits += 8;
            while (bits >= 0)
            {
                output.push_back(table[(val >> bits) & mask]);
                bits -= 6;
            }
        }
        if (bits > -6)
        {
            output.push_back(table[((val << 8) >> (bits + 8)) & mask]);
        }

        // Remove padding and convert to base64url
        while (!output.empty() && output.back() == '=')
        {
            output.pop_back();
        }
        std::replace(output.begin(), output.end(), '+', '-');
        std::replace(output.begin(), output.end(), '/', '_');

        return output;
    }

    std::string JwtToken::base64UrlDecode(const std::string& input)
    {
        // Convert base64url to standard base64
        std::string b64 = input;
        std::replace(b64.begin(), b64.end(), '-', '+');
        std::replace(b64.begin(), b64.end(), '_', '/');

        // Add padding
        while (b64.size() % 4 != 0)
        {
            b64 += '=';
        }

        // Decode
        static const int table[256] = {
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
            -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        };

        std::string output;
        output.reserve(b64.size() * 3 / 4);

        int val = 0;
        int bits = -8;

        for (unsigned char c : b64)
        {
            if (table[c] == -1) break;
            val = (val << 6) + table[c];
            bits += 6;
            if (bits >= 0)
            {
                output.push_back(static_cast<char>((val >> bits) & 0xFF));
                bits -= 8;
            }
        }

        return output;
    }

    std::string JwtToken::hmacSha256(const std::string& data, const std::string& key)
    {
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int result_len = 0;

        HMAC(EVP_sha256(),
             key.data(), static_cast<int>(key.size()),
             reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()),
             result, &result_len);

        return std::string(reinterpret_cast<char*>(result), result_len);
    }
}