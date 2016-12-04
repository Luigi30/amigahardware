#include "amiga.h"

struct Custom *Hardware = (struct Custom *)0xdff000;
struct CIA *PortA = (struct CIA *)0xbfe001;
struct CIA *PortB = (struct CIA *)0xbfe000;
UWORD SystemInts;
UWORD SystemDMA;
UBYTE *ICRA = (UBYTE *)0xbfed01;