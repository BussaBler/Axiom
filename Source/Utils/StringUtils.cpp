#include "StringUtils.h"
#include "axpch.h"

namespace Axiom {
    std::string StringUtils::trim(const std::string &str) {
        const char *whitespace = " \t\n\r\f\v";
        size_t start = str.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return "";
        }
        size_t end = str.find_last_not_of(whitespace);
        return str.substr(start, end - start + 1);
    }

    std::vector<std::string> StringUtils::split(const std::string &str, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }
} // namespace Axiom
