#include "loader.h"

void G_InitGraphics(struct ImageData **list){
	//first node of the linked list will always be the background tilemap
	*list = AllocMem(sizeof(struct ImageData), MEMF_PUBLIC|MEMF_CLEAR);
	struct ImageData *head = *list;
	strcpy(head->name, "BGTILES");
	head->bytes = IFF_LoadImageFromIFF(&head->bitmap, "gfx/tiles.iff");
}

struct ImageData *G_FindGraphic(struct ImageData *list, char *seekName){
	//Return a pointer to the node with a matching name
	struct ImageData *current = list;
	while(current != NULL){
		if(strcmp(current->name, seekName) == 0){
			return current;
		} else {
			current = current->next;
		}
	}
}

void G_FreeAllGraphics(struct ImageData *list){
	struct ImageData *current = list;
	while(current != NULL){
		struct ImageData *nextNode = current->next;
		FreeMem(current->bitmap, current->bytes);
		current = nextNode;
	}
}