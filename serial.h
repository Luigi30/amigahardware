#ifndef SERIAL_H
#define SERIAL_H

#include <string.h>
#include "amiga.h"

void S_SendByte(UBYTE byte);
void S_SendString(char *msg);

#endif
