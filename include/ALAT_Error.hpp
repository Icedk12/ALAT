#ifndef ALAT_ERROR_HPP
#define ALAT_ERROR_HPP

#include <iostream>
#include <string>

/// The status of the LAT program. If this is true, there have been no fatal errors thus far. If false... you know.
inline bool ALAT_HAS_ERRORS = false;

/// The status of the LAT program. If this is 0, there have been no fatal errors thus far. If -1 then you're cooked.
inline int ALAT_EXIT_CODE = false;

/// @brief A function to print an error message to the console. This also sets ALAT_HAS_ERRORS and ALAT_HAS_ERRORS_i to true and -1 respectively. This can be used to cancel the program based on what the error reports.
/// EXAMPLE: push_error("Failed to open file."); would print "[ALAT]: Failed to open file."
/// @param message The message to be displayed to the console, the string "[ALAT]:" is automatically inserted before.
inline void push_error(std::string message)
{
    std::cerr << "[ALAT]: " << message << std::endl;
    ALAT_HAS_ERRORS = true;
    ALAT_EXIT_CODE = -1;
}

/// @brief A function to print an warning message to the console. Use for non-fatal errors.
/// EXAMPLE: push_warning("Failed to open file."); would print "[ALAT]: Failed to open file."
/// @param message The message to be displayed to the console, the string "[ALAT]:" is automatically inserted before.
inline void push_warning(std::string message)
{
    std::cerr << "[ALAT]: " << message << std::endl;
}

#endif