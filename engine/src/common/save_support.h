// Tsukihime Vita save metadata and checked writes.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef TSUKIHIME_SAVE_SUPPORT_H
#define TSUKIHIME_SAVE_SUPPORT_H

#include <cstdio>
#include <cstddef>

namespace tsukihime {

struct SaveTime {
    int year, month, day, hour, minute, second;
};

bool validSaveTime(const SaveTime &time);
// Timestamp availability must never decide whether a slot can be loaded.
SaveTime chooseSaveTime(const SaveTime &file_time, const SaveTime &clock_time);
void formatSaveNumbers(int slot, const SaveTime &time,
                       char (&number)[5], char (&month)[5], char (&day)[5],
                       char (&hour)[5], char (&minute)[5]);

static const char SAVE_EXIST_FORMAT[] = "`%s %s  %s/%s  %s:%s";
static const char SAVE_EMPTY_FORMAT[] = "`%s %s  Empty";
static const char SAVE_CONFIRM_FORMAT[] = "`Save to %s %s?";
static const char LOAD_CONFIRM_FORMAT[] = "`Load %s %s?";
static const char YES[] = "`Yes";
static const char NO[] = "`No";
static const char OK[] = "`OK";
static const char CANCEL[] = "`Cancel";

// Owns/closes fp. Returns 0 only if payload, optional caption, flush, and close
// succeed. The caller captures errno immediately for device diagnostics.
int writeSavePayload(FILE *fp, const unsigned char *bytes, size_t length,
                     const char *caption);

}
#endif
