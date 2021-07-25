#include "list.h"

// Define the struct of the handle here
typedef struct {
    void ** array;
    size_t count;
} LIST;

#define LIST(h) if (!h) { return 0; } LIST* list = (LIST*)h;
#define LISTN(h) if (!h) { return; } LIST* list = (LIST*)h;

// Creates a handle of a list
// Returns the pointer of the handle on success
// Returns NULL otherwise
void * listCreate() {
    LIST * list = (LIST*)calloc(1, sizeof(LIST));
    if (list == NULL) {
        return NULL;
    }

    list->array = NULL; // start with an empty array
    list->count = 0;

    return list;
}

// Gets an item in the list using the given index
// Returns the pointer of the item in the list.
// Returns NULL if item is not found
void * listGet(void * hList, size_t index) {
    LIST(hList)

    if (index < list->count) {
        return list->array[index];
    } else {
        return NULL;
    }
}

// Gets the number of items stored in the list
// Returs the number of items in the list
size_t listCount(void * hList) {
    LIST(hList)

    return list->count;
}

// Inserts a new item into the list
// Returns the total number of items in the list after insertion
size_t listInsert(void * hList, size_t index, void * pNewItem) {
    LIST(hList)

    size_t prevCount = list->count;

    if (index >= list->count) { // extend the list to accomodate out of range insertion
        list->count = index + 1;

        list->array = (void**)realloc(list->array, sizeof(void *) * (list->count));

        if (!list->array) {
            list->count = 0;
            return 0; // failed to reallocate
        } 
    } else {
        list->count++;   

        list->array = (void**)realloc(list->array, sizeof(void *) * (list->count)); 

        // loop from the end
        // shift items
        for (long int j = (long int)prevCount - 1; j >= (long int)index; j--) {
            list->array[j + 1] = list->array[j];
        }
    }

    list->array[index] = pNewItem;

    return list->count;
}

// Sets an item in the list using the given index
// Returns nothing
void listSet(void * hList, size_t index, void * pItem) {
    LISTN(hList)

    if (index < list->count) {
        list->array[index] = pItem;
    }
}

// Inserts a new item at the end of the list
// Returns the total number of items in the list after insertion
size_t listPush(void * hList, void * pNewItem) {
    LIST(hList)

    list->array = (void**)realloc(list->array, sizeof(void *) * (list->count + 1));

    if (list->array == NULL) {
        return list->count;
    }

    list->array[list->count] = pNewItem;

    return ++list->count;
}

// Delete the list
// Does not destroys the item pointers of the list
void listDelete(void * hList) {
    LISTN(hList)

    free(list->array);
    free(list);
}

// Removes an item in the list based on the index provided
// Returns the pointer of the removed item
// Returns NULL if an error occured
void * listRemove(void * hList, size_t index) {
    LIST(hList)

    if (index >= list->count) { // array is empty
        return NULL;
    }

    void * removedItem = list->array[index];

    // transfer items first
    for (size_t j = index + 1; j < list->count; j++) {
        list->array[j - 1] = list->array[j];
    }

    list->count--;

    if (list->count > 0) {
        list->array = (void**)realloc(list->array, sizeof(void*) * (list->count));
        if (!list->array) {
            return NULL;
        }
    } else {
        free(list->array);
        list->array = NULL;
    }

    return removedItem;
}

// Finds an item in the list
// Outputs the index using a pointer (if pointer is NULL, index will not be passed)
// Returns 1 if found
// Returns 0 if not found
int listSearch(void * hList, void * item, size_t* pIndex) {
    LIST(hList)

    size_t i = 0;
    for (; i < list->count; i++) {
        if (list->array[i] == item) {
            break;
        }
    }
                
    if (i == list->count) { // not in the list
        return 0;
    } else {
        if (pIndex) {
            *pIndex = i;
        }
        return 1;
    }  
}