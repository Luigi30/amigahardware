#ifndef LOADER_H
#define LOADER_H

#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>

#include "iff.h"

/* node for linked list of graphics */
struct ImageData {
	char name[8];
	PLANEPTR bitmap;
	UWORD bytes;
	struct ImageData *next;
};

void G_InitGraphics(struct ImageData **);
struct ImageData *G_FindGraphic(struct ImageData *, char *seekName);
void G_FreeAllGraphics(struct ImageData *list);

#endif
