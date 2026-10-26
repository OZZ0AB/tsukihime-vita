// Tsukihime Vita startup diagnostics, added 2026-10-26.
// Distributed under GPL-2.0-or-later, like the surrounding engine.
#include "vita_startup.h"
#include <SDL2/SDL.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "debugScreen.h"

// Use VitaSDK's supported allocator with a bounded heap, leaving memory for
// SDL's GPU buffers and threads. The old custom allocator reserved all free RAM.
extern "C" {
unsigned int _newlib_heap_size_user = 128 * 1024 * 1024;
}

static const char *log_path = "ux0:data/tsukihime/startup.log";
static int log_fd = -1;
static bool screen_active = false;
static char last_error[2048];
static char log_notice[256];

void vitaStartupLog(const char *message)
{
    if (log_fd < 0 || !message) return;
    size_t remaining = std::strlen(message);
    while (remaining) {
        int written = sceIoWrite(log_fd, message, remaining);
        if (written <= 0) break;
        message += written;
        remaining -= written;
    }
}

void vitaStartupRecordError(const char *message)
{
    std::snprintf(last_error, sizeof(last_error), "%s", message);
    vitaStartupLog(message);
}

static void sdlLog(void *, int category, SDL_LogPriority priority, const char *message)
{
    char line[2304];
    std::snprintf(line, sizeof(line), "SDL[%d/%d] %s\n", category, priority, message);
    vitaStartupLog(line);
}

static void onExit()
{
    vitaStartupLog("Process exiting.\n");
    if (last_error[0]) {
        vitaStartupLog("Last engine error: ");
        vitaStartupLog(last_error);
    }
    if (log_fd >= 0) sceIoSync("ux0:", 0);
}

static int openLog(const char *path)
{
    // Use separate truncate/append opens. Diagnostics also append through stdio,
    // so every writer must append rather than maintain a separate file offset.
    int fd = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) return fd;
    sceIoClose(fd);
    return sceIoOpen(path, SCE_O_WRONLY | SCE_O_APPEND, 0777);
}

void vitaStartupInit()
{
    int data_result = sceIoMkdir("ux0:data", 0777);
    int root_result = sceIoMkdir("ux0:data/tsukihime", 0777);
    log_fd = openLog(log_path);
    int primary_error = log_fd;
    if (log_fd < 0) {
        log_path = "ux0:data/tsukihime-startup.log";
        log_fd = openLog(log_path);
    }
    if (log_fd >= 0) {
        std::snprintf(log_notice, sizeof(log_notice), "Log: %s", log_path);
    } else {
        std::snprintf(log_notice, sizeof(log_notice),
                      "Logging unavailable; continuing.\nPrimary error: 0x%08X; fallback: 0x%08X",
                      (unsigned int)primary_error, (unsigned int)log_fd);
    }
    // Capture direct printf/fprintf calls as well as the engine/SDL log paths.
    if (log_fd >= 0) {
        if (std::freopen(log_path, "a", stdout)) std::setvbuf(stdout, NULL, _IONBF, 0);
        if (std::freopen(log_path, "a", stderr)) std::setvbuf(stderr, NULL, _IONBF, 0);
    }
    std::atexit(onExit);
    SDL_LogSetOutputFunction(sdlLog, NULL);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    screen_active = psvDebugScreenInit() >= 0;
    if (screen_active) psvDebugScreenPrintf("Tsukihime Vita 01.04\n\nStarting native runtime...\n\n");
    vitaStartupStage("Tsukihime Vita 01.04: main reached (128 MiB SDK heap)");
    vitaStartupStage(log_notice);
    char message[160];
    std::snprintf(message, sizeof(message), "mkdir results: ux0:data=0x%08X, tsukihime=0x%08X",
                  (unsigned int)data_result, (unsigned int)root_result);
    vitaStartupStage(message);
}

void vitaStartupStage(const char *message)
{
    vitaStartupLog(message);
    vitaStartupLog("\n");
    if (log_fd >= 0) sceIoSync("ux0:", 0);
    if (screen_active) psvDebugScreenPrintf("%s\n", message);
}

void vitaStartupFinishScreen()
{
    if (screen_active) psvDebugScreenFinish();
    screen_active = false;
}

void vitaStartupRequireGameData(const char *directory)
{
    const char *required[] = {"nscript.dat", "arc.sar", "arc.nsa", "arc1.nsa", "default.ttf"};
    for (const char *name : required) {
        char path[256];
        std::snprintf(path, sizeof(path), "%s/%s", directory, name);
        SceIoStat stat = {};
        int result = sceIoGetstat(path, &stat);
        char message[384];
        if (result < 0 || !SCE_S_ISREG(stat.st_mode) || stat.st_size <= 0) {
            std::snprintf(message, sizeof(message),
                          "Missing or unreadable game file:\n%s\nstat=0x%08X, mode=0x%08X, bytes=%llu\n\nCopy the contents of Game directly into this directory.",
                          path, (unsigned int)result, (unsigned int)stat.st_mode,
                          (unsigned long long)stat.st_size);
            vitaStartupFatal(message);
        }
        SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
        unsigned char header[8];
        if (fd < 0) {
            std::snprintf(message, sizeof(message), "Cannot open game file:\n%s\nopen=0x%08X", path, (unsigned int)fd);
            vitaStartupFatal(message);
        }
        int read = sceIoRead(fd, header, sizeof(header));
        sceIoClose(fd);
        if (read != sizeof(header)) {
            std::snprintf(message, sizeof(message), "Game file read failed:\n%s\nread=0x%08X", path, (unsigned int)read);
            vitaStartupFatal(message);
        }
        std::snprintf(message, sizeof(message), "File OK: %s (%llu bytes)", name, (unsigned long long)stat.st_size);
        vitaStartupStage(message);
    }
    int result = sceIoMkdir("ux0:data/tsukihime/save", 0777);
    SceIoStat stat = {};
    int stat_result = sceIoGetstat("ux0:data/tsukihime/save", &stat);
    if (stat_result < 0 || !SCE_S_ISDIR(stat.st_mode)) {
        char message[256];
        std::snprintf(message, sizeof(message),
                      "Cannot access save directory:\nux0:data/tsukihime/save\nmkdir=0x%08X, stat=0x%08X",
                      (unsigned int)result, (unsigned int)stat_result);
        vitaStartupFatal(message);
    }
}

void vitaStartupFatal(const char *message)
{
    // SDL_Quit can invalidate an SDL error buffer passed by the caller.
    char error_copy[2048];
    std::snprintf(error_copy, sizeof(error_copy), "%s", message);
    vitaStartupRecordError(error_copy);
    vitaStartupStage("\nFATAL: startup/runtime stopped.");
    // SDL may have initialized GXM already. Release its display resources before
    // showing the SDK framebuffer, which needs neither a game font nor SDL.
    if (!screen_active) {
        SDL_Quit();
        screen_active = psvDebugScreenInit() >= 0;
    }
    if (screen_active) {
        psvDebugScreenPrintf("\nERROR\n\n%s\n\n%s\n\nPress Cross to close.\n", error_copy, log_notice);
    }
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);
    SceCtrlData pad = {};
    bool released = false;
    while (true) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (!(pad.buttons & SCE_CTRL_CROSS)) released = true;
        if (released && (pad.buttons & SCE_CTRL_CROSS)) break;
        sceKernelDelayThread(20000);
    }
    if (log_fd >= 0) sceIoSync("ux0:", 0);
    sceKernelExitProcess(1);
    for (;;) sceKernelDelayThread(1000000);
}
