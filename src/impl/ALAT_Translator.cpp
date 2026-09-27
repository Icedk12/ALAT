#include "ALAT_Translator.hpp"

#include <algorithm>
#include <sstream>

bool ALAT_Translator::storeBackup(const std::string& backup_filename)
{
    std::ofstream output(backup_filename, std::ios::trunc);

    if (!output.is_open())
    {
        push_error("Could not create restore backup: " + backup_filename);
        return false;
    } 

    output << main_file.text_content;
    output.flush();
    if (!output)
    {
        push_error("Could not write restore backup: " + backup_filename);
        return false;
    }

    return true;
}

ALAT_Translator::ALAT_Translator(ALAT_Accent my_accent, ALAT_SourceFile main_file)
    : my_accent(my_accent), main_file(main_file) {}

void ALAT_Translator::toMyAccent()
{
    toMyAccent("ALAT_store.txt");
}

void ALAT_Translator::toMyAccent(const std::string& backup_filename)
{
    std::string translated_text;
    std::size_t position = 0;

    for (const ALAT_Token& token : main_file.tokens)
    {
        translated_text.append(main_file.text_content, position, token.start - position);

        // black magic
        const auto pair = std::find_if(
            my_accent.accent_map.begin(),
            my_accent.accent_map.end(),
            [&token](const ALAT_Pair& candidate) {
                return candidate.token.getType() == token.getType();
            });

        if (pair != my_accent.accent_map.end())
            translated_text += pair->term;
        else
            translated_text.append(main_file.text_content, token.start, token.length);

        position = token.start + token.length;
    }

    translated_text.append(main_file.text_content, position, std::string::npos);

    if (!storeBackup(backup_filename)) return;

    std::ofstream output(main_file.filename, std::ios::trunc);
    if (!output.is_open())
    {
        push_error("Could not open source file for writing: " + main_file.filename);
        return;
    }

    output << translated_text;
    output.flush();
    if (!output)
    {
        push_error("Failed while writing translated source file: " + main_file.filename);
        return;
    }

    main_file.text_content = std::move(translated_text);
}

void ALAT_Translator::revertTranslation()
{
    revertTranslation(main_file.filename, "ALAT_store.txt");
    if (ALAT_HAS_ERRORS)
        return;

    std::ifstream restored_file(main_file.filename);
    std::stringstream restored_buffer;
    restored_buffer << restored_file.rdbuf();
    main_file.text_content = restored_buffer.str();
}

void ALAT_Translator::revertTranslation(
    const std::string& source_filename,
    const std::string& backup_filename)
{
    std::ifstream input(backup_filename);

    if (!input.is_open())
    {
        push_error("Could not open restore backup: " + backup_filename);
        return;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    const std::string original_text = buffer.str();

    std::ofstream output(source_filename, std::ios::trunc);
    if (!output.is_open())
    {
        push_error("Could not open source file for restoring: " + source_filename);
        return;
    }

    output << original_text;
    output.flush();
    if (!output)
    {
        push_error("Failed while restoring source file: " + source_filename);
        return;
    }

}