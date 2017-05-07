#pragma once

#include <string>

class FilePathManager {

public:

    char filler[0x1C];

    static void (*FilePathManager_construct)(FilePathManager*, std::string, bool);

    FilePathManager(std::string str, bool b) {
        FilePathManager_construct(this, std::move(str), b);
    }


};