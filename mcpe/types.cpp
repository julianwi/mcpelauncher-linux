#include <string>
#include <vector>

#include "AppPlatform.h"

AppPlatform** AppPlatform::_singleton = nullptr;
void** AppPlatform::myVtable = nullptr;
void (*AppPlatform::AppPlatform_construct)(AppPlatform*);
void (*AppPlatform::AppPlatform__fireAppFocusGained)(AppPlatform*);
void (*AppPlatform::AppPlatform_initialize)(AppPlatform*);

#include "App.h"

void (*App::App_init)(App*, AppContext&);

#include "MinecraftClient.h"

void (*MinecraftClient::MinecraftClient_construct)(MinecraftClient*, int, char**);
void (*MinecraftClient::MinecraftClient_update)(MinecraftClient*);
void (*MinecraftClient::MinecraftClient_setRenderingSize)(MinecraftClient*, int, int);
void (*MinecraftClient::MinecraftClient_setUISizeAndScale)(MinecraftClient*, int, int, float);
Options* (*MinecraftClient::MinecraftClient_getOptions)(MinecraftClient*);

#include "Options.h"

bool (*Options::Options_getFullscreen)(Options*);
void (*Options::Options_setFullscreen)(Options*, bool);

#include "gl.h"

std::string (*gl::getOpenGLVendor)();
std::string (*gl::getOpenGLRenderer)();
std::string (*gl::getOpenGLVersion)();
std::string (*gl::getOpenGLExtensions)();
void (*mce::Platform::OGL::OGL_initBindings)();

#include "Mouse.h"

void (*Mouse::feed)(char, char, short, short, short, short);

#include "Keyboard.h"

void (*Keyboard::Keyboard_feedText)(const std::string&, bool, unsigned char);
std::vector<KeyboardAction>* Keyboard::inputs;
int* Keyboard::states;

#include "Whitelist.h"

void (*WhitelistFile::WhitelistFile_construct)(WhitelistFile*, std::string const&);

#include "OpsList.h"

void (*OpsFile::OpsFile_construct)(OpsFile*, std::string const&);

#include "LevelSettings.h"

void (*LevelSettings::LevelSettings_construct)(LevelSettings*);

#include "MinecraftEventing.h"

void (*MinecraftEventing::MinecraftEventing_construct)(MinecraftEventing*, std::string const&);

#include "ResourcePack.h"

void (*PackManifestFactory::PackManifestFactory_construct)(PackManifestFactory*, MinecraftEventing&);
void (*ResourcePackRepository::ResourcePackRepository_construct)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&);
void (*ResourcePackManager::ResourcePackManager_construct)(ResourcePackManager*, std::string const&);

#include "Minecraft.h"

Level* (*Minecraft::Minecraft_getLevel)(Minecraft*);

#include "UUID.h"

mce::UUID* mce::UUID::EMPTY;
mce::UUID (*mce::UUID::fromString)(std::string const&);

#include "ServerInstance.h"

void (*ServerInstance::ServerInstance_construct)(ServerInstance*, Whitelist const&, OpsList const&, std::string const&, std::chrono::duration<long long>, std::string, std::string, LevelSettings, minecraft::api::Api&, int, int, int, int, bool, std::vector<std::string> const&, std::string, bool, mce::UUID const&, MinecraftEventing&, ResourcePackRepository&, ResourcePackManager&, ResourcePackManager*);
void (*ServerInstance::ServerInstance_update)(ServerInstance*);