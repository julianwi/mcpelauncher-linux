#pragma once

#include <chrono>
#include <vector>
#include <string>

class IMinecraftApp;
class Minecraft;
class Whitelist;
class OpsList;
class LevelSettings;
namespace minecraft { namespace api { class Api; } }
namespace mce { class UUID; }
class MinecraftEventing;
class ResourcePackRepository;
class ResourcePackManager;

class ServerInstance {

public:

    char filler[0xc];
    Minecraft* minecraft;
    char filler2[0x200];

    static void (*ServerInstance_construct)(ServerInstance*, IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, std::string, std::string, std::string, std::string, std::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<std::string> const&, std::string, bool, mce::UUID const&, MinecraftEventing&, ResourcePackRepository&, ResourcePackManager&, ResourcePackManager*);
    static void (*ServerInstance_update)(ServerInstance*);

    void update() {
        ServerInstance_update(this);
    }

};