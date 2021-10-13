#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <wait.h>
#include <stdio.h>
#include "sizes.h"
#include "stackLib.h"
#include <limits.h>

void handle_Interrupt();
void handle_Stop();
void handle_Child();
int fgHandler(int pid);
void changeExitcode(int value);
int checkIfBgProcess(char* command);

Stack* suspendedProcesses;
Stack* runningProcesses;

char* getPath();

void initialiseProcessStacks()
{
    suspendedProcesses = createStack(5);
    runningProcesses = createStack(5);
}

void killAllChildren()
{
    int process;
    while((process = sPop(runningProcesses)) != INT_MAX)
        kill(process, SIGTERM);
    while((process = sPop(suspendedProcesses)) != INT_MAX)
        kill(process, SIGTERM);
}

void sigHandler(int signal)
{
    switch(signal)
    {
        case(SIGINT):handle_Interrupt();
            break;
        case(SIGTSTP):handle_Stop();
            break;
        case(SIGCHLD):handle_Child();
            break;
        default: return;
    }
}

void handle_Child()
{
    waitpid(-1, NULL, WNOHANG);//killing zombies process
}

void handle_Stop()
{
    int process = sPop(runningProcesses);
    if (process != INT_MAX)//if stack isnt empty
    {
        if(kill(process, SIGSTOP) < 0) {
            sPush(runningProcesses, process);//if suspending fails, push it back to running processes stack
            perror("Stop error: ");
        }
        else {
            sPush(suspendedProcesses, process);
        }
    }
}

void handle_Interrupt()
{
    int process = sPop(runningProcesses);
    if(process != INT_MAX)
    {
        if (kill(process, SIGTERM) < 0) {
            //if termination fails, force kill
            kill(process, SIGKILL);
        }
    }
}

void bgHandler(int newProcess)//if new process is 1, it means that an & was found and the end of the command
{
    int currentProcess;
    if(newProcess < 0)
    {
        currentProcess = sPop(suspendedProcesses);//getting last suspended process
        if (currentProcess != INT_MAX) {
            if (kill(currentProcess, SIGCONT) < 0)
            {
                perror("Error resuming process: ");
                sPush(suspendedProcesses, currentProcess);//push suspended process back on stack if resumption fails
            } else
                sPush(runningProcesses, currentProcess);
        } else
            printf("\nNo processes on stack\n");
    }
}

int fgHandler(int pid)
{
    int sus = 0;
    int status = 0;
    if(pid < 0)
    {
        int currentProcess = sPop(runningProcesses);//put first bg running process in foreground
        if (currentProcess == INT_MAX) {
            currentProcess = sPop(suspendedProcesses);//if no bg process, check for suspended ones
            sus = 1;
        }
        if (currentProcess != INT_MAX)//if no bg or suspended processes found, put them in foreground
        {
            if (sus == 1) {
                if (kill(currentProcess, SIGCONT) < 0)//result determines whether kill generated an error or not
                {
                    sPush(suspendedProcesses, currentProcess);
                    perror("Error resuming process: ");
                }//push suspended process back on stack if resumption fails
                else {
                    sPush(runningProcesses,currentProcess);
                    if (waitpid(currentProcess, &status, WUNTRACED) < -1) {
                        perror("Fg wait error: ");
                    }
                }
            }
            else
            {
                sPush(runningProcesses,currentProcess);//pushing running process pid back on stack
                if (waitpid(currentProcess, &status, WUNTRACED) < -1) {
                    perror("Fg wait error: ");
                }
            }
        }
        else
            printf("\nNo processes on stack\n");
    }
    else{
        sPush(runningProcesses, pid);
        if (waitpid(pid, &status, WUNTRACED) < -1) {
            perror("Fg wait error 2: ");
        }
    }
    if(!WIFEXITED(status))
    {
        changeExitcode(WEXITSTATUS(status));
        sPop(runningProcesses);//pop child back from stack
    }else {
        changeExitcode(WEXITSTATUS(status));
        if(WEXITSTATUS(status) == 1)//if no command found
        return -1;
    }
}

int checkIfBgProcess(char* command)
{
    int size = (int)(strlen(command) - 1);
    //if last character is an '&'
    if(*(command+size) == '&')
    {
        *(command + size) = '\0';//replace it with a \0
        return 1;
    }
    else return 0;
}

int externCommand(char* line)
{
    char lineCopy[stringSize];
    char pathCopy[envVarSize];
    int i = 0;
    int bg = 0;//to check if command has & as the last character
    char* token;
    char tokenCopy[stringSize];
    char* argList[SIZE] = {};
    char firstArgWithSlash[stringSize]= {};
    __pid_t child;
    //making a copy of line to prevent the original from being altered if the input is not an external function
    strcpy(lineCopy, line);
    strcpy(pathCopy, getPath());
    token = strtok(lineCopy, " ");

    bg = checkIfBgProcess(token);
    //getting the arguments
    while(token)
    {
        if(i == 0)
        {
            //putting a / infront of the first argument to be used in the execv function
            strcpy(firstArgWithSlash, "/");
            strcat(firstArgWithSlash, token);
        }
        argList[i] = token;
        token = strtok(NULL," ");
        i++;
    }

    //getting first path
    token = strtok(pathCopy, ":");
    //if child is -1 that means there was a forking error. Forking hell!
    if((child=fork())==-1)
    {
        perror("Fork error");
        return -1;
    }
    else if(child == 0)//this is the child
    {
        //ignoring signals in the child so  that parent takes cares of forwarding signals
        if(signal(SIGINT, SIG_IGN)== SIG_ERR)
            perror("Ignore error: ");
        if(signal(SIGTSTP, SIG_IGN)== SIG_ERR)
            perror("Ignore error ");
        while(token != NULL)
        {
            //making a copy of token to prevent tokenising issues
            strcpy(tokenCopy, token);
            strcat(tokenCopy, firstArgWithSlash);
            //no need to check if execv is < 0 since it has to iterate multiple time
            execv(tokenCopy, argList);
            token = strtok(NULL, ":");
        }
        //if child reaches this point, it means it found no process to replace it
        exit(EXIT_FAILURE);
    }
    else//parent process
    {
        if(bg == 0)
            return fgHandler(child);
        else
            sPush(runningProcesses, child);//dont call fg handler since we dont want to wait for it
    }
}