/*
========================================================================
=  Advanced Language Accent Translator, Copyright Tom Patton-Low 2026  =
========================================================================
Hey! Do not steal ALAT.

The entirety of the backend:
    - translator
    - file reader
    - program design
was made by me. The application installer was not made by me, feel free to steal that.

You are allowed to download, modify and use the source code of Advanced Language Accent Translator, on the conditions that you:
    1. Do not list any modified code as your own.
    2. Do not redistribute without crediting.
*/

#include "ALAT.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include <windows.h>
#include <shellapi.h>

namespace
{
    std::filesystem::path absolutePath(const std::string& value)
    {
        std::error_code error;
        const auto result = std::filesystem::absolute(value, error);
        return error ? std::filesystem::path(value) : result.lexically_normal();
    }

    bool isRegularFile(const std::filesystem::path& path)
    {
        std::error_code error;
        return std::filesystem::is_regular_file(path, error);
    }

    void printHelp()
    {
        std::cout
            << "ALAT - language accent translator\n\n"
            << "Usage:\n"
            << "  alat lang <filename>  Set the source-language definition file\n"
            << "  alat acc <filename>   Set the target accent file\n"
            << "  alat tran <filename>  Translate a source file in place\n"
            << "  alat rest             Restore the last translated source file\n"
            << "  alat stat             Show current configuration\n"
            << "  alat lang -o          Open the configured language file\n"
            << "  alat acc -o           Open the configured accent file\n"
            << "  alat rest -o          Open the saved restore file\n"
            << "  alat -h               Show this help\n\n"
            << "Translation replaces the source file. Its original contents are\n"
            << "saved under your ALAT user settings directory for 'alat rest'.\n";
    }

    bool openIfPresent(const std::filesystem::path& path)
    {
        if (!isRegularFile(path))
            return true;

        const auto result = reinterpret_cast<INT_PTR>(
            ShellExecuteA(nullptr, "open", path.string().c_str(), nullptr, nullptr, SW_SHOWNORMAL));
        if (result <= 32)
        {
            push_error("Could not open file: " + path.string());
            return false;
        }
        return true;
    }

    void printConfiguration(const ALAT_Config& config)
    {
        std::cout << "Language: " << config.language_file.string() << '\n'
                  << "Accent:   " << config.accent_file.string() << '\n'
                  << "Restore:  " << config.restore_file.string() << '\n'
                  << "Source:   ";
        if (config.last_source_file.empty())
            std::cout << "(none)\n";
        else
            std::cout << config.last_source_file.string() << '\n';
    }

    int setConfigFile(ALAT_Config& config, const std::string& filename, bool is_language)
    {
        const auto path = absolutePath(filename);
        if (!isRegularFile(path))
        {
            push_error("File does not exist: " + path.string());
            return ALAT_EXIT_CODE;
        }

        if (is_language)
            config.language_file = path;
        else
            config.accent_file = path;

        if (!config.save())
        {
            push_error("Could not save ALAT configuration.");
            return ALAT_EXIT_CODE;
        }

        std::cout << (is_language ? "Language" : "Accent")
                  << " file set to: " << path.string() << '\n';
        return ALAT_EXIT_CODE;
    }

    int translate(ALAT_Config& config, const std::string& filename)
    {
        const auto source_path = absolutePath(filename);
        if (!isRegularFile(source_path))
        {
            push_error("Source file does not exist: " + source_path.string());
            return ALAT_EXIT_CODE;
        }
        if (!isRegularFile(config.language_file))
        {
            push_error("Language file is missing. Set it with 'alat lang <filename>'.");
            return ALAT_EXIT_CODE;
        }
        if (!isRegularFile(config.accent_file))
        {
            push_error("Accent file is missing. Set it with 'alat acc <filename>'.");
            return ALAT_EXIT_CODE;
        }

        ALAT_Language language{config.language_file.string()};
        ALAT_Accent accent{config.accent_file.string()};
        ALAT_SourceFile source{source_path.string()};
        if (ALAT_HAS_ERRORS)
            return ALAT_EXIT_CODE;

        ALAT_FileReader reader{};
        reader.readFile(source, language);
        if (ALAT_HAS_ERRORS)
            return ALAT_EXIT_CODE;

        config.last_source_file.clear();
        if (!config.save())
        {
            push_error("Could not save ALAT configuration before translation.");
            return ALAT_EXIT_CODE;
        }

        ALAT_Translator translator{accent, source};
        translator.toMyAccent(config.restore_file.string());
        if (ALAT_HAS_ERRORS)
            return ALAT_EXIT_CODE;

        config.last_source_file = source_path;
        if (!config.save())
        {
            ALAT_Translator::revertTranslation(source_path.string(), config.restore_file.string());
            push_error("Could not save restore information; attempted to restore the source file.");
            return ALAT_EXIT_CODE;
        }

        std::cout << "Translated: " << source_path.string() << '\n';
        return ALAT_EXIT_CODE;
    }
}

int main(int argc, char* argv[])
{
    if (argc == 1 || (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")))
    {
        printHelp();
        return 0;
    }

    const auto executable_path = absolutePath(argv[0]);
    auto executable_directory = executable_path.parent_path();
    if (!std::filesystem::exists(executable_directory / "languages" / "cpp.alat")
        && std::filesystem::exists(executable_directory.parent_path() / "languages" / "cpp.alat"))
        executable_directory = executable_directory.parent_path();

    ALAT_Config config{executable_directory};
    if (ALAT_HAS_ERRORS)
        return ALAT_EXIT_CODE;

    const std::string command = argv[1];
    if (command == "lang" || command == "acc")
    {
        const bool is_language = command == "lang";
        if (argc == 3 && std::string(argv[2]) == "-o")
        {
            openIfPresent(is_language ? config.language_file : config.accent_file);
            return ALAT_EXIT_CODE;
        }
        if (argc == 3)
            return setConfigFile(config, argv[2], is_language);
    }
    else if (command == "tran" && argc == 3)
    {
        return translate(config, argv[2]);
    }
    else if (command == "rest")
    {
        if (argc == 3 && std::string(argv[2]) == "-o")
        {
            openIfPresent(config.restore_file);
            return ALAT_EXIT_CODE;
        }
        if (argc == 2)
        {
            if (config.last_source_file.empty())
            {
                push_error("There is no recorded translation to restore.");
                return ALAT_EXIT_CODE;
            }
            if (!isRegularFile(config.restore_file))
            {
                push_error("The restore file does not exist: " + config.restore_file.string());
                return ALAT_EXIT_CODE;
            }

            ALAT_Translator::revertTranslation(
                config.last_source_file.string(), config.restore_file.string());
            if (!ALAT_HAS_ERRORS)
                std::cout << "Restored: " << config.last_source_file.string() << '\n';
            return ALAT_EXIT_CODE;
        }
    }
    else if (command == "stat" && argc == 2)
    {
        printConfiguration(config);
        return 0;
    }

    push_error("Invalid command or arguments. Run 'alat -h' for help.");
    return ALAT_EXIT_CODE;
}