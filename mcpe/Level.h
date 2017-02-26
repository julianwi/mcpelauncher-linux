#pragma once

class Level {

public:

    void** vtable;

    void tick() {
        ((void (*)(Level*)) vtable[19])(this);
    }

};