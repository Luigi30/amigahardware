#ifndef IFF_H
#define IFF_H

#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <libraries/iffparse.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>

extern struct Library *IFFParseBase;
extern struct Library *DOSBase;

int IFF_LoadImageFromIFF(PLANEPTR *, char *);
extern void UnpackBits(__reg("a0") APTR src, __reg("a1") APTR dest, __reg("d2") sourceLength, __reg("d3") resultLength);

#define ID_ILBM         MAKE_ID('I','L','B','M')
#define ID_BMHD         MAKE_ID('B','M','H','D')
#define ID_CMAP         MAKE_ID('C','M','A','P')
#define ID_CAMG         MAKE_ID('C','A','M','G')
#define ID_BODY         MAKE_ID('B','O','D','Y')

typedef UBYTE Masking;  /* Choice of masking technique - Usually 0. */
#define mskNone                 0
#define mskHasMask              1
#define mskHasTransparentColor  2
#define mskLasso                3

/* Compression algorithm applied to the rows of all source
 * and mask planes. "cmpByteRun1" is byte run encoding.
 * Do not compress across rows!  Compression is usually 1.
 */
typedef UBYTE Compression;
#define cmpNone                 0
#define cmpByteRun1             1

/* The BitMapHeader structure expressed as a C structure */
typedef struct {
    UWORD w, h;                 /* raster width & height in pixels      */
    WORD  x, y;                 /* pixel position for this image        */
    UBYTE nPlanes;              /* # bitplanes (without mask, if any)   */
    Masking     masking;        /* One of the values above.  Usually 0  */
    Compression compression;    /* One of the values above.  Usually 1  */
    UBYTE reserved1;            /* reserved; ignore on read, write as 0 */
    UWORD transparentColor;     /* transparent color number. Usually 0  */
    UBYTE xAspect, yAspect;     /* pixel aspect, a ratio width : height */
    WORD  pageWidth, pageHeight;    /* source "page" size in pixels */
} BitMapHeader;

#endif
