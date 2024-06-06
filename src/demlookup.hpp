#pragma once

#include <FS.h>

int32_t dem_setup(fs::FS& fs, const String &dirName);
void printDems(void);
void publishDems(void);
