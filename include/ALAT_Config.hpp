#ifndef ALAT_CONFIG_HPP
#define ALAT_CONFIG_HPP

#include <filesystem>

struct ALAT_Config
{
    std::filesystem::path directory;
    std::filesystem::path config_file;
    std::filesystem::path restore_file;
    std::filesystem::path language_file;
    std::filesystem::path accent_file;
    std::filesystem::path last_source_file;

    explicit ALAT_Config(const std::filesystem::path& executable_directory);
    bool save() const;
};

#endif