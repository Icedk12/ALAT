#ifndef ALAT_TRANSLATOR_HPP
#define ALAT_TRANSLATOR_HPP

#include <string>
#include <fstream>

#include "ALAT.hpp"

struct ALAT_Translator
{
    
    ALAT_Accent my_accent;
    ALAT_SourceFile main_file;

    ALAT_Translator(ALAT_Accent my_accent, ALAT_SourceFile main_file);

    void toMyAccent();
    void toMyAccent(const std::string& backup_filename);
    void revertTranslation();
    static void revertTranslation(
        const std::string& source_filename,
        const std::string& backup_filename);

private:
    bool storeBackup(const std::string& backup_filename);
};

#endif