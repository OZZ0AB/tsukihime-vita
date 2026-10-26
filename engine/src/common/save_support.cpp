// SPDX-License-Identifier: GPL-2.0-or-later
#include "save_support.h"
#include <cerrno>
#include <cstring>

namespace tsukihime {

bool validSaveTime(const SaveTime &t)
{
    if (t.year < 1 || t.year > 9999 || t.month < 1 || t.month > 12 ||
        t.hour < 0 || t.hour > 23 || t.minute < 0 || t.minute > 59 ||
        t.second < 0 || t.second > 59) return false;
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int limit = days[t.month - 1];
    if (t.month == 2 && t.year % 4 == 0 && (t.year % 100 != 0 || t.year % 400 == 0)) ++limit;
    return t.day >= 1 && t.day <= limit;
}

SaveTime chooseSaveTime(const SaveTime &file_time, const SaveTime &clock_time)
{
    if (validSaveTime(file_time)) return file_time;
    if (validSaveTime(clock_time)) return clock_time;
    return SaveTime{2000, 1, 1, 0, 0, 0};
}

void formatSaveNumbers(int slot, const SaveTime &time,
                       char (&number)[5], char (&month)[5], char (&day)[5],
                       char (&hour)[5], char (&minute)[5])
{
    const SaveTime t = chooseSaveTime(time, SaveTime{});
    // The port exposes 20 slots. Bound the number for the inherited 5-byte field.
    if (slot < 0) slot = 0;
    if (slot > 9999) slot = 9999;
    std::snprintf(number, sizeof(number), "%02d", slot);
    std::snprintf(month, sizeof(month), "%02d", t.month);
    std::snprintf(day, sizeof(day), "%02d", t.day);
    std::snprintf(hour, sizeof(hour), "%02d", t.hour);
    std::snprintf(minute, sizeof(minute), "%02d", t.minute);
}

int writeSavePayload(FILE *fp, const unsigned char *bytes, size_t length, const char *caption)
{
    if (!fp) { errno = EINVAL; return -1; }
    int error = 0;
    if (std::fwrite(bytes, 1, length, fp) != length) error = errno ? errno : EIO;
    if (!error && caption) {
        const size_t size = std::strlen(caption);
        if (std::fputc('"', fp) == EOF || std::fwrite(caption, 1, size, fp) != size ||
            std::fputc('"', fp) == EOF || std::fputc('*', fp) == EOF)
            error = errno ? errno : EIO;
    }
    if (std::fflush(fp) != 0 && !error) error = errno ? errno : EIO;
    if (std::fclose(fp) != 0 && !error) error = errno ? errno : EIO;
    if (error) { errno = error; return -2; }
    return 0;
}

}
