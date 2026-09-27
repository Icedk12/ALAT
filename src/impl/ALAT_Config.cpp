#include "ALAT_Config.hpp"

#include <cstdlib>
#include <fstream>
#include <string>

#include "ALAT_Error.hpp"

namespace
{
    std::filesystem::path makeAbsolute(const std::filesystem::path& path)
    {
        return std::filesystem::absolute(path).lexically_normal();
    }

    std::filesystem::path defaultAsset(
        const std::filesystem::path& executable_directory,
        const std::filesystem::path& relative_path)
    {
        const auto beside_executable = executable_directory / relative_path;
        if (std::filesystem::exists(beside_executable))
            return makeAbsolute(beside_executable);

        const auto beside_project = executable_directory.parent_path() / relative_path;
        return makeAbsolute(beside_project);
    }
}

ALAT_Config::ALAT_Config(const std::filesystem::path& executable_directory)
{
    const char* override_directory = std::getenv("ALAT_HOME");
    if (override_directory != nullptr && *override_directory != '\0')
    {
        directory = override_directory;
    }
    else
    {
        const char* app_data = std::getenv("APPDATA");
        if (app_data == nullptr || *app_data == '\0')
        {
            push_error("APPDATA is not set; cannot locate ALAT settings.");
            return;
        }
        directory = std::filesystem::path(app_data) / "ALAT";
    }

    directory = makeAbsolute(directory);
    config_file = directory / "config.ini";
    restore_file = directory / "restore.txt";
    language_file = defaultAsset(executable_directory, "languages/cpp.alat");
    accent_file = defaultAsset(executable_directory, "accents/default.alat");

    std::error_code filesystem_error;
    std::filesystem::create_directories(directory, filesystem_error);
    if (filesystem_error)
    {
        push_error("Could not create ALAT settings directory: " + directory.string());
        return;
    }

    if (!std::filesystem::exists(config_file))
    {
        if (!save())
            push_error("Could not create ALAT configuration file: " + config_file.string());
        return;
    }

    std::ifstream input(config_file);
    if (!input.is_open())
    {
        push_error("Could not read ALAT configuration file: " + config_file.string());
        return;
    }

    std::string line;
    while (std::getline(input, line))
    {
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos)
            continue;

        const std::string key = line.substr(0, separator);
        const std::string value = line.substr(separator + 1);
        if (key == "language" && !value.empty())
            language_file = value;
        else if (key == "accent" && !value.empty())
            accent_file = value;
        else if (key == "last_source" && !value.empty())
            last_source_file = value;
    }
}

bool ALAT_Config::save() const
{
    std::error_code filesystem_error;
    std::filesystem::create_directories(directory, filesystem_error);
    if (filesystem_error)
        return false;

    std::ofstream output(config_file, std::ios::trunc);
    if (!output.is_open())
        return false;

    output << "language=" << language_file.string() << '\n'
           << "accent=" << accent_file.string() << '\n'
           << "last_source=" << last_source_file.string() << '\n';
    output.flush();
    return static_cast<bool>(output);
}