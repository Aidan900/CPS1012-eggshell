#include <malloc.h>
#include <memory.h>
#include "dynamicArray.h"

dynArray* initialiseDynArray(size_t initSize)
{
    if(initSize==0)return NULL;
    else
    {
        dynArray *ptr = (dynArray *) malloc(sizeof(dynArray));
        ptr->currentSize = 0;
        ptr->variable = (struct variable*)malloc(initSize*sizeof(struct variable));
        ptr->maxSize = initSize;
        return ptr;
    }
}

int addVariable(dynArray* arr, char* name, char* value)
{
    char newValue[stringSize];
    strncpy(newValue, value, strlen(value));//not storing \n
    if(arr->currentSize==arr->maxSize)
    {
        arr->variable = realloc(arr->variable,arr->maxSize*2);
        arr->currentSize++;//incrementing before to point to empty memory location
        if(arr->variable == NULL)return -1;
        arr->maxSize = arr->maxSize*2;
        strcpy((arr->variable + arr->currentSize)->varName,name);
        strcpy((arr->variable + arr->currentSize)->value,newValue);
    }
    else
    {
        strcpy((arr->variable + arr->currentSize)->varName,name);
        strcpy((arr->variable+ arr->currentSize)->value,newValue);
        arr->currentSize++;
    }
    return 1;
}

int editVariable(dynArray* arr, char* name, char* newValue)
{
    int successful =-1;
    for(int i =0; i<arr->currentSize;i++)
    {
        if(strcmp((arr->variable+i)->varName,name)==0)
        {
            strcpy((arr->variable+i)->value,newValue);
            successful = 1;
        }
    }
    return successful;
}

int showVariable(dynArray* arr, char* name)
{
    int i =0;
    while(i < arr->currentSize)
    {
        if(strcmp((arr->variable+i)->varName,name)==0)
        {
            printf("%s ",(arr->variable+i)->value);
            return 1;
        }
        else i++;
    }
    return -1;
}

char* getValue(dynArray* arr, char* name)
{
    int i =0;
    while(i < arr->currentSize)
    {
        if(strcmp((arr->variable + i)->varName,name)==0)
        {
            return (arr->variable + i)->value;
        }
        else i++;
    }
    return NULL;
}

void showAllArrayVariables(dynArray* arr)
{
    for(int i = 0; i < arr->currentSize; i++)
        printf("%s=%s\n",(arr->variable + i)->varName, (arr->variable + i)->value);
}
