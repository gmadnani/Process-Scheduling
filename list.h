#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>

// This file is used to define the generic list object for pointers
// Each element of this list is a pointer to a object.

// Creates a handle of a list
// Returns the pointer of the handle on success
// Returns NULL otherwise
void * listCreate();

// Gets an item in the list using the given index
// Returns the pointer of the item in the list.
// Returns NULL if item is not found
void * listGet(void * hList, size_t index);

// Gets the number of items stored in the list
// Returs the number of items in the list
size_t listCount(void * hList);

// Inserts a new item into the list
// Returns the total number of items in the list after insertion
size_t listInsert(void * hList, size_t index, void * pNewItem);

// Sets an item in the list using the given index
// Returns nothing
void listSet(void * hList, size_t index, void * pItem);

// Inserts a new item at the end of the list
// Returns the total number of items in the list after insertion
size_t listPush(void * hList, void * pNewItem);

// Delete the list
// Does not destroys the item pointers of the list
void listDelete(void * hList);

// Removes an item in the list based on the index provided
// Returns the pointer of the removed item
// Returns NULL if an error occured
void * listRemove(void * hList, size_t index);

// Finds an item in the list
// Outputs the index using a pointer (if pointer is NULL, index will not be passed)
// Returns 1 if found
// Returns 0 if not found
int listSearch(void * hList, void * item, size_t* pIndex);

#endif