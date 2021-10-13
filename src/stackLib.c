//Created by Aidan Cauchi

#include "stackLib.h"
#include <malloc.h>
#include <limits.h>

Stack* createStack(size_t initSize)
{
    Stack* stk = (Stack*) malloc(sizeof(Stack));//creating structure in memory
    stk->stackPtr = (int* ) malloc(sizeof(int)*initSize); //allocating the actual stack in memory
    stk->maxSize= initSize;
    stk->usedSize= 0;
    return stk;
}

int sPush(Stack* stk, int item)
{
    if(stk->maxSize == stk->usedSize)//if stack is full
    {
        stk->stackPtr = (int* ) realloc(stk,(stk->maxSize)+5);//increase stack size by 5
        if(stk->stackPtr == NULL) return 0 ;//return error
        else  //else increment stack pointer and push
        {
            stk->maxSize += 5;
            stk->stackPtr++;
            *(stk->stackPtr) = item;
            stk->usedSize++;
        }
    }
    else if(stk->usedSize == 0)//if stack is empty
    {
        *(stk->stackPtr) = item;//push item on stack without increasing the stack pointer(first item)
        stk->usedSize++;
    }
    else //else increment stack pointer and push
    {
        stk->stackPtr++;
        *(stk->stackPtr) = item;
        stk->usedSize++;
    }
    return 1;
}

int sPop(Stack* stk)
{
    int temp;
    if(stk->usedSize!=0)//if stack isnt empty, pop
    {
        temp = *(stk->stackPtr);
        stk->stackPtr--;
        stk->usedSize--;
        return temp;
    }
    else return INT_MAX;//otherwise return error
}
