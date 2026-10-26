// Exercise the production readers against user-provided data on the build host.
// No game text or image is copied into the output.
#include "NsaReader.h"
#include "ScriptHandler.h"
#include "sjis2utf16.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

SJIS2UTF16 encoding;
Coding2UTF16 *coding2utf16 = &encoding;

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "Usage: verify_game_data GAME_DIRECTORY\n");
        return 2;
    }
    encoding.init();
    std::string root = std::string(argv[1]) + "/";
    ScriptHandler script;
    script.reset();
    if (script.openScript(&root[0])) return 1;

    // The English patch uses backticks in define-section string arguments.
    char *click = std::strstr(script.getAddress(0), "clickstr ``, 2");
    if (!click) {
        std::fprintf(stderr, "Expected English-patch clickstr command not found\n");
        return 1;
    }
    script.setCurrent(click + std::strlen("clickstr "));
    // The leading backtick is retained internally to mark single-byte text.
    if (std::strcmp(script.readStr(), "`")) {
        std::fprintf(stderr, "Empty backtick string was parsed incorrectly\n");
        return 1;
    }
    if (script.readInt() != 2) {
        std::fprintf(stderr, "Backtick string did not consume its closing delimiter\n");
        return 1;
    }
    std::puts("English backtick string parsing: OK");

    NsaReader archives(0, &root[0], BaseReader::ARCHIVE_TYPE_NSA | BaseReader::ARCHIVE_TYPE_NS2);
    if (archives.open()) return 1;
    struct Probe { const char *name; int archive; const char *magic; size_t magic_size; };
    const Probe probes[] = {
        {"image\\word\\mirrormoon.png", BaseReader::ARCHIVE_TYPE_NSA, "\x89PNG", 4},
        {"image\\title\\title1.png", BaseReader::ARCHIVE_TYPE_NSA, "\x89PNG", 4},
        {"image\\event\\mmpaint.jpg", BaseReader::ARCHIVE_TYPE_NSA, "\xff\xd8", 2},
        {"wave\\se_01.wav", BaseReader::ARCHIVE_TYPE_SAR, "RIFF", 4},
        {"image\\title\\title1.jpg", BaseReader::ARCHIVE_TYPE_SAR, "\xff\xd8", 2},
    };
    for (const Probe &probe : probes) {
        size_t length = archives.getFileLength(probe.name);
        if (!length) {
            std::fprintf(stderr, "Missing archive entry: %s\n", probe.name);
            return 1;
        }
        std::vector<unsigned char> bytes(length);
        int location = 0;
        if (archives.getFile(probe.name, bytes.data(), &location) != length ||
            location != probe.archive || length < probe.magic_size ||
            std::memcmp(bytes.data(), probe.magic, probe.magic_size)) {
            std::fprintf(stderr, "Archive read mismatch: %s\n", probe.name);
            return 1;
        }
        std::printf("Archive read: %s (%zu bytes): OK\n", probe.name, length);
    }
    return 0;
}
