#pragma once

#include <string>
#include <functional>
class MinecraftEventing;
class FilePathManager;

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

    static void (*ResourcePackRepository_construct)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&, FilePathManager*);

    ResourcePackRepository(MinecraftEventing& ev, PackManifestFactory& fact, FilePathManager* pm) {
        ResourcePackRepository_construct(this, ev, fact, pm);
    }

};

class ResourcePackManager {

public:

    char filler[0x48];

    static void (*ResourcePackManager_construct)(ResourcePackManager*, std::function<std::string ()> const&);

    ResourcePackManager(std::function<std::string ()> const& f) {
        ResourcePackManager_construct(this, f);
    }

};