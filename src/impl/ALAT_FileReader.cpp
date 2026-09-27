#include "ALAT_FileReader.hpp"

#include <utility>

namespace
{
    bool isIdentifierCharacter(char character)
    {
        return (character >= 'a' && character <= 'z')
            || (character >= 'A' && character <= 'Z')
            || (character >= '0' && character <= '9')
            || character == '_';
    }
}

void ALAT_FileReader::readFile(ALAT_SourceFile& file_to_read, const ALAT_Language& language)
{
    std::ifstream file(file_to_read.filename);
    if (!file.is_open())
    {
        push_error("Failed to open source file: " + file_to_read.filename);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file_to_read.text_content = buffer.str();

    std::vector<ALAT_Token> tokens{};
    std::size_t position = 0;
    while (position < file_to_read.text_content.size())
    {
        const ALAT_SyntaxRule* matched_rule = nullptr;

        for (const ALAT_SyntaxRule& rule : language.rules)
        {
            if (file_to_read.text_content.compare(position, rule.spelling.size(), rule.spelling) != 0)
                continue;

            const bool starts_with_word = isIdentifierCharacter(rule.spelling.front());
            const bool ends_with_word = isIdentifierCharacter(rule.spelling.back());
            if (starts_with_word && position > 0
                && isIdentifierCharacter(file_to_read.text_content[position - 1]))
                continue;

            const std::size_t end = position + rule.spelling.size();
            if (ends_with_word && end < file_to_read.text_content.size()
                && isIdentifierCharacter(file_to_read.text_content[end]))
                continue;

            if (matched_rule == nullptr || rule.spelling.size() > matched_rule->spelling.size())
                matched_rule = &rule;
        }

        if (matched_rule == nullptr)
        {
            ++position;
            continue;
        }

        const std::size_t token_length = matched_rule->spelling.size();
        tokens.emplace_back(matched_rule->token_type, position, token_length);
        position += token_length;

        if (matched_rule->token_type == ALAT_TokenType::COMMENT)
        {
            while (position < file_to_read.text_content.size()
                && file_to_read.text_content[position] != '\n')
                ++position;
        }
    }

    file_to_read.tokens = std::move(tokens);
}