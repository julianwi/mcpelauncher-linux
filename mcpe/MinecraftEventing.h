#pragma once

class MinecraftEventing {

public:

    static void (*MinecraftEventing_construct)(MinecraftEventing*, std::string const&);

    char filler[0x48];

    MinecraftEventing(std::string const& str) {
        MinecraftEventing_construct(this, str);
    }

};