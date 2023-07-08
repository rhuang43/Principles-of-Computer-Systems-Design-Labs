#include "List.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define FULL     10
#define NOT_FULL 0
#define OVERFLOW 2

//types of miss global counter
int capacity = 0; //if its a miss and the element was seen
int compulsory = 0; //if its a miss and the elemt has never been seen before

//checks if the size of the cache is the desired size
int cache_size_checker(List L, int size) {
    if (length(L) == size) {
        return FULL; //full
    } else if (length(L) > size) {
        printf("size: %d", size);
        printf("length: %d", length(L));
        printf("CACHE SIZE INCREASED???\n");
        return OVERFLOW; //ERROR CACHE SIZE INCREASED
    }
    return NOT_FULL; // everything is a o k
}

//checks if the input is in the cache
bool list_check(List L, char *input) {
    if (length(L) == 0) {
        return false;
    }
    //checks if the input is in the list and return true or false
    moveFront(L);
    while (Index(L) != -1) {
        char *elem = (char *) get(L);
        if (strcmp(input, elem) == 0) {
            return true;
        }
        moveNext(L);
    }
    return false;
}

//clock style input checker for cache cause im mentally done
int list_check_clock(List L, char *input) {
    if (length(L) == 0) {
        return -1;
    }
    moveFront(L);
    while (Index(L) != -1) {
        char *elem = (char *) get(L); // gets the cursor element from the list
        if (strcmp(input, elem) == 0) {
            return Index(L); // found the element so I return the index
        }
        moveNext(L);
    }
    return -1; // element not found
}

//FIFO policy
void FIFO_insert(List L, List History, char *input, int size) {
    if (list_check(L, input) == true) {
        printf("HIT\n");
        return;
    }
    printf("MISS\n");
    if (list_check(History, input) == true) {
        capacity++;
    } else {
        compulsory++;
    }
    if (cache_size_checker(L, size) == FULL) {
        //cache = full
        append(History, strdup(front(L))); //
        deleteFront(L); //pop out the oldest entry
    }
    append(L, strdup(input)); //
}

//Least Recently Used (LRU)
void LRU_insert(List L, List History, char *input, int size) {
    if (list_check(L, input)) {
        printf("HIT\n");
        //since we know there is an existing input in the cache we delete it from its spot in the cache and move to theback
        delete (L);
    } else {
        printf("MISS\n");
        if (list_check(History, input) == true) {
            capacity++;
        } else {
            compulsory++;
        }
    }
    if (cache_size_checker(L, size) == FULL) {
        //cache = full
        append(History, strdup(front(L)));
        deleteFront(L); //pop out the oldest entry
    }
    //adding the most recently used entry
    append(L, strdup(input));
}

//Clock Policy
void CLOCK_insert(List L, List History, char *input, int size) {
    // int current_position = Index(L);
    int pos = list_check_clock(L, input);
    if (pos != -1) {
        printf("HIT\n");
        setReferenceBit(L, pos, 1); // Set the reference bit to 1
        return;
    }
    printf("MISS\n");
    if (list_check(History, input) == true) {
        capacity += 1;
    } else {
        compulsory += 1;
    }

    // Check if the cache is full
    if (cache_size_checker(L, size) != FULL) {
        append(L, strdup(input));
        setReferenceBit(L, length(L) - 1, 0);
        makeClockHand(L); //keeps it at first item
        return;
    }

    // the cache is full, time to pop out an element
    char *deleted = (char *) clockDelete(L);
    append(History, strdup(deleted));
    insertAfterClockHand(L, strdup(input));
    delete (L);
    setReferenceBit(L, Index(L), 0);
    return;
}

int main(int argc, char *argv[]) {
    int size = 0;
    char *policy;
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Invalid Arguments\n");
        return 1;
    }
    if (argc == 3) {
        if (strcmp(argv[1], "-N") == 0) {
            size = atoi(argv[2]);
            if (size <= 0) {
                fprintf(stderr, "size too low\n");
                return -1;
            }
            policy = "FIFO";
        } else {
            printf("Invalid argument: %s\n", argv[1]);
            return 1;
        }
    } else if (argc == 4) {

        if (strcmp(argv[3], "-C") == 0) {
            policy = "CLOCK";
            size = atoi(argv[2]);
        } else if (strcmp(argv[3], "-F") == 0) {
            policy = "FIFO";
            size = atoi(argv[2]);
        } else if (strcmp(argv[3], "-L") == 0) {
            policy = "LRU";
            size = atoi(argv[2]);
        } else {
            printf("Invalid argument: %s\n", argv[3]);
            return 1;
        }
    }
    // create a cache List
    List cache = newList();
    // create history list for types of misses
    List History = newList();
    // user input
    char input[1024] = { 0 };
    // user input to pass into the functions
    char *inputPtr;
    while (fgets(input, 1024, stdin) != NULL) {
        // read user input from stdin using fgets
        input[strcspn(input, "\n")] = 0; // Remove newline character from input
        // cast input to a char pointer
        inputPtr = input;
        if (strcmp(policy, "CLOCK") == 0) {
            makeClockHand(cache);
            CLOCK_insert(cache, History, inputPtr, size);
        } else if (strcmp(policy, "FIFO") == 0) {
            FIFO_insert(cache, History, inputPtr, size);
        } else if (strcmp(policy, "LRU") == 0) {
            LRU_insert(cache, History, inputPtr, size);
        }
    }

    // prints out the number of compulsory and capacity misses
    printf("%d %d\n", compulsory, capacity);

    // free the cache memory
    freeList(&cache);
    freeList(&History);
    return 0;
}
