#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "save_support.h"
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace tsukihime;

static void ascii(const char *s)
{
    for (; *s; ++s) assert((unsigned char)*s >= 0x20 && (unsigned char)*s <= 0x7e);
}

struct FailingOutput { bool closed; bool fail_write; bool fail_close; };

static ssize_t failingWrite(void *cookie, const char *, size_t size)
{
    FailingOutput *out = static_cast<FailingOutput *>(cookie);
    if (out->fail_write) { errno = ENOSPC; return -1; }
    return size;
}

static int failingClose(void *cookie)
{
    FailingOutput *out = static_cast<FailingOutput *>(cookie);
    out->closed = true;
    if (out->fail_close) { errno = EIO; return -1; }
    return 0;
}

int main()
{
    const SaveTime file = {2026, 10, 26, 23, 59, 12};
    const SaveTime clock = {2024, 2, 29, 8, 7, 0};
    const SaveTime invalid = {};
    assert(validSaveTime(file));
    assert(validSaveTime(clock));
    assert(!validSaveTime(SaveTime{2025, 2, 29, 0, 0, 0}));
    assert(!validSaveTime(SaveTime{1900, 2, 29, 0, 0, 0}));
    assert(!validSaveTime(SaveTime{2026, 1, 1, 24, 0, 0}));
    assert(chooseSaveTime(file, clock).month == 10);
    assert(chooseSaveTime(invalid, clock).year == 2024);
    const SaveTime fake = chooseSaveTime(invalid, invalid);
    assert(fake.year == 2000 && fake.month == 1 && fake.day == 1 &&
           fake.hour == 0 && fake.minute == 0);

    char number[5], month[5], day[5], hour[5], minute[5], row[512], confirm[256];
    formatSaveNumbers(20, file, number, month, day, hour, minute);
    std::snprintf(row, sizeof(row), SAVE_EXIST_FORMAT, "Memory", number, month, day, hour, minute);
    assert(!std::strcmp(row, "`Memory 20  10/26  23:59"));
    std::snprintf(confirm, sizeof(confirm), SAVE_CONFIRM_FORMAT, "Memory", number);
    assert(!std::strcmp(confirm, "`Save to Memory 20?"));
    ascii(row); ascii(confirm); ascii(YES); ascii(NO); ascii(OK); ascii(CANCEL);
    formatSaveNumbers(1, invalid, number, month, day, hour, minute);
    assert(!std::strcmp(month, "01") && !std::strcmp(day, "01") &&
           !std::strcmp(hour, "00") && !std::strcmp(minute, "00"));
    std::puts("Save labels/dates: ASCII; real, invalid, and missing-clock cases passed");

    char directory[] = "/tmp/tsukihime-save-check.XXXXXX";
    assert(mkdtemp(directory));
    char path[256];
    std::snprintf(path, sizeof(path), "%s/save1.dat", directory);
    const unsigned char payload[] = {'O','N','S',2,8,0,1,0xff,0};
    FILE *fp = std::fopen(path, "wb");
    assert(fp);
    assert(writeSavePayload(fp, payload, sizeof(payload), "checkpoint") == 0);
    fp = std::fopen(path, "rb");
    assert(fp);
    unsigned char readback[128] = {};
    size_t count = std::fread(readback, 1, sizeof(readback), fp);
    assert(std::fclose(fp) == 0);
    const char trailer[] = "\"checkpoint\"*";
    assert(count == sizeof(payload) + std::strlen(trailer));
    assert(!std::memcmp(readback, payload, sizeof(payload)));
    assert(!std::memcmp(readback + sizeof(payload), trailer, std::strlen(trailer)));
    assert(unlink(path) == 0);
    assert(rmdir(directory) == 0);
    std::puts("Save payload/caption: written, closed, reopened, and matched byte-for-byte");

    cookie_io_functions_t io = {};
    io.write = failingWrite;
    io.close = failingClose;
    FailingOutput full = {false, true, false};
    fp = fopencookie(&full, "w", io);
    assert(fp);
    errno = 0;
    assert(writeSavePayload(fp, payload, sizeof(payload), NULL) == -2);
    assert(full.closed && errno == ENOSPC);
    FailingOutput close_error = {false, false, true};
    fp = fopencookie(&close_error, "w", io);
    assert(fp);
    errno = 0;
    assert(writeSavePayload(fp, payload, sizeof(payload), NULL) == -2);
    assert(close_error.closed && errno == EIO);
    std::puts("Full storage and close failures are reported, not treated as successful saves");
}
