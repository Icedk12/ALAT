#ifndef ALAT_SRC_FILE_HPP
#define ALAT_SRC_FILE_HPP

#include <vector>
#include <string>

#include "ALAT_Token.hpp"

/// @brief The file you want to translate (.cpp, .c, .py, etc)
struct ALAT_SourceFile 
{
    std::string filename;
    std::vector<ALAT_Token> tokens;

    std::string text_content;

    ALAT_SourceFile(std::string filename);
};

#endif