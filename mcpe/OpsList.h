#pragma once

struct OpsList {

    char filler[0x10];

};

class OpsFile {

public:

    static void (*OpsFile_construct)(OpsFile*, std::string const&);

    std::string str;
    OpsList list;

    OpsFile(std::string const& str) {
        OpsFile_construct(this, str);
    }

};