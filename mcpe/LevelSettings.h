#pragma once

#include <string.h>

class LevelSettings {

public:

    static void (*LevelSettings_construct)(LevelSettings*);

    int seed; // 4
    int gametype; // 8
    int difficulty; // c
    int forcegametype; // 10
    int generator; // 14
    bool loadedInCreative; // 18
    int dimension; // 1c
    int stopTime; // 20
    bool edu; // 21
    float rainLevel, lightningLevel; // 28, 2c
    bool mpGame, lanBroadcast, xblBroadcast, commandsEnabled, texturepacksRequired, overrideSavedSettings; // 2d, 2e, 2f, 30, 31, 32~34
    int sx, sy, sz; // 38, 3c, 40
    char filler[0x58-0x40];

    LevelSettings() {
        LevelSettings_construct(this);
    }
    LevelSettings(LevelSettings const& org) {
        memcpy((void*) this, (void const*) &org, sizeof(LevelSettings));
    }

};