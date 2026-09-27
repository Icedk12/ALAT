#ifndef ALAT_FILE_READER_HPP
#define ALAT_FILE_READER_HPP

#include <iostream>
#include <fstream>

#include <sstream>
#include <string>
#include <vector>

#include "ALAT_Error.hpp"
#include "ALAT_Language.hpp"
#include "ALAT_Source.hpp"

class ALAT_FileReader 
{
public:
    void readFile(ALAT_SourceFile& file_to_read, const ALAT_Language& language);

    ALAT_FileReader() = default;
};

#endif