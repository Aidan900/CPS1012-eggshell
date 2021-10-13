#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include "sizes.h"
#include "dynamicArray.h"

int changeEnvVars(char* varName, char* value);

char path[envVarSize]="";
char prompt[envVarSize] = "eggshell-1.0>";
char cwd[envVarSize]="";
char user[envVarSize]="";
char home[envVarSize]="";
char shell[envVarSize];
char* terminal;
int exitcode = -1;

char* getPath()
{
    return path;
}

void changeEnvCwd(char* newD)
{
    strcpy(cwd, newD);
}

char* getPrompt()
{
    return prompt;
}
void showAllEnvVars(dynArray* arr)
{
    printf("PATH=%s\n", path);
    printf("PROMPT=%s\n", prompt);
    printf("SHELL=%s\n", shell);
    printf("USER=%s\n", user);
    printf("HOME=%s\n", home);
    printf("CWD=%s\n", cwd);
    printf("TERMINAL=%s\n", terminal);
    printf("EXITCODE=%d\n", exitcode);
    showAllArrayVariables(arr);
}

void loadEnvVars(char** env)
{
    terminal = ttyname(STDIN_FILENO);
    getcwd(shell, envVarSize);
    char* token;
    int envIndex = 0;
    char* currentEnvVar;
    char tempCopy[envVarSize];
    while(env[envIndex]!=NULL)
    {
        //tokenising env vars with '='
        strncpy(tempCopy, env[envIndex], envVarSize);
        currentEnvVar = strtok(tempCopy, "=");
        token = strtok(NULL,"\n");
        if(strcmp(currentEnvVar,"PATH") ==0) strcpy(path,token);
        else if(strcmp(currentEnvVar,"USER") ==0) strcpy(user,token);
        else if(strcmp(currentEnvVar,"HOME") ==0) strcpy(home,token);
        else if(strcmp(currentEnvVar,"PWD") ==0) strcpy(cwd,token);
        envIndex++;
    }
}

int showEnvVar(char* varName)
{
    if(strcmp(varName,"PATH") ==0)
    {
        printf("%s\n",path);
        return 1;
    }
    else if(strcmp(varName,"USER") ==0)
    {
        printf("%s\n",user);
        return 1;
    }
    else if(strcmp(varName,"HOME") ==0)
    {
        printf("%s\n",home);
        return 1;
    }
    else if(strcmp(varName,"CWD") ==0)
    {
        printf("%s\n",cwd);
        return 1;
    }
    else if(strcmp(varName,"PROMPT") ==0)
    {
        printf("%s\n",prompt);
        return 1;
    }
    else if(strcmp(varName,"TERMINAL") ==0)
    {
        printf("%s\n",terminal);
        return 1;
    }
    return -1;
}

int variableHandler(dynArray* vars, char* variable,char* value)
{
    //if variable isnt an env var, proceed to check if variable is user declared. If is isnt in the dynamic array then create it
    if(value[0]=='$')
    {
        value = getValue(vars, &value[1]);
        if(value == NULL)return -1;
    }
    if (changeEnvVars(variable, value) == 1)return 1;
    else
    {
        if (editVariable(vars, variable, value) == 1)return 1;
        else return addVariable(vars, variable, value);
    }
}

int changeEnvVars(char* varName, char* value)
{
    if(strcmp(varName,"PATH")==0)strcpy(path,value);
    else if(strcmp(varName,"HOME")==0)strcpy(home,value);
    else if(strcmp(varName,"USER")==0)
    {
        if(setenv("USER", value, 1)<0)perror("Error editing USER:");
        strcpy(user, value);
    }
    else if(strcmp(varName,"PROMPT")==0)
    {
        strcpy(prompt,value);
        strcat(prompt, ">");
    }
    else if(strcmp(varName,"TERMINAL")==0) strcpy(terminal,value);
    else return 0;//if not found
    return 1;
}

void changeExitcode(int value){exitcode = value;}