#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>
#include "android_symbols.h"
#include "egl_symbols.h"
#include "gles_symbols.h"
#include "fmod_symbols.h"
#include "common.h"
#include "LinuxAppPlatform.h"
#include "../mcpe/Api.h"
#include "../mcpe/Level.h"
#include "../mcpe/Whitelist.h"
#include "../mcpe/OpsList.h"
#include "../mcpe/ResourcePack.h"
#include "../mcpe/Minecraft.h"
#include "../mcpe/MinecraftEventing.h"
#include "../mcpe/UUID.h"
#include "../mcpe/LevelSettings.h"
#include "../mcpe/ServerInstance.h"

extern "C" {
#include "../hybris/include/hybris/dlfcn.h"
#include "../hybris/include/hybris/hook.h"
#include "../hybris/src/jb/linker.h"
}

void stubFunc() {
    std::cout << "warn: stubbed function call\n";
}
void patchNotesModelStub() {
    std::cout << "fetch patch notes: stub\n";
}

int main(int argc, char *argv[]) {
    registerCrashHandler();

    std::cout << "loading hybris libraries\n";
    stubSymbols(android_symbols, (void*) stubFunc);
    stubSymbols(egl_symbols, (void*) stubFunc);
    stubSymbols(gles_symbols, (void*) stubFunc);
    stubSymbols(fmod_symbols, (void*) stubFunc);
    hybris_hook("eglGetProcAddress", (void*) stubFunc);
    hookAndroidLog();
    if (!loadLibrary("libc.so") || !loadLibrary("libstdc++.so") || !loadLibrary("libm.so") || !loadLibrary("libz.so"))
        return -1;
    // load stub libraries
    if (!loadLibrary("libandroid.so") || !loadLibrary("liblog.so") || !loadLibrary("libEGL.so") || !loadLibrary("libGLESv2.so") || !loadLibrary("libOpenSLES.so") || !loadLibrary("libfmod.so") || !loadLibrary("libGLESv1_CM.so"))
        return -1;
    if (!loadLibrary("libmcpelauncher_mod.so"))
        return -1;
    std::cout << "loading MCPE\n";
    void* handle = hybris_dlopen((getCWD() + "libs/libminecraftpe.so").c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "failed to load MCPE: " << hybris_dlerror() << "\n";
        return -1;
    }

    unsigned int libBase = ((soinfo*) handle)->base;
    std::cout << "loaded MCPE (at " << libBase << ")\n";

    std::cout << "apply patches\n";

    unsigned int patchOff = (unsigned int) hybris_dlsym(handle, "_ZN15PatchNotesModel17preloadPatchNotesEv");
    patchCallInstruction((void*) patchOff, (void*) &patchNotesModelStub, true);

    std::cout << "patches applied!\n";

    std::cout << "init symbols\n";
    AppPlatform::myVtable = (void**) hybris_dlsym(handle, "_ZTV11AppPlatform");
    AppPlatform::_singleton = (AppPlatform**) hybris_dlsym(handle, "_ZN11AppPlatform10mSingletonE");
    AppPlatform::AppPlatform_construct = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatformC2Ev");
    AppPlatform::AppPlatform_initialize = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatform10initializeEv");
    AppPlatform::AppPlatform__fireAppFocusGained = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatform19_fireAppFocusGainedEv");

    WhitelistFile::WhitelistFile_construct = (void (*)(WhitelistFile*, std::string const&)) hybris_dlsym(handle, "_ZN13WhitelistFileC2ERKSs");
    OpsFile::OpsFile_construct = (void (*)(OpsFile*, std::string const&)) hybris_dlsym(handle, "_ZN7OpsFileC2ERKSs");
    LevelSettings::LevelSettings_construct = (void (*)(LevelSettings*)) hybris_dlsym(handle, "_ZN13LevelSettingsC2Ev");
    MinecraftEventing::MinecraftEventing_construct = (void (*)(MinecraftEventing*, std::string const&)) hybris_dlsym(handle, "_ZN17MinecraftEventingC2ERKSs");
    Minecraft::Minecraft_getLevel = (Level* (*)(Minecraft*)) hybris_dlsym(handle, "_ZN9Minecraft8getLevelEv");
    PackManifestFactory::PackManifestFactory_construct = (void (*)(PackManifestFactory*, MinecraftEventing&)) hybris_dlsym(handle, "_ZN19PackManifestFactoryC2ER17MinecraftEventing");
    ResourcePackManager::ResourcePackManager_construct = (void (*)(ResourcePackManager*, std::string const&)) hybris_dlsym(handle, "_ZN19ResourcePackManagerC2ERKSs");
    ResourcePackRepository::ResourcePackRepository_construct = (void (*)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&)) hybris_dlsym(handle, "_ZN22ResourcePackRepositoryC2ER17MinecraftEventingR19PackManifestFactory");
    ((void*&) ServerInstance::ServerInstance_construct) = hybris_dlsym(handle, "_ZN14ServerInstanceC2ERK9WhitelistRK7OpsListRKSsNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSs13LevelSettingsRN9minecraft3api3ApiEiiiibRKSt6vectorISsSaISsEESsbRKN3mce4UUIDER17MinecraftEventingR22ResourcePackRepositoryR19ResourcePackManagerPSV_");
    ServerInstance::ServerInstance_update = (void (*)(ServerInstance*)) hybris_dlsym(handle, "_ZN14ServerInstance6updateEv");
    mce::UUID::EMPTY = (mce::UUID*) hybris_dlsym(handle, "_ZN3mce4UUID5EMPTYE");
    mce::UUID::fromString = (mce::UUID (*)(std::string const&)) hybris_dlsym(handle, "_ZN3mce4UUID10fromStringERKSs");

    std::cout << "init app platform vtable\n";
    LinuxAppPlatform::initVtable(handle);
    std::cout << "create app platform\n";
    LinuxAppPlatform* platform = new LinuxAppPlatform();
    std::cout << "init app platform\n";
    platform->initialize();
    std::cout << "app platform initialized\n";

    std::cout << "load white-list and ops-list\n";
    WhitelistFile whitelist ("whitelist.txt");
    OpsFile ops ("ops.txt");
    std::cout << "create minecraft api class\n";
    minecraft::api::Api api;
    api.vtable = hybris_dlsym(handle, "_ZTVN9minecraft3api3Api");
    api.envPath = getCWD();
    api.networkIfaceVtable = hybris_dlsym(handle, "_ZTVN9minecraft3api16NetworkInterface");
    api.playerIfaceVtable = hybris_dlsym(handle, "_ZTVN9minecraft3api15PlayerInterface");
    std::cout << "create level settings\n";
    // init it with test data
    LevelSettings levelSettings;
    levelSettings.gametype = 0;
    levelSettings.forcegametype = 0;
    levelSettings.difficulty = 0;
    levelSettings.dimension = 0;
    levelSettings.generator = 0;
    levelSettings.edu = false;
    levelSettings.sx = 0;
    levelSettings.sy = 100;
    levelSettings.sz = 0;
    levelSettings.mpGame = true;
    levelSettings.lanBroadcast = true;
    levelSettings.commandsEnabled = true;
    levelSettings.texturepacksRequired = false;

    std::cout << "create minecraft eventing\n";
    MinecraftEventing eventing (getCWD());
    std::cout << "create resource pack manager\n";
    ResourcePackManager resourcePackManager (getCWD());
    std::cout << "create pack manifest factory\n";
    PackManifestFactory packManifestFactory (eventing);
    std::cout << "create resource pack repository\n";
    ResourcePackRepository resourcePackRepo (eventing, packManifestFactory);
    std::cout << "create server instance\n";
    ServerInstance instance;
    ServerInstance::ServerInstance_construct(&instance, whitelist.list, ops.list, "data/user/minecraftWorlds/", std::chrono::duration<long long>(0), /* world dir */ "Og0AABoNAAA=", /* world name */ "My World", /* settings */ levelSettings, api, 22, /* (query?) port */ 12345, /* (maybe not) port */ 12346, /* max player count */ 5, /* requiresXboxLive */ false, {}, "normal", /* preload world? */ false, *mce::UUID::EMPTY, eventing, resourcePackRepo, resourcePackManager, nullptr);
    std::cout << "initialized lib\n";;
    while (true) {
        ServerInstance::ServerInstance_update(&instance);
        Level* level = instance.minecraft->getLevel();
        if (level != nullptr)
            level->tick();

        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = 50;
        nanosleep(&tim, &tim2);
    }
    return 0;
}
