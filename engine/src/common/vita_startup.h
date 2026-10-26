#ifndef TSUKIHIME_VITA_STARTUP_H
#define TSUKIHIME_VITA_STARTUP_H

void vitaStartupInit();
void vitaStartupLog(const char *message);
void vitaStartupRecordError(const char *message);
void vitaStartupStage(const char *message);
void vitaStartupFinishScreen();
void vitaStartupRequireGameData(const char *directory);
void vitaStartupFatal(const char *message) __attribute__((noreturn));

#endif
