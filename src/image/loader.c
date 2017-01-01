#include "image/loader.h"

struct ImageData *G_FindTailImage(struct ImageData *list){
	struct ImageData *current = list;
	while(current->next != NULL){
		current = current->next;
	}
	
	return current; //ptr to the last node in the list
}

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

void G_AddGraphic(struct ImageData *list, char *filename, char *imagename){
	//add a new node to the end of the linked list.
	struct ImageData *tail = G_FindTailImage(list);
	
	struct ImageData *newNode = AllocMem(sizeof(struct ImageData), MEMF_PUBLIC|MEMF_CLEAR);
	strcpy(newNode->name, imagename);
	newNode->bytes = IFF_LoadImageFromIFF(&newNode->bitmap, filename);
	tail->next = newNode;
}

void G_FreeAllGraphics(struct ImageData *list){
	struct ImageData *current = list;
	while(current != NULL){
		struct ImageData *nextNode = current->next;
		FreeMem(current->bitmap, current->bytes);
		current = nextNode;
	}
}