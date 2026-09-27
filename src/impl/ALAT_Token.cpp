#include "ALAT_Token.hpp"

const ALAT_TokenType &ALAT_Token::getType() const { return type; }

ALAT_Token::ALAT_Token(ALAT_TokenType type, std::size_t start, std::size_t length)
    : type(type), start(start), length(length) {}