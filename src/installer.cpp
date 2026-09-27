#include <algorithm>
#include <array>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <windows.h>
#include <shlobj.h>

#include "installer_resource.h"

namespace
{
    struct EmbeddedFile
    {
        int resource_id;
        const wchar_t* relative_path;
    };

    constexpr std::array<EmbeddedFile, 5> embedded_profiles{{
        {IDR_LANGUAGE_C, L"languages\\c.alat"},
        {IDR_LANGUAGE_CPP, L"languages\\cpp.alat"},
        {IDR_LANGUAGE_GDSCRIPT, L"languages\\gdscript.alat"},
        {IDR_LANGUAGE_PYTHON, L"languages\\python.alat"},
        {IDR_DEFAULT_ACCENT, L"accents\\default.alat"}
    }};

    std::filesystem::path getInstallDirectory()
    {
        std::array<wchar_t, MAX_PATH> local_app_data{};
        const HRESULT result = SHGetFolderPathW(
            nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr,
            SHGFP_TYPE_CURRENT, local_app_data.data());
        if (FAILED(result))
            return {};

        return std::filesystem::path(local_app_data.data()) / L"Programs" / L"ALAT";
    }

    std::wstring trimPathEntry(std::wstring entry)
    {
        while (!entry.empty() && iswspace(entry.front()))
            entry.erase(entry.begin());
        while (!entry.empty() && iswspace(entry.back()))
            entry.pop_back();
        if (entry.size() >= 2 && entry.front() == L'"' && entry.back() == L'"')
            entry = entry.substr(1, entry.size() - 2);

        while (entry.size() > 3 && (entry.back() == L'\\' || entry.back() == L'/'))
            entry.pop_back();
        return entry;
    }

    std::wstring expandEnvironmentStrings(const std::wstring& value)
    {
        const DWORD required = ExpandEnvironmentStringsW(value.c_str(), nullptr, 0);
        if (required == 0)
            return value;

        std::vector<wchar_t> expanded(required);
        if (ExpandEnvironmentStringsW(value.c_str(), expanded.data(), required) == 0)
            return value;
        return expanded.data();
    }

    bool pathContains(const std::vector<std::wstring>& entries, const std::wstring& target)
    {
        const std::wstring normalized_target = trimPathEntry(target);
        for (const std::wstring& entry : entries)
        {
            const std::wstring expanded_entry = trimPathEntry(expandEnvironmentStrings(entry));
            if (_wcsicmp(expanded_entry.c_str(), normalized_target.c_str()) == 0)
                return true;
        }
        return false;
    }

    std::vector<std::wstring> splitPath(const std::wstring& path)
    {
        std::vector<std::wstring> entries;
        std::size_t start = 0;
        while (start < path.size())
        {
            const std::size_t separator = path.find(L';', start);
            const std::size_t length = separator == std::wstring::npos
                ? path.size() - start
                : separator - start;
            if (length != 0)
                entries.push_back(path.substr(start, length));
            if (separator == std::wstring::npos)
                break;
            start = separator + 1;
        }
        return entries;
    }

    std::wstring joinPath(const std::vector<std::wstring>& entries)
    {
        std::wstring result;
        for (const std::wstring& entry : entries)
        {
            if (!result.empty())
                result += L';';
            result += entry;
        }
        return result;
    }

    bool updateUserPath(const std::filesystem::path& install_directory, bool add, std::wstring& error)
    {
        HKEY environment_key = nullptr;
        const LSTATUS open_result = RegCreateKeyExW(
            HKEY_CURRENT_USER, L"Environment", 0, nullptr, REG_OPTION_NON_VOLATILE,
            KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &environment_key, nullptr);
        if (open_result != ERROR_SUCCESS)
        {
            error = L"Could not open the current user's environment settings.";
            return false;
        }

        DWORD value_type = REG_EXPAND_SZ;
        DWORD value_size = 0;
        LSTATUS query_result = RegQueryValueExW(
            environment_key, L"Path", nullptr, &value_type, nullptr, &value_size);

        std::wstring current_path;
        if (query_result == ERROR_SUCCESS)
        {
            if (value_type != REG_SZ && value_type != REG_EXPAND_SZ)
            {
                RegCloseKey(environment_key);
                error = L"The user PATH registry value has an unsupported type.";
                return false;
            }

            std::vector<wchar_t> buffer(value_size / sizeof(wchar_t) + 1, L'\0');
            query_result = RegQueryValueExW(
                environment_key, L"Path", nullptr, &value_type,
                reinterpret_cast<LPBYTE>(buffer.data()), &value_size);
            if (query_result != ERROR_SUCCESS)
            {
                RegCloseKey(environment_key);
                error = L"Could not read the current user's PATH.";
                return false;
            }
            current_path = buffer.data();
        }
        else if (query_result != ERROR_FILE_NOT_FOUND)
        {
            RegCloseKey(environment_key);
            error = L"Could not read the current user's PATH.";
            return false;
        }

        const std::wstring target = install_directory.wstring();
        std::vector<std::wstring> entries = splitPath(current_path);
        const bool already_present = pathContains(entries, target);
        if (add && !already_present)
            entries.push_back(target);
        else if (!add && already_present)
        {
            const std::wstring normalized_target = trimPathEntry(target);
            entries.erase(
                std::remove_if(entries.begin(), entries.end(), [&normalized_target](const std::wstring& entry) {
                    const std::wstring expanded_entry = trimPathEntry(expandEnvironmentStrings(entry));
                    return _wcsicmp(expanded_entry.c_str(), normalized_target.c_str()) == 0;
                }),
                entries.end());
        }

        if (already_present == add)
        {
            RegCloseKey(environment_key);
            return true;
        }

        const std::wstring updated_path = joinPath(entries);
        LSTATUS set_result = ERROR_SUCCESS;
        if (!add && updated_path.empty())
        {
            set_result = RegDeleteValueW(environment_key, L"Path");
            if (set_result == ERROR_FILE_NOT_FOUND)
                set_result = ERROR_SUCCESS;
        }
        else
        {
            set_result = RegSetValueExW(
                environment_key, L"Path", 0, value_type,
                reinterpret_cast<const BYTE*>(updated_path.c_str()),
                static_cast<DWORD>((updated_path.size() + 1) * sizeof(wchar_t)));
        }

        RegCloseKey(environment_key);
        if (set_result != ERROR_SUCCESS)
        {
            error = L"Could not update the current user's PATH.";
            return false;
        }

        DWORD_PTR message_result = 0;
        SendMessageTimeoutW(
            HWND_BROADCAST, WM_SETTINGCHANGE, 0,
            reinterpret_cast<LPARAM>(L"Environment"), SMTO_ABORTIFHUNG,
            5000, &message_result);
        return true;
    }

    bool writeResource(int resource_id, const std::filesystem::path& destination,
        bool overwrite, std::wstring& error)
    {
        std::error_code filesystem_error;
        if (!overwrite && std::filesystem::exists(destination, filesystem_error))
            return true;

        const HRSRC resource = FindResourceW(
            nullptr, MAKEINTRESOURCEW(resource_id), MAKEINTRESOURCEW(10));
        if (resource == nullptr)
        {
            error = L"An installer payload resource is missing.";
            return false;
        }

        const HGLOBAL loaded_resource = LoadResource(nullptr, resource);
        const DWORD resource_size = SizeofResource(nullptr, resource);
        const void* resource_data = LockResource(loaded_resource);
        if (loaded_resource == nullptr || resource_data == nullptr || resource_size == 0)
        {
            error = L"Could not read an installer payload resource.";
            return false;
        }

        std::ofstream output(destination, std::ios::binary | std::ios::trunc);
        if (!output.is_open())
        {
            error = L"Could not write an installed file: " + destination.wstring();
            return false;
        }

        output.write(static_cast<const char*>(resource_data), static_cast<std::streamsize>(resource_size));
        output.flush();
        if (!output)
        {
            error = L"Failed while writing an installed file: " + destination.wstring();
            return false;
        }
        return true;
    }

    bool install(const std::filesystem::path& install_directory, std::wstring& error)
    {
        std::error_code filesystem_error;
        std::filesystem::create_directories(install_directory, filesystem_error);
        if (filesystem_error)
        {
            error = L"Could not create the install directory: " + install_directory.wstring();
            return false;
        }

        if (!writeResource(IDR_ALAT_EXECUTABLE, install_directory / L"alat.exe", true, error))
            return false;

        for (const EmbeddedFile& profile : embedded_profiles)
        {
            const auto destination = install_directory / profile.relative_path;
            std::filesystem::create_directories(destination.parent_path(), filesystem_error);
            if (filesystem_error)
            {
                error = L"Could not create a profile directory under: " + install_directory.wstring();
                return false;
            }
            if (!writeResource(profile.resource_id, destination, false, error))
                return false;
        }

        return updateUserPath(install_directory, true, error);
    }

    bool uninstall(const std::filesystem::path& install_directory, std::wstring& error)
    {
        if (!updateUserPath(install_directory, false, error))
            return false;

        std::error_code filesystem_error;
        std::filesystem::remove_all(install_directory, filesystem_error);
        if (filesystem_error)
        {
            error = L"Could not remove the install directory: " + install_directory.wstring();
            return false;
        }
        return true;
    }

    void showError(const std::wstring& error)
    {
        MessageBoxW(nullptr, error.c_str(), L"ALAT Installer", MB_OK | MB_ICONERROR);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    const auto install_directory = getInstallDirectory();
    if (install_directory.empty())
    {
        showError(L"Could not locate this user's Local AppData folder.");
        return 1;
    }

    const auto installed_executable = install_directory / L"alat.exe";
    const bool is_installed = std::filesystem::is_regular_file(installed_executable);
    const int answer = is_installed
        ? MessageBoxW(nullptr,
            L"ALAT is already installed for this user.\n\nWould you like to uninstall it?",
            L"ALAT Installer", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)
        : MessageBoxW(nullptr,
            L"Install ALAT for this Windows user?\n\nIt will be added to the user PATH and available in new command prompts.",
            L"ALAT Installer", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

    if (answer != IDYES)
        return 0;

    std::wstring error;
    const bool succeeded = is_installed
        ? uninstall(install_directory, error)
        : install(install_directory, error);
    if (!succeeded)
    {
        showError(error);
        return 1;
    }

    MessageBoxW(nullptr,
        is_installed
            ? L"ALAT was uninstalled. Your settings and restore backup were kept."
            : L"ALAT was installed. Open a new command prompt and run 'alat -h'.",
        L"ALAT Installer", MB_OK | MB_ICONINFORMATION);
    return 0;
}