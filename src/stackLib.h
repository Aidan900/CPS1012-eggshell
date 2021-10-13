//Created by Aidan Cauchi
#include <stddef.h>

#ifndef DATA_STRUCTURES_ASSIGNMENT_STACKLIB_H
#define DATA_STRUCTURES_ASSIGNMENT_STACKLIB_H

#endif //DATA_STRUCTURES_ASSIGNMENT_STACKLIB_H

typedef struct stk
{
    int* stackPtr;
    size_t maxSize;
    size_t usedSize;
}Stack;

/*Pushes item on stack
 * Return 1 if sucessful
 * Returns 0 if unsuccessful*/
int sPush(Stack* stk, int item);

/*Pops an item from the stack
 * Returns the item if successful
 * Returns max value of int if not successful
 */
int sPop(Stack* stk);

//returns a pointer to a stack
Stack* createStack(size_t initSize);