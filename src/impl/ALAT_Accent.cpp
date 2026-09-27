#include "ALAT_Accent.hpp"

ALAT_Accent::ALAT_Accent(std::string filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        push_error("Could not open file " + filename);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            push_error("ALAT Accent file: '=' not found between terms.");
            continue;
        }

        std::string token = trim(line.substr(0, eq_pos));
        std::string term = trim(line.substr(eq_pos + 1));

        if (token.empty() || term.empty()) {
            push_error("ALAT Accent file: malformed pair.");
            continue;
        }

        accent_map.push_back({stringToToken(token), term});
    }
}