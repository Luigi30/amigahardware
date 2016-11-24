#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((LONG)(l) >> 16) & 0xFFFF))

void AssumeDirectControl();
void ReleaseSystem();
int WaitForLMB();
