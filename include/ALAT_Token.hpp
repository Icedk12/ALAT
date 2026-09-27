#ifndef ALAT_TOKEN_HPP
#define ALAT_TOKEN_HPP

#include <cstddef>

enum class ALAT_TokenType
{
    ADD_OPERATOR,           // Addition
    SUB_OPERATOR,           // Subtraction
    MULT_OPERATOR,          // Multiplication
    DIV_OPERATOR,           // Division

    IF_KEYWORD,             // The keyword for an if statement: e.g 'if'
    FOR_KEYWORD,            // The keyword for a for loop: e.g 'for'
    WHILE_KEYWORD,          // The keyword for a while loop: e.g 'while'

    VAR_DECL_KEYWORD,       // The keyword for defining a variable: e.g 'var'
    LIBRARY_IMPORT_KEYWORD, // The keyword to import a library: e.g '#include' or 'import'
    FUNC_DECL_KEYWORD,      // The keyword for defining a function: e.g 'func' or 'def'

    COMMENT,                // The symbol infront of a comment: e.g '//' or '#'

    IGNORE,                 // Do not translate
};

class ALAT_Token
{
private:
    ALAT_TokenType type = ALAT_TokenType::IGNORE;

public:
    std::size_t start;
    std::size_t length;

    const ALAT_TokenType& getType() const;

    ALAT_Token(ALAT_TokenType type, std::size_t start = 0, std::size_t length = 0);

};


#endif