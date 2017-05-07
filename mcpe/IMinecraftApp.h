#pragma once

class IMinecraftApp {

public:

    virtual ~IMinecraftApp() { }
    virtual bool isDedicatedServer() = 0;
    virtual void* getAutomationClient() = 0;
    virtual int getDefaultNetworkMaxPlayers() = 0;
    virtual bool isEduMode() = 0;
    virtual bool canActivateKeyboard() = 0;

};