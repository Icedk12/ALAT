#ifndef ALAT_LANGUAGE_HPP
#define ALAT_LANGUAGE_HPP

#include <string>
#include <vector>

#include "ALAT_Token.hpp"

struct ALAT_SyntaxRule
{
    ALAT_TokenType token_type;
    std::string spelling;
};

struct ALAT_Language
{
    std::vector<ALAT_SyntaxRule> rules;

    explicit ALAT_Language(const std::string& filename);
};

#endif