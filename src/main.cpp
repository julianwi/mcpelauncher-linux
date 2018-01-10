#include <iostream>
#include <dlfcn.h>
#include <stdarg.h>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <codecvt>
#include <locale>
#include <dirent.h>
#include <fstream>
#include <X11/Xlib.h>
#include <functional>
#include <sys/mman.h>
#include <EGL/egl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "symbols/gles_symbols.h"
#include "symbols/android_symbols.h"
#include "symbols/egl_symbols.h"
#include "symbols/fmod_symbols.h"
#include "symbols/libm_symbols.h"
#include "minecraft/symbols.h"
#include "minecraft/gl.h"
#include "minecraft/AppPlatform.h"
#include "minecraft/MinecraftGame.h"
#include "minecraft/Mouse.h"
#include "minecraft/Keyboard.h"
#include "minecraft/Options.h"
#include "minecraft/Common.h"
#include "minecraft/Xbox.h"
#include "minecraft/MultiplayerService.h"
#include "linux_appplatform.h"
#include "linux_store.h"
#include "common.h"
#include "hook.h"
#include "xboxlive.h"
#include "extract.h"
#ifndef DISABLE_CEF
#include "browser.h"
#include "xbox_login_browser.h"
#include "initial_setup_browser.h"
#include "path_helper.h"
#endif
#ifndef DISABLE_PLAYAPI
#include "google_login_browser.h"

#endif

extern "C" {

#include <eglut.h>
#include "../hybris/include/hybris/dlfcn.h"
#include "../hybris/include/hybris/hook.h"
#include "../hybris/src/jb/linker.h"

}

void androidStub() {
    Log::warn("Launcher", "Android function stub call");
}

void eglStub() {
    Log::warn("Launcher", "EGL function stub call");
}

std::unique_ptr<LinuxStore> createStoreHookFunc(const mcpe::string& idk, StoreListener& listener) {
    Log::trace("Launcher", "Creating fake store (%s)", idk.c_str());
    return std::unique_ptr<LinuxStore>(new LinuxStore());
}

class HTTPRequest;

class LinuxHttpRequestInternal {
public:
    void* vtable;
    int filler1;
    HTTPRequest* request;

    void destroy() {
        Log::trace("Launcher", "LinuxHttpRequestInternal::~LinuxHttpRequestInternal");
    }
};
void** linuxHttpRequestInternalVtable;

void constructLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal, HTTPRequest* request) {
    requestInternal->vtable = linuxHttpRequestInternalVtable;
    requestInternal->request = request;
}

void sendLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal) {
    Log::trace("Launcher", "HTTPRequestInternalAndroid::send stub called");
    // TODO: Implement it
}

void abortLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal) {
    Log::trace("Launcher", "HTTPRequestInternalAndroid::abort stub called");
    // TODO: Implement it
}


static MinecraftGame* client;
static LinuxAppPlatform* platform;

int winId = 0;
bool moveMouseToCenter = false;

static void minecraft_idle() {
    if (client->wantToQuit()) {
        delete client;
        eglutDestroyWindow(winId);
        eglutFini();
        return;
    }
    int cx = eglutGetWindowWidth() / 2;
    int cy = eglutGetWindowHeight() / 2;
    if (moveMouseToCenter) {
        eglutWarpMousePointer(cx, cy);
        moveMouseToCenter = false;
    }
    eglutPostRedisplay();
}
static void minecraft_draw() {
    platform->runOnMainThreadMutex.lock();
    auto queue = std::move(platform->runOnMainThreadQueue);
    platform->runOnMainThreadMutex.unlock();
    for (auto const& func : queue)
        func();
    client->update();
}
float pixelSize = 2.f;
int oldw = -1, oldh = -1;
static void minecraft_reshape(int w, int h) {
    if (oldw == w && oldh == h)
        return;
    oldw = w;
    oldh = h;
    client->setRenderingSize(w, h);
    client->setUISizeAndScale(w, h, pixelSize);
}
static void minecraft_mouse(int x, int y) {
    if (LinuxAppPlatform::mousePointerHidden) {
        int cx = eglutGetWindowWidth() / 2;
        int cy = eglutGetWindowHeight() / 2;
        if (x != cy || y != cy) {
            Mouse::feed(0, 0, x, y, x - cx, y - cy);
            moveMouseToCenter = true;
        }
    } else {
        Mouse::feed(0, 0, x, y, 0, 0);
    }
}
static void minecraft_mouse_button(int x, int y, int btn, int action) {
    int mcBtn = (btn == 1 ? 1 : (btn == 2 ? 3 : (btn == 3 ? 2 : (btn == 5 ? 4 : btn))));
    Mouse::feed((char) mcBtn, (char) (action == EGLUT_MOUSE_PRESS ? (btn == 5 ? -120 : (btn == 4 ? 120 : 1)) : 0), x, y, 0, 0);
}

int getKeyMinecraft(int keyCode) {
    if (keyCode == 65505)
        return 16;
    if (keyCode >= 97 && keyCode <= 122)
        return (keyCode + 65 - 97);
    if (keyCode >= 65361 && keyCode <= 65364)
        return (keyCode + 37 - 65361);
    if (keyCode >= 65470 && keyCode <= 65481)
        return (keyCode + 112 - 65470);

    return keyCode;
}
static void minecraft_keyboard(char str[5], int action) {
    if (strcmp(str, "\t") == 0 || strcmp(str, "\26") == 0 || strcmp(str, "\33") == 0) // \t, paste, esc
        return;
    if (action == EGLUT_KEY_PRESS || action == EGLUT_KEY_REPEAT) {
        if (str[0] == 13) {
            str[0] = 10;
            str[1] = 0;
        }
        std::stringstream ss;
        ss << str;
        Keyboard::feedText(ss.str(), false, 0);
    }
}
bool modCTRL = false;
static void minecraft_keyboard_special(int key, int action) {
    if (key == 65507)
        modCTRL = (action != EGLUT_KEY_RELEASE);
    if (modCTRL && (key == 86 || key == 118) && action == EGLUT_KEY_PRESS) {
        eglutRequestPaste();
    }
    if (key == 65480) {
        if (action == EGLUT_KEY_PRESS) {
            client->getPrimaryUserOptions()->setFullscreen(!client->getPrimaryUserOptions()->getFullscreen());
        }
        return;
    }
    int mKey = getKeyMinecraft(key);
    if (action == EGLUT_KEY_PRESS) {
        Keyboard::feed((unsigned char) mKey, 1);
        //Keyboard::states[mKey] = 1;
    } else if (action == EGLUT_KEY_RELEASE) {
        Keyboard::feed((unsigned char) mKey, 0);
        //Keyboard::states[mKey] = 0;
    }
}
static void minecraft_paste(const char* str, int len) {
    for (int i = 0; i < len; i++) {
        char c = str[i];
        int l = 1;
        if ((c & 0b11110000) == 0b11100000)
            l = 3;
        else if ((c & 0b11100000) == 0b11000000)
            l = 2;
        Keyboard::feedText(mcpe::string(&str[i], (size_t) l), false, 0);
    }
}
static void minecraft_close() {
    client->quit();
}

void detachFromJavaStub() {
    Log::trace("Launcher", "detach_from_java stub called");
}
void* getJVMEnvStub() {
    Log::trace("Launcher", "get_jvm_env stub called");
    return nullptr;
}
bool verifyCertChainStub() {
    Log::trace("Launcher", "verify_cert_chain_platform_specific stub called");
    return true;
}
mcpe::string xboxReadConfigFile(void* th) {
    Log::trace("Launcher", "Reading xbox config file");
    std::ifstream f(PathHelper::findDataFile("assets/xboxservices.config"));
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}
void workerPoolDestroy(void* th) {
    Log::trace("Launcher", "worker pool-related class destroy %uli", (unsigned long) th);
}
xbox::services::xbox_live_result<void> xboxLogTelemetrySignin(void* th, bool b, mcpe::string const& s) {
    Log::trace("Launcher", "log_telemetry_signin %i %s", (int) b, s.c_str());
    xbox::services::xbox_live_result<void> ret;
    ret.code = 0;
    ret.error_code_category = xbox::services::xbox_services_error_code_category();
    ret.message = " ";
    return ret;
}
mcpe::string xboxGetLocalStoragePath() {
    return "data/";
}
xbox::services::xbox_live_result<void> xboxInitSignInActivity(void*, int requestCode) {
    Log::trace("Launcher", "init_sign_in_activity %i", requestCode);
    xbox::services::xbox_live_result<void> ret;
    ret.code = 0;
    ret.error_code_category = xbox::services::xbox_services_error_code_category();

    if (requestCode == 1) { // silent signin
        auto account = XboxLiveHelper::getMSAStorageManager()->getAccount();
        xbox::services::system::java_rps_ticket ticket;
        if (account) {
            auto tokens = account->requestTokens({{"user.auth.xboxlive.com", "mbi_ssl"}});
            auto xboxLiveToken = tokens[{"user.auth.xboxlive.com"}];
            if (!xboxLiveToken.hasError()) {
                ticket.token = std::static_pointer_cast<MSACompactToken>(xboxLiveToken.getToken())->getBinaryToken();
                ticket.error_code = 0;
                xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent->set(ticket);
                return ret;
            }
        }
        ticket.error_code = 1;
        ticket.error_text = "Must show UI to acquire an account.";
        xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent->set(ticket);
    } else if (requestCode == 6) { // sign out
        XboxLiveHelper::getMSAStorageManager()->setAccount(std::shared_ptr<MSAAccount>());

        xbox::services::xbox_live_result<void> arg;
        arg.code = 0;
        arg.error_code_category = xbox::services::xbox_services_error_code_category();
        xbox::services::system::user_auth_android::s_signOutCompleteEvent->set(arg);
    }

    return ret;
}
void xboxInvokeAuthFlow(xbox::services::system::user_auth_android* ret) {
    Log::trace("Launcher", "invoke_auth_flow");

#ifdef DISABLE_CEF
    std::cerr << "This build does not support Xbox Live login.\n";
    std::cerr << "To log in please build the launcher with CEF support.\n";
    ret->auth_flow->auth_flow_result.code = 2;
    pplx::task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set(
            &ret->auth_flow->auth_flow_event, ret->auth_flow->auth_flow_result);
#else
    XboxLoginBrowserClient::OpenBrowser(ret);
#endif
}
std::vector<mcpe::string> xblGetLocaleList() {
    std::vector<mcpe::string> ret;
    ret.push_back("en-US");
    return ret;
}
void xblRegisterNatives() {
    Log::trace("Launcher", "register_natives stub");
}
xbox::services::xbox_live_result<void> xblLogCLL(void* th, mcpe::string const& a, mcpe::string const& b, mcpe::string const& c) {
    Log::trace("Launcher", "log_cll %s %s %s", a.c_str(), b.c_str(), c.c_str());
    XboxLiveHelper::getCLL()->addEvent(a.std(), b.std(), c.std());
    xbox::services::xbox_live_result<void> ret;
    ret.code = 0;
    ret.error_code_category = xbox::services::xbox_services_error_code_category();
    ret.message = " ";
    return ret;
}

static int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
    std::cerr << "X error received: "
              << "type " << event->type << ", "
              << "serial " << event->serial << ", "
              << "error_code " << static_cast<int>(event->error_code) << ", "
              << "request_code " << static_cast<int>(event->request_code) << ", "
              << "minor_code " << static_cast<int>(event->minor_code);
    return 0;
}

static int XIOErrorHandlerImpl(Display* display) {
    return 0;
}

extern "C"
void pshufb(char* dest, char* src) {
    char new_dest[16];
    for (int i = 0; i < 16; i++)
        new_dest[i] = (src[i] & 0x80) ? 0 : dest[src[i] & 15];
    memcpy(dest, new_dest, 16);
}
extern "C"
void pshufb_xmm4_xmm0();

using namespace std;
int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "extract") == 0) {
        ExtractHelper::extractApk(argv[2]);
        return 0;
    }


    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

#ifndef DISABLE_CEF
    BrowserApp::RegisterRenderProcessHandler<InitialSetupRenderHandler>();
    BrowserApp::RegisterRenderProcessHandler<XboxLoginRenderHandler>();
#ifndef DISABLE_PLAYAPI
    BrowserApp::RegisterRenderProcessHandler<GoogleLoginRenderHandler>();
#endif
    CefMainArgs cefArgs(argc, argv);
    int exit_code = CefExecuteProcess(cefArgs, BrowserApp::singleton.get(), NULL);
    if (exit_code >= 0)
        return exit_code;

    {
        bool found = true;
        try {
            PathHelper::findDataFile("libs/libminecraftpe.so");
        } catch (std::exception e) {
            found = false;
        }
        if (!found || (argc > 1 && strcmp(argv[1], "setup") == 0)) {
            if (!InitialSetupBrowserClient::OpenBrowser()) {
                BrowserApp::Shutdown();
                return 1;
            }
        }
    }
#endif

    bool enableStackTracePrinting = true;
    bool workaroundAMD = false;

    int windowWidth = 720;
    int windowHeight = 480;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            i++;
            pixelSize = std::stof(argv[i]);
        } else if (strcmp(argv[i], "-sw") == 0 || strcmp(argv[i], "--width") == 0) {
            i++;
            windowWidth = std::stoi(argv[i]);
        } else if (strcmp(argv[i], "-sh") == 0 || strcmp(argv[i], "--height") == 0) {
            i++;
            windowHeight = std::stoi(argv[i]);
        } else if (strcmp(argv[i], "-ns") == 0 || strcmp(argv[i], "--no-stacktrace") == 0) {
            enableStackTracePrinting = false;
        } else if (strcmp(argv[i], "--pocket-guis") == 0) {
            enablePocketGuis = true;
        } else if (strcmp(argv[i], "--amd-fix") == 0) {
            std::cout << "--amd-fix: Enabling AMD Workaround.\n";
            workaroundAMD = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "Help\n";
            std::cout << "--help               Shows this help information\n";
            std::cout << "--scale <scale>      Sets the pixel scale\n";
            std::cout << "--width <width>      Sets the window width\n";
            std::cout << "--height <height>    Sets the window height\n";
            std::cout << "--pocket-guis        Switches to Pocket Edition GUIs\n";
            std::cout << "--no-stacktrace      Disables stack trace printing\n";
            std::cout << "--amd-workaround     Fixes crashes on pre-i686 and AMD CPUs\n\n";
            std::cout << "EGL Options\n";
            std::cout << "-display <display>  Sets the display\n";
            std::cout << "-info               Shows info about the display\n\n";
            std::cout << "MCPE arguments:\n";
            std::cout << "edu <true|false>\n";
            std::cout << "mcworld <world>\n";
            return 0;
        }
    }

    if (enableStackTracePrinting) {
        registerCrashHandler();
    }

    setenv("LC_ALL", "C", 1); // HACK: Force set locale to one recognized by MCPE so that the outdated C++ standard library MCPE uses doesn't fail to find one

    Log::trace("Launcher", "Loading native libraries");
    void* glesLib = loadLibraryOS("libGLESv2.so.2", gles_symbols);
    void* fmodLib = loadLibraryOS(PathHelper::findDataFile("libs/native/libfmod.so.9.6").c_str(), fmod_symbols);
    void* libmLib = loadLibraryOS("libm.so.6", libm_symbols);
    if (glesLib == nullptr || fmodLib == nullptr || libmLib == nullptr)
        return -1;
    Log::trace("Launcher", "Loading hybris libraries");
    stubSymbols(android_symbols, (void*) androidStub);
    stubSymbols(egl_symbols, (void*) eglStub);
    hybris_hook("eglGetProcAddress", (void*) eglGetProcAddress);
    hybris_hook("mcpelauncher_hook", (void*) hookFunction);
    hookAndroidLog();
    if (!load_empty_library("libc.so") || !load_empty_library("libm.so"))
        return -1;
    // load stub libraries
    if (!load_empty_library("libandroid.so") || !load_empty_library("liblog.so") || !load_empty_library("libEGL.so") || !load_empty_library("libGLESv2.so") || !load_empty_library("libOpenSLES.so") || !load_empty_library("libfmod.so") || !load_empty_library("libGLESv1_CM.so"))
        return -1;
    if (!load_empty_library("libmcpelauncher_mod.so"))
        return -1;
    Log::trace("Launcher", "Loading Minecraft library");
    std::string mcpePath = PathHelper::findDataFile("libs/libminecraftpe.so");
    void* handle = hybris_dlopen(mcpePath.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error("Launcher", "Failed to load Minecraft: %s", hybris_dlerror());
        return -1;
    }
    addHookLibrary(handle, mcpePath);

    unsigned int libBase = ((soinfo*) handle)->base;
    Log::info("Launcher", "Loaded Minecraft library");
    Log::debug("Launcher", "Minecraft is at offset 0x%x", libBase);

    std::vector<void*> mods;
    {
        std::string modsDir = PathHelper::getPrimaryDataDirectory() + "mods/";
        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir(modsDir.c_str())) != NULL) {
            Log::info("Launcher", "Loading mods");
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_name[0] == '.')
                    continue;
                std::string fileName(ent->d_name);
                int len = fileName.length();
                if (len < 4 || fileName[len - 3] != '.' || fileName[len - 2] != 's' || fileName[len - 1] != 'o')
                    continue;
                Log::info("Launcher", "Loading mod: %s", fileName);
                void* mod = loadMod(modsDir + fileName);
                if (mod != nullptr)
                    mods.push_back(mod);
            }
            closedir(dir);
            Log::info("Launcher", "Loading %li mods", mods.size());
        }
    }

    Log::info("Launcher", "Applying patches");

    unsigned int patchOff = (unsigned int) hybris_dlsym(handle, "_ZN12AndroidStore21createGooglePlayStoreERKSsR13StoreListener");
    patchCallInstruction((void*) patchOff, (void*) &createStoreHookFunc, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroidC2ER11HTTPRequest");
    patchCallInstruction((void*) patchOff, (void*) &constructLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroid4sendEv");
    patchCallInstruction((void*) patchOff, (void*) &sendLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroid5abortEv");
    patchCallInstruction((void*) patchOff, (void*) &abortLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9crossplat10threadpool16detach_from_javaEPv");
    patchCallInstruction((void*) patchOff, (void*) &detachFromJavaStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9crossplat11get_jvm_envEv");
    patchCallInstruction((void*) patchOff, (void*) &getJVMEnvStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN3web4http6client7details35verify_cert_chain_platform_specificERN5boost4asio3ssl14verify_contextERKSs");
    patchCallInstruction((void*) patchOff, (void*) &verifyCertChainStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services12java_interop16read_config_fileEv");
    patchCallInstruction((void*) patchOff, (void*) &xboxReadConfigFile, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services12java_interop20log_telemetry_signinEbRKSs");
    patchCallInstruction((void*) patchOff, (void*) &xboxLogTelemetrySignin, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services12java_interop22get_local_storage_pathEv");
    patchCallInstruction((void*) patchOff, (void*) &xboxGetLocalStoragePath, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android21init_sign_in_activityEi");
    patchCallInstruction((void*) patchOff, (void*) &xboxInitSignInActivity, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android16invoke_auth_flowEv");
    patchCallInstruction((void*) patchOff, (void*) &xboxInvokeAuthFlow, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services5utils15get_locale_listEv");
    patchCallInstruction((void*) patchOff, (void*) &xblGetLocaleList, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services12java_interop16register_nativesEP15JNINativeMethod");
    patchCallInstruction((void*) patchOff, (void*) &xblRegisterNatives, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services12java_interop7log_cllERKSsS3_S3_");
    patchCallInstruction((void*) patchOff, (void*) &xblLogCLL, true);

    linuxHttpRequestInternalVtable = (void**) ::operator new(8);
    linuxHttpRequestInternalVtable[0] = (void*) &LinuxHttpRequestInternal::destroy;
    linuxHttpRequestInternalVtable[1] = (void*) &LinuxHttpRequestInternal::destroy;

    if (workaroundAMD) {/*
        patchOff = (unsigned int) hybris_dlsym(handle, "_ZN21BlockTessallatorCache5resetER11BlockSourceRK8BlockPos") +
                   (0x40AD97 - 0x40ACD0);
        for (unsigned int i = 0; i < 0x40ADA0 - 0x40AD97; i++)
            ((char *) (void *) patchOff)[i] = 0x90;*/
        patchOff = (unsigned int) hybris_dlsym(handle, "_ZN21BlockTessallatorCache5resetER11BlockSourceRK8BlockPos") + (0x40AD9B - 0x40ACD0);
        patchCallInstruction((void*) patchOff, (void*) &pshufb_xmm4_xmm0, false);
    }

    Log::info("Launcher", "Patches were successfully applied");

    mcpe::string::empty = (mcpe::string*) hybris_dlsym(handle, "_ZN4Util12EMPTY_STRINGE");

    minecraft_symbols_init(handle);

    Log::info("Launcher", "Game version: %s", Common::getGameVersionStringNet().c_str());
    XboxLiveHelper::getCLL()->setAppVersion(Common::getGameVersionStringNet().std());

    AppPlatform::myVtable = (void**) hybris_dlsym(handle, "_ZTV11AppPlatform");
    AppPlatform::_singleton = (AppPlatform**) hybris_dlsym(handle, "_ZN11AppPlatform10mSingletonE");

    Keyboard::states = (int*) hybris_dlsym(handle, "_ZN8Keyboard7_statesE");

    xbox::services::xbox_services_error_code_category = (void* (*)()) hybris_dlsym(handle, "_ZN4xbox8services33xbox_services_error_code_categoryEv");
    xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent = (pplx::task_completion_event_java_rps_ticket*) hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android26s_rpsTicketCompletionEventE");
    xbox::services::system::user_auth_android::s_signOutCompleteEvent = (pplx::task_completion_event_xbox_live_result_void*) hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android22s_signOutCompleteEventE");

    Log::info("Launcher", "Creating window");
    eglutInitWindowSize(windowWidth, windowHeight);
    eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
    eglutInit(argc, argv);

    winId = eglutCreateWindow("Minecraft", PathHelper::getIconPath().c_str());

    Log::info("Launcher", "Starting game initialization");

    void** ptr = (void**) hybris_dlsym(handle, "_ZN9crossplat3JVME");
    *ptr = (void*) 1; // this just needs not to be null

    std::shared_ptr<xbox::services::java_interop> javaInterop = xbox::services::java_interop::get_java_interop_singleton();
    javaInterop->activity = (void*) 1; // this just needs not to be null as well

    Log::trace("Launcher", "Initializing AppPlatform (vtable)");
    LinuxAppPlatform::initVtable(handle);
    Log::trace("Launcher", "Initializing AppPlatform (create instance)");
    platform = new LinuxAppPlatform();
    Log::trace("Launcher", "Initializing AppPlatform (initialize call)");
    platform->initialize();

    Log::trace("Launcher", "Initializing OpenGL bindings");
    mce::Platform::OGL::InitBindings();

    Log::trace("Launcher", "Initializing MinecraftGame (create instance)");
    client = new MinecraftGame(argc, argv);
    Log::trace("Launcher", "Initializing MinecraftGame (init call)");
    AppContext ctx;
    ctx.platform = platform;
    ctx.doRender = true;
    client->init(ctx);
    Log::info("Launcher", "Game initialized");

    if (client->getPrimaryUserOptions()->getFullscreen())
        eglutToggleFullscreen();

    if (!mods.empty())
        Log::info("Launcher", "Initializing mods");
    for (void* mod : mods) {
        void (*initFunc)(MinecraftGame*) = (void (*)(MinecraftGame*)) hybris_dlsym(mod, "mod_set_minecraft");
        if ((void*) initFunc != nullptr)
            initFunc(client);
    }

    eglutIdleFunc(minecraft_idle);
    eglutReshapeFunc(minecraft_reshape);
    eglutDisplayFunc(minecraft_draw);
    eglutMouseFunc(minecraft_mouse);
    eglutMouseButtonFunc(minecraft_mouse_button);
    eglutKeyboardFunc(minecraft_keyboard);
    eglutSpecialFunc(minecraft_keyboard_special);
    eglutPasteFunc(minecraft_paste);
    eglutCloseWindowFunc(minecraft_close);
    Log::trace("Launcher", "Initialized display");

    //(*AppPlatform::_singleton)->_fireAppFocusGained();
    client->setRenderingSize(windowWidth, windowHeight);
    client->setUISizeAndScale(windowWidth, windowHeight, pixelSize);
    eglutMainLoop();

    // this is an ugly hack to workaround the close app crashes MCPE causes
    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9TaskGroupD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);
    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN10WorkerPoolD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);
    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9SchedulerD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);

#ifndef DISABLE_CEF
    BrowserApp::Shutdown();
#endif
    XboxLiveHelper::shutdown();

    return 0;
}
