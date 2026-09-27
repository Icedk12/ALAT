#include "ALAT_Language.hpp"

#include <fstream>

#include "ALAT_Error.hpp"
#include "ALAT_Utils.hpp"

ALAT_Language::ALAT_Language(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        push_error("Could not open language definition file: " + filename);
        return;
    }

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(file, line))
    {
        ++line_number;
        const std::string trimmed_line = trim(line);
        if (trimmed_line.empty() || trimmed_line.front() == ';')
            continue;

        const std::size_t equals_position = line.find('=');
        if (equals_position == std::string::npos)
        {
            push_error(filename + ":" + std::to_string(line_number) + ": expected TOKEN = spelling.");
            continue;
        }

        const std::string token_name = trim(line.substr(0, equals_position));
        const std::string spelling = trim(line.substr(equals_position + 1));
        if (token_name.empty() || spelling.empty())
        {
            push_error(filename + ":" + std::to_string(line_number) + ": token name and spelling must not be empty.");
            continue;
        }

        const ALAT_Token parsed_token = stringToToken(token_name);
        const ALAT_TokenType token_type = parsed_token.getType();
        if (token_type == ALAT_TokenType::IGNORE)
        {
            push_error(filename + ":" + std::to_string(line_number) + ": unknown token type '" + token_name + "'.");
            continue;
        }

        rules.push_back({token_type, spelling});
    }

    if (rules.empty())
        push_error("Language definition file contains no valid syntax rules: " + filename);
}