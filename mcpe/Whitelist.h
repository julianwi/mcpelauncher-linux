#pragma once

struct Whitelist {

    char filler[0xc];

};

class WhitelistFile {

public:

    static void (*WhitelistFile_construct)(WhitelistFile*, std::string const&);

    std::string str;
    Whitelist list;

    WhitelistFile(std::string const& str) {
        WhitelistFile_construct(this, str);
    }

};