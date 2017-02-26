#pragma once

class Level;

class Minecraft {

public:

    static Level* (*Minecraft_getLevel)(Minecraft*);

    Level* getLevel() {
        return Minecraft_getLevel(this);
    }

};