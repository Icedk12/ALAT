#ifndef ALAT_ACCENT_HPP
#define ALAT_ACCENT_HPP

#include <iostream>
#include <fstream>

#include <vector>
#include <string>

#include "ALAT_Token.hpp"
#include "ALAT_Error.hpp"
#include "ALAT_Utils.hpp"

/// @brief A pair with a token (the token to translate) and the term (what to translate to).
struct ALAT_Pair
{
    ALAT_Token token;
    std::string term;
};

/// @brief The representation of your accent, using a .alat file.
struct ALAT_Accent
{
    std::vector<ALAT_Pair> accent_map;

    ALAT_Accent(std::string filename);
};

#endif