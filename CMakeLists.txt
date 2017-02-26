cmake_minimum_required(VERSION 2.8)
project(mcpelauncher)

enable_language(C ASM)

#SET(CMAKE_CXX_COMPILER "g++-4.9")
#SET(CMAKE_C_COMPILER "gcc-4.9")

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(${PNG_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0")

set(HYBRIS_SOURCES hybris/src/cache.c hybris/src/dlfcn.c hybris/src/hooks.c hybris/src/hooks_shm.c hybris/src/logging.c hybris/src/properties.c hybris/src/strlcpy.c hybris/src/sysconf.c hybris/src/jb/dlfcn.c hybris/src/jb/linker.c hybris/src/jb/linker_environ.c hybris/src/jb/linker_format.c hybris/src/jb/rt.c)

add_definitions(-DSERVER)
set(SOURCE_FILES src/dedi.cpp src/common.cpp src/hook.cpp src/LinuxAppPlatform.cpp mcpe/types.cpp mcpe/ImagePickingCallback.h mcpe/FilePickerSettings.h)
add_executable(mcpelauncher ${HYBRIS_SOURCES} ${SOURCE_FILES})
target_link_libraries(mcpelauncher ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} rt uuid)
target_compile_definitions(mcpelauncher PRIVATE ANDROID_X86_LINKER _GNU_SOURCE LINKER_TEXT_BASE=0xB0000100 LINKER_AREA_SIZE=0x01000000 LINKER_DEBUG=1)
set_target_properties(mcpelauncher PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
