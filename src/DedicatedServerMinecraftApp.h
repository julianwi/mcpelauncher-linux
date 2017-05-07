#pragma once

#include "../mcpe/IMinecraftApp.h"

class DedicatedServerMinecraftApp : public IMinecraftApp {

public:

    virtual bool isDedicatedServer() { return true; }
    virtual void* getAutomationClient() { return nullptr; }
    virtual int getDefaultNetworkMaxPlayers() { return 20; }
    virtual bool isEduMode() { return false; }
    virtual bool canActivateKeyboard() { return true; }

};