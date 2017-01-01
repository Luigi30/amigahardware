#include "image/iff.h"

/* Returns the image size in bytes. Images should be 4bpp interleaved IFF. */
int IFF_LoadImageFromIFF(PLANEPTR *buffer, char *filename){
	
	if(IFFParseBase == NULL){
		IFFParseBase = OpenLibrary("iffparse.library", 0);
		if(IFFParseBase == NULL){
			return 0;
		}
	}
	
	struct IFFHandle *iff = AllocIFF();
	if(!(iff->iff_Stream = Open(filename, MODE_OLDFILE))){
		printf("Failed to open %s\r\n", filename);
		return 0;
	}
	
	InitIFFasDOS(iff);
	
	if(OpenIFF(iff, IFFF_READ) != 0){
		printf("OpenIFF failed.");
		return 0;
	}
	
	struct StoredProperty *sp;
	BitMapHeader *bmhd;
	
	//Define the chunks we want to look for
	PropChunk(iff, ID_ILBM, ID_BMHD);
	StopChunk(iff, ID_ILBM, ID_BODY);
	ParseIFF(iff, IFFPARSE_SCAN);
	
	if(sp = FindProp(iff, ID_ILBM, ID_BMHD)){
		UBYTE *data = sp->sp_Data;
		bmhd = (BitMapHeader *)sp->sp_Data;
	} else {
		printf("No BMHD chunk found.\r\n");
	}
	
	printf("%dx%d bitmap, %d bitplanes. Compression type %d.\r\n", 
		bmhd->w, 
		bmhd->h, 
		bmhd->nPlanes,
		bmhd->compression);
	int uncompressedSizeBytes = ((bmhd->w*bmhd->h)/8) * bmhd->nPlanes;
	printf("Uncompressed size should be %d bytes.\r\n", uncompressedSizeBytes);
	
	//Decode the body chunk.
	int bytesRead = 0;
	int compressedSize = 0;
	UWORD resultLength = 0;
	
	//Allocate a temporary buffer in fast RAM if available and then copy to chip mem.
	if(bmhd->compression == 1){
		printf("Compressed.\r\n");
		APTR temp = AllocMem(uncompressedSizeBytes, MEMF_PUBLIC|MEMF_CLEAR);
		int compressedSize = ReadChunkBytes(iff, temp, uncompressedSizeBytes);
		if(compressedSize > 0){
			*buffer = AllocMem(uncompressedSizeBytes, MEMF_CHIP|MEMF_CLEAR);			
			UnpackBits(temp, *buffer, compressedSize, resultLength);
			printf("Uncompressed %d bytes to %d bytes.\r\n", compressedSize, resultLength);
		} else {
			printf("Couldn't read body chunk!\r\n");
		}
		FreeMem(temp, uncompressedSizeBytes);		
	} else {
		//Not compressed.
		printf("Not compressed.\r\n");
		*buffer = AllocMem(uncompressedSizeBytes, MEMF_CHIP|MEMF_CLEAR);
		int read = ReadChunkBytes(iff, *buffer, uncompressedSizeBytes);
		if(read != uncompressedSizeBytes){
			printf("Error reading body chunk: Expected %d bytes, read %d bytes\r\n", uncompressedSizeBytes, read);
		}
	}
	
	printf("IFF_LoadImageFromIFF(): success loading %s\r\n", filename);
	
	return uncompressedSizeBytes;
}