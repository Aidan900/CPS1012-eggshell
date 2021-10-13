//Created by Aidan Cauchi

/*#define SIZE 20
#define envVarSize 150
#define stringSize 50*/

#include <stdio.h>
#include <printf.h>
#include <wchar.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include "linenoise.h"
#include "dynamicArray.h"
#include "sizes.h"


void loadEnvVars(char** env);
int variableHandler(dynArray* vars, char* variable,char* value);
int showEnvVar(char* varName);
void changeDir(char* token);
void printCommand(char* line, dynArray* vars);
void showAllEnvVars(dynArray* arr);
char* commandParser(char* line,dynArray* vars);
void sourceCmd(char* line, dynArray* vars);
int externCommand(char* line);
char* getPath();
char* getPrompt();
//void redir(char* line, FILE* redirInOut, int* isStreamChanged, int* saveStream);
//void closeRedir(FILE* streamToReset, FILE* redirInOut, int* isStreamChanged, int saveStream);
void closeRedir();
char* redirHandler(char* line);
void changeExitcode(int value);
int pipeInterface(char* line, dynArray* vars);
void initialiseProcessStacks();
void bgHandler(int pid);
int fgHandler(int pid);
void sigHandler(int signal);
void killAllChildren();

int main(int argc, char **argv, char **env)
{
    char* line;
    dynArray* vars = initialiseDynArray(3);
    initialiseProcessStacks();
    loadEnvVars(env);

    if(signal(SIGINT, sigHandler)== SIG_ERR)
        perror("Interrupt error: ");

    if(signal(SIGTSTP, sigHandler)== SIG_ERR)
        perror("Interrupt 2 error: ");

    if(signal(SIGCHLD, sigHandler)== SIG_ERR)
        perror("Child handling error: ");
    //TODO: ADD NO COMMAND ERROR
    /*TODO:
     * STRTOK WITH SPACE
     * PARSE ALL COMMANDS
     * IF NO COMMAND FOUND
     * TOKENISE AND = SHOULD BE FOUND
     * IF = IS FOUND THEN IT IS ASSIGNMENT STATEMENT
     * IF = IS NOT FOUND THEN VARIABLE DISPLAY IF VARIABLE EXISTS
     * */
    while(!NULL)
    {
        if((line=linenoise(getPrompt())) != NULL && (strncmp(line,"\0",1)!=0))//if line is equal to enter or ctrl c(line noise returns NULL), ignore it
        {
            if (pipeInterface(line, vars) < 0) {
                line = redirHandler(line);
                line = commandParser(line, vars);
                linenoiseFree(line);
            }
        }
    }
    return 0;
}

char* commandParser(char* line, dynArray* vars)
{
    char* token;
    if (strncmp(line, "\0", 1) != 0) {
        char lineCopy[stringSize];
        strcpy(lineCopy, line);//making a copy of line since strtok changes the string
        token = strtok(lineCopy, " ");
        //print command
        if (strncmp("print", token, stringSize) == 0) {
            printCommand(line, vars);
        }
            //exit command
        else if (strncmp("exit", token, stringSize) == 0)
        {
            killAllChildren();
            exit(EXIT_SUCCESS);
        }

        else if (strncmp("chdir", token, stringSize) == 0)
            changeDir(token);

        else if (strncmp("all", token, stringSize) == 0)
            showAllEnvVars(vars);

        else if (strncmp("source",token,stringSize) ==0)
            sourceCmd(line, vars);

        else if (strncmp("bg",token,stringSize) ==0)
            bgHandler(-1);

        else if (strncmp("fg",token,stringSize) ==0)
            fgHandler(-1);
        else
        {
            if(externCommand(line) < 0)
            {
                //if code reaches this point, that means that the command either deal with assignment, show variable or is invalid
                strncpy(lineCopy, line, stringSize);
                token = strtok(lineCopy, "=");
                char* varName = token;
                //if varname is different than line, then strtok found an =
                if (strncmp(varName, line,stringSize) != 0)
                {
                    char* varValue = strtok(NULL, "\0");//to store entire string
                    varName = strtok(varName, " ");//to remove space between varname and =
                    //varvalue starts from location 1 to remove space
                    if(varValue[1] != '$') {
                        if (variableHandler(vars, varName, &varValue[1]) == -1)perror("Error handling variable: ");
                    }
                    else
                        {
                            if(variableHandler(vars, varName, getValue(vars, &varValue[2])) == -1)perror("Error handling variable: ");
                        }
                }
                    //otherwise it must be either a call to display a variable or invalid
                else {
                    if (showEnvVar(line) < 0) {
                        //if variable isnt found anywhere (< 0) then it is an invalid command
                        if (showVariable(vars, line) < 0)printf("Error: %s is not a valid command.\n", line);
                        else
                            printf("\n");//outputting (flushing) the value in the stdout buffer
                    }
                }
            }
        }
        closeRedir();
    }
    return line;
}