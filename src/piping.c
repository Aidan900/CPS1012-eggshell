#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "dynamicArray.h"
#include "sizes.h"

#define pipeArraySize 10

int pipeCommandHandler(char** commands, int size, dynArray* vars);
char* commandParser(char* line,dynArray* vars);

int pipeInterface(char* line, dynArray* vars)
{
    char lineCopy[stringSize];
    char* token;
    strncpy(lineCopy, line, stringSize);//making a copy of line to prevent strtok changing the initial line
    token = strtok(lineCopy, "|");
    //if token and line are not the same, THERE IS A PIPE WOOOOoOOOO
    if(strncmp(token, line, stringSize) != 0)
    {
        //create an array to store commands
        char** commands = malloc(pipeArraySize*sizeof(char*));
        for(int i = 0; i < pipeArraySize; i++)
        {
            commands[i] = calloc(stringSize, sizeof(char));
        }

        strncpy(lineCopy, line,stringSize);//making a copy of line to prevent strtok changing the initial line
        token = strtok(lineCopy, "|");
        int totalCommands = 0;
        while(token)//populating array with the commands
        {
            if(totalCommands>0) {
                //removing spaces
                strncpy(commands[totalCommands], &token[1], stringSize);

            }
            else strncpy(commands[totalCommands], token, stringSize);
            totalCommands++;
            token = strtok(NULL, "|");
        }
        pipeCommandHandler(commands, totalCommands, vars);
        free(commands);
        return 1;
    }
    else return -1;
}

int pipeCommandHandler(char** commands, int size, dynArray* vars)
{
    int stdoutOriginal = dup(STDOUT_FILENO);
    int stdinOriginal = dup(STDIN_FILENO);
    int index;
    int readPipe;
    int writePipe;
    int pipesArray[(size-1)*2];//size is total commands and pipes is -1 this value. *2 since pipe() requires an array of 2
    for(int i = 0; i < (size-1);  i++)
    {
        if(pipe(&pipesArray[i*2]) < 0)//creating pipes
            perror("Error creating pipes: ");
    }
    fflush(stdout);
    fflush(stdin);
    for(index = 0; index < size; index++)
    {
        readPipe = (index - 1) *2;//even indices are read pipes
        writePipe = (index * 2) + 1;//odd indices are write pipes
        if(index == 0)//first case
        {
            if (dup2(pipesArray[writePipe], STDOUT_FILENO) < 0)
                perror("Error piping commands: ");
            commandParser(commands[index], vars);
            close(pipesArray[writePipe]);//we can close pipe since we copied it
        }
        else if(index == size -1)//last case
        {
            if (dup2(pipesArray[readPipe], STDIN_FILENO) < 0)
                perror("Error piping commands: ");
            if (dup2(stdoutOriginal, STDOUT_FILENO) < 0)
                perror("Error piping commands: ");
            commandParser(commands[index], vars);
            close(pipesArray[readPipe]);
            if (dup2(stdinOriginal, STDIN_FILENO) < 0)
                perror("Error piping commands: ");
        }
        else //general case
        {
            if (dup2(pipesArray[readPipe], STDIN_FILENO) < 0)
                perror("Error piping commands: ");
            if (dup2(pipesArray[writePipe], STDOUT_FILENO) < 0)
                perror("Error piping commands: ");
            commandParser(commands[index], vars);
            close(pipesArray[readPipe]);
            close(pipesArray[writePipe]);
        }
    }
}