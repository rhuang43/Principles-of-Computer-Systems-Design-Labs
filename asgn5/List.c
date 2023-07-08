/*********************************************************************************
 * Raymond Huang, rhuang43
 * 2022 Spring CSE101 PA3
 * List.c
 * Source file for List ADT
 *********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "List.h"

// private NodeObj type
// yoinked from queue.c in examples
typedef struct NodeObj {
    void *data;
    struct NodeObj *next;
    struct NodeObj *prev;
    int reference_bit;
} NodeObj;

typedef struct NodeObj *Node; // yoinked from queue.c in examples

typedef struct ListObj {
    int length;
    int index_cursor;
    Node Cursor;
    Node front;
    Node back;
    Node clock_hand;

} ListObj;

Node newNode(void *x) {
    Node N = NULL;
    N = malloc(sizeof(NodeObj));
    N->data = x;
    N->next = NULL;
    N->prev = NULL;
    N->reference_bit = 0; //for clock (is binary)
    return N;
}
// Constructors-Destructors ---------------------------------------------------
List newList(void) { // Creates and returns a new empty List.
    List L;
    L = malloc(sizeof(ListObj));
    L->front = NULL;
    L->back = NULL;
    L->Cursor = NULL;
    L->length = 0;
    L->index_cursor = -1;
    L->clock_hand = NULL;
    return L;
}

void freeNode(Node *pN) {
    if (pN != NULL && *pN != NULL) {
        if ((*pN)->data != NULL) {
            free((*pN)->data);
            (*pN)->data = NULL;
        }
        free(*pN);
        *pN = NULL;
    }
}

void freeList(List *pL)
// Frees all heap memory associated with *pL, and sets
// *pL to NULL.
{
    if (pL == NULL || *pL == NULL) {
        return;
    }

    Node temp = NULL;
    Node currentNode = (*pL)->front;

    while (currentNode != NULL) {
        temp = currentNode;
        currentNode = currentNode->next;

        if (temp->data != NULL) {
            free(temp->data);
            temp->data = NULL;
        }
        freeNode(&temp);
        (*pL)->length--;
    }

    free(*pL);
    *pL = NULL;
}

// Access functions -----------------------------------------------------------
int length(List L) { // Returns the number of elements in L.
    if (L == NULL) // checks if the list is null
    {
        exit(EXIT_FAILURE);
    } else {
        return L->length;
    }
}

int Index(List L) { // Returns index of cursor element if defined, -1 otherwise.
    if (L == NULL) {
        return -1;
    }
    return L->index_cursor;
}

void *front(List L) { // Returns front element of L. Pre: length()>0
    if (L == NULL || L->length <= 0) {
        exit(EXIT_FAILURE);
    } else {
        return L->front->data;
    }
}
void *back(List L) { // Returns back element of L. Pre: length()>0
    if (L->length <= 0 || L == NULL) {
        exit(EXIT_FAILURE);
    }
    return L->back->data;
}
void *get(List L) { // Returns cursor element of L. Pre: length()>0, Index()>=0

    if (length(L) <= 0 || L->index_cursor < 0 || L->Cursor == NULL) {
        exit(EXIT_FAILURE);
    }
    return L->Cursor->data;
}

int getReferenceBit(List L, int index) {
    //Returns the reference bit
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    if (index < 0 || index >= L->length) {
        exit(EXIT_FAILURE);
    }

    Node temp = L->front;
    for (int i = 0; i < index; i++) {
        temp = temp->next;
    }
    return temp->reference_bit;
}

// Manipulation procedures ----------------------------------------------------
void clear(List L)
// Resets L to its original empty state.
{
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    while (L->front != NULL) {
        deleteFront(L); // removes each individual item
    }
    L->front = NULL;
    L->back = NULL;
    L->length = 0;
    L->index_cursor = -1;
    L->Cursor = NULL;
    L->clock_hand = NULL;
}
void moveFront(List L) { // If L is non-empty, sets cursor under the front element,
    // otherwise does nothing.
    if (L->length > 0) {
        L->Cursor = L->front;
        L->index_cursor = 0;
    }
}
void moveBack(List L) { // If L is non-empty, sets cursor under the back element,
    // otherwise does nothing.
    if (L->length > 0) {
        L->Cursor = L->back;
        L->index_cursor = L->length - 1; // so its before the NULL
    }
}

void movePrev(List L) { // If cursor is defined and not at front, move cursor one
    // step toward the front of L; if cursor is defined and at
    // front, cursor becomes undefined; if cursor is undefined
    // do nothing
    if (L->Cursor != NULL && L->index_cursor != 0) {
        L->Cursor = L->Cursor->prev;
        L->index_cursor -= 1;
    } else if (L->Cursor != NULL && L->index_cursor == 0) {
        L->Cursor = NULL;
        L->index_cursor = -1;
    }
}
void moveNext(List L)
// If cursor is defined and not at back, move cursor one
// step toward the back of L; if cursor is defined and at
// back, cursor becomes undefined; if cursor is undefined
// do nothing
{
    if (L->Cursor == L->back) {
        L->Cursor = NULL;
        L->index_cursor = -1;
    } else {
        L->Cursor = L->Cursor->next;
        L->index_cursor += 1;
    }
}
void prepend(List L, void *x)
// Insert new element into L. If L is non-empty,
// insertion takes place before front element.
{
    Node Holder = newNode(x);
    if (L->length == 0) {
        L->front = Holder;
        L->back = Holder;
    } else {
        L->front->prev = Holder;
        Holder->next = L->front;
        L->front = Holder;
        L->index_cursor += 1;
    }
    L->length += 1;
}
void append(List L, void *x)
// Insert new element into L. If L is non-empty,
// insertion takes place after back element.
{
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    if (L->length == 0) {
        Node Holder = newNode(x);
        L->front = Holder;
        L->back = Holder;
        L->length += 1;
    } else if (L->length > 0) {
        Node Holder = newNode(x);
        L->back->next = Holder;
        Holder->prev = L->back;
        L->back = Holder;
        L->length += 1;
    }
}
void insertBefore(List L, void *x)
// Insert new element before cursor.
// Pre: length()>0, Index()>=0
{
    if (L->length > 0 && L->index_cursor >= 0) {
        Node Holder = newNode(x);
        setReferenceBit(L, Index(L), 0);
        Holder->next = L->Cursor;
        if (L->Cursor == L->front) {
            Holder->prev = L->Cursor;
            L->Cursor->prev = Holder;
            L->front = Holder;
        } else {
            Holder->prev = L->Cursor->prev;
            L->Cursor->prev->next = Holder;
        }
        L->index_cursor += 1;
        L->length += 1;
    } else {
        exit(EXIT_FAILURE);
    }
}

void insertAfter(List L, void *x)
// Insert new element after cursor.
// Pre: length()>0, Index()>=0
{
    Node Holder = newNode(x);
    if (L->length > 0 && Index(L) >= 0 && L != NULL) {
        if (L->Cursor == L->back) // checks if it is the last element
        {
            Holder->prev = L->back;
            L->back->next = Holder;
            L->back = Holder;
            L->length += 1;
        } else {
            Holder->next = L->Cursor->next;
            Holder->prev = L->Cursor;
            L->Cursor->next->prev = Holder;
            L->Cursor->next = Holder;
            L->length += 1;
        }
    } else {
        exit(EXIT_FAILURE);
    }
}

void deleteFront(List L)
// Delete the front element. Pre: length()>0
{
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    if (L->length > 0) {
        Node Holder = L->front;
        if (L->length == 1) // checks if its just one item
        {
            freeNode(&Holder);
            L->front = NULL;
            L->back = NULL;
            L->index_cursor = -1;
        } else // checks if its not the first item
        {
            L->front = L->front->next;
            L->front->prev = NULL;
            if (L->Cursor != NULL) {
                L->index_cursor -= 1;
            }
            freeNode(&Holder);
        }
    }
    L->length -= 1;
}
void deleteBack(List L)
// Delete the back element. Pre: length()>0
{
    if (L == NULL || L->length <= 0) {
        exit(EXIT_FAILURE);
    }
    Node Bak = L->back;
    if (L->length == 1) // checks if its the last item
    {
        L->back = NULL;
        L->front = NULL;
        L->Cursor = NULL;
        L->index_cursor = -1;
    } else {
        L->back = L->back->prev;
        L->back->next = NULL;
        if (L->index_cursor == L->length - 1) {
            L->index_cursor = -1;
        }
    }

    freeNode(&Bak);
    L->length--;
}
void delete (List L)
// Delete cursor element, making cursor undefined.
// Pre: length()>0, Index()>=0
{
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    if (L->length <= 0) {
        exit(EXIT_FAILURE);
    }
    if (L->index_cursor < 0) {
        exit(EXIT_FAILURE);
    } else if (L->Cursor == L->front) {
        deleteFront(L);
    } else if (L->Cursor == L->back) {
        deleteBack(L);
    } else {
        Node Holder = L->Cursor;
        L->Cursor->prev->next = L->Cursor->next;
        L->Cursor->next->prev = L->Cursor->prev;
        freeNode(&Holder);
        L->length--;
    }
}

void setReferenceBit(List L, int index, int value) {
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }
    if (index < 0 || index >= L->length) {
        exit(EXIT_FAILURE);
    }
    if (value != 0 && value != 1) {
        exit(EXIT_FAILURE);
    }

    Node temp = L->front;
    for (int i = 0; i < index; i++) {
        temp = temp->next;
    }
    temp->reference_bit = value;
}

// Other operations -----------------------------------------------------------
List copyList(List L)
// Returns a new List representing the same integer
// sequence as L. The cursor in the new list is undefined,
// regardless of the state of the cursor in L. The state
// of L is unchanged.
{
    List new = newList();
    Node Holder = L->front;
    while (Holder != NULL) {
        append(new, Holder->data);
        Holder = Holder->next;
    }
    return new;
}

//clock_hand implementation:
void makeClockHand(List L) {
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }

    L->clock_hand = L->front;
}
void moveNextClockHand(List L) {
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }

    if (L->clock_hand == NULL) { //you never know
        makeClockHand(L);
    } else {
        L->clock_hand = L->clock_hand->next;
        if (L->clock_hand == NULL) {
            L->clock_hand = L->front;
        }
    }
}
void moveCursorToClockHand(List L) {
    if (L == NULL) {
        exit(EXIT_FAILURE);
    }

    if (L->clock_hand == NULL) {
        makeClockHand(L);
    }

    L->Cursor = L->clock_hand;

    // moves the index_cursor
    int index = 0;
    Node temp = L->front;
    while (temp != NULL) {
        if (temp == L->clock_hand) {
            L->index_cursor = index;
            break;
        }
        index++;
        temp = temp->next;
    }
}

void *clockDelete(List L) {
    //Deletes the element and returns the deleted element
    if (L == NULL || L->clock_hand == NULL) {
        exit(EXIT_FAILURE);
    }
    while (L->clock_hand->reference_bit != 0) {
        L->clock_hand->reference_bit = 0;
        moveNextClockHand(L);
    }
    void *deleted = L->clock_hand->data;
    moveCursorToClockHand(L);
    moveNextClockHand(L);
    return deleted;
}

void insertAfterClockHand(List L, void *x) {
    // Insert new element after clock_hand.
    if (L == NULL || L->clock_hand == NULL) {
        exit(EXIT_FAILURE);
    }

    Node Holder = newNode(x);
    if (L->clock_hand == L->back) {
        Holder->prev = L->back;
        L->back->next = Holder;
        L->back = Holder;
    } else {
        Holder->next = L->clock_hand->next;
        Holder->prev = L->clock_hand;
        L->clock_hand->next->prev = Holder;
        L->clock_hand->next = Holder;
    }
    L->length += 1;
}
