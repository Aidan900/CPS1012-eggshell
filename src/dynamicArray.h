//
// Created by aidan on 4/19/18.
//
#include <glob.h>
#include "sizes.h"
//#define stringSize 50

#ifndef EGGSHELL_DYNAMICARRAY_H
#define EGGSHELL_DYNAMICARRAY_H

#endif //EGGSHELL_DYNAMICARRAY_H

struct variable
{
    char varName[stringSize];
    char value[stringSize];
};

typedef struct dynamicArray
{
    struct variable* variable;
    size_t currentSize;
    size_t maxSize;
}dynArray;

/*
 * Allocates a dynamic array and returns a pointer to it
 */
dynArray* initialiseDynArray(size_t initSize);


/*
 * Add a variable to the dynamic array.
 * Returns -1 if there was an error
 */
int addVariable(dynArray* arr,char* name, char* value);

/*
 * Changes the values of an already existing variable
 * Returns -1 if variable waas not found
 */
int editVariable(dynArray* arr,char* name, char* newValue);

/*
 * Outputs the value of a variable on screen
 * Returns -1 if variable was not found
 */
int showVariable(dynArray* arr, char* name);

/*
 * Display all the contents of the array
 */
void showAllArrayVariables(dynArray* arr);

/*
 * Returns the value of a variable
 * Returns NULL if value was not found
 */
char* getValue(dynArray* arr, char* name);