#ifndef ALAT_UTILS_HPP
#define ALAT_UTILS_HPP

#include <string>
#include "ALAT_Token.hpp"

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/// @brief A helper function to replace all occurences of a specific string with another.
/// @param str The string you want to replace in.
/// @param from What to replace with "to".
/// @param to What to replace "from" with.
inline void ALAT_replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

inline ALAT_Token stringToToken(const std::string& str)
{
    if (str == "ADD")
    {
        return ALAT_Token{ALAT_TokenType::ADD_OPERATOR};
    }
    if (str == "SUB")
    {
        return ALAT_Token{ALAT_TokenType::SUB_OPERATOR};
    }
    if (str == "MULT")
    {
        return ALAT_Token{ALAT_TokenType::MULT_OPERATOR};
    }
    if (str == "DIV")
    {
        return ALAT_Token{ALAT_TokenType::DIV_OPERATOR};
    }
    if (str == "IF")
    {
        return ALAT_Token{ALAT_TokenType::IF_KEYWORD};
    }
    if (str == "FOR")
    {
        return ALAT_Token{ALAT_TokenType::FOR_KEYWORD};
    }
    if (str == "WHILE")
    {
        return ALAT_Token{ALAT_TokenType::WHILE_KEYWORD};
    }
    if (str == "VAR_DECL")
    {
        return ALAT_Token{ALAT_TokenType::VAR_DECL_KEYWORD};
    }
    if (str == "LIBRARY_IMPORT")
    {
        return ALAT_Token{ALAT_TokenType::LIBRARY_IMPORT_KEYWORD};
    }
    if (str == "FUNC_DECL")
    {
        return ALAT_Token{ALAT_TokenType::FUNC_DECL_KEYWORD};
    }
    if (str == "COMMENT")
    {
        return ALAT_Token{ALAT_TokenType::COMMENT};
    }
    else
    {
        return ALAT_Token{ALAT_TokenType::IGNORE};
    }
}

#endif