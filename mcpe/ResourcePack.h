#pragma once

#include <string>
class MinecraftEventing;

class PackManifestFactory {
public:

    char filler[4];

    static void (*PackManifestFactory_construct)(PackManifestFactory*, MinecraftEventing&);

    PackManifestFactory(MinecraftEventing& ev) {
        PackManifestFactory_construct(this, ev);
    }

};

class ResourcePackRepository {

public:

    char filler[0x68];

    static void (*ResourcePackRepository_construct)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&);

    ResourcePackRepository(MinecraftEventing& ev, PackManifestFactory& fact) {
        ResourcePackRepository_construct(this, ev, fact);
    }

};

class ResourcePackManager {

public:

    char filler[0x48];

    static void (*ResourcePackManager_construct)(ResourcePackManager*, std::string const&);

    ResourcePackManager(std::string const& str) {
        ResourcePackManager_construct(this, str);
    }

};