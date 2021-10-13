
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include "dynamicArray.h"
#include "sizes.h"

char* commandParser(char* line,dynArray* vars);
void changeEnvCwd(char* newD);
int showEnvVar(char* varName);

void sourceCmd(char* line, dynArray* vars)
{
    //getting file name (supposedly)
    //note user must specify file type
    char* token = strtok(line, " ");//getting past the source keyword
    token = strtok(NULL, " ");//getting filename
    char buffer[stringSize]= {};
    FILE* fp;
    int lineNum = 0;
    if(token!=NULL)
    {
        fp = fopen(token, "r");
        if(fp == NULL)
        {
            perror("Error opening file: ");
            return;
        }
        //getting line by line and inputting it as a command
        while (fgets(buffer, stringSize, fp) != NULL) {
            /*
             * For some reason, this loop was executing again once fgets reached the end of file
             * When fflush(fp) was introduced this stopped happening (and thus the bug was fixed, i think).
             */
            fflush(fp);

            //making a copy of line to remove the '\n' which was causing errors when used as input to the command parser
            char lineCopy[stringSize] = {};
            strncpy(lineCopy, buffer, strlen(buffer) - 1);
            commandParser(lineCopy, vars);
            }
        fclose(fp);
    }
}


void changeDir(char* token)
{
    char cwd[stringSize]={}; //to hold new cwd
    token = strtok(NULL, " ");//token should now be the directory destination
    if(token != NULL)
    {
        // .. means one level above current directory
        if (strncmp(token, "..", 2) == 0) {
            char cwdCopy[stringSize];//making a copy so as not to alter the original cwd with strtok
            char cwdPaths[SIZE][stringSize] = {};//array for storing the different levels of the dir
            int pathsTokenised = 0;//counts the level of depth which indicates the size of the array

            getcwd(cwdCopy, stringSize);
            token = strtok(cwdCopy, "/");//getting first dir level
            while (token != NULL) {
                //if token isn't null, then it is a dir level
                if (token != NULL) {
                    strcpy(cwdPaths[pathsTokenised], token);//copy level into respective index in array
                    pathsTokenised++;
                }
                token = strtok(NULL, "/");
            }
            /*note:
             * i starts at 1 since at index 0 array has a blank space due to paths starting with /
             * and strtok tokenizes using /. It also ends at <pathsTokenised to
             * */
            //-1 since index starts from 0
            for (int i = 0; i < pathsTokenised - 1; i++) {
                if (i == 0) {
                    strcpy(cwd, "/");
                    strcat(cwd, cwdPaths[i]);
                }//if i == 1, replace cwd from the beginning
                else {
                    strcat(cwd, "/");
                    strcat(cwd, cwdPaths[i]);
                }
            }
        }
            //if first character is a / it means the user is specifying a directory outside the current one
        else if (token[0] == '/') {
            strcpy(cwd, token);
        }
            //else it is a custom directory inside the cwd
        else {
            getcwd(cwd, stringSize);
            strcat(cwd, "/");
            strcat(cwd, token);
        }
        //if chdir is 0, path is valid
        if (chdir(cwd) == 0) {
            //making sure cwd is the correct cwd
            getcwd(cwd, envVarSize);
            printf("New directory: %s\n", cwd);
        } else {
            //else reset cwd
            getcwd(cwd, envVarSize);
            printf("CWD: %s\n", cwd);
            perror("chdir failed: ");
        }
        changeEnvCwd(cwd);
    }
    else
        printf("Invalid directory\n");
}

void printCommand(char* line, dynArray* vars)
{
    char* token = strtok(line, " ");
    int firstToken = 1;
    while(token != NULL)
    {
        //get string separated by whitespace
        token = strtok(NULL, " ");
        if (token != NULL)
        {
            //if first character is a $ search for variable
            if (token[0] == '$')
            {
                //check  env vars and user vars excluding the '$'
                if (showEnvVar(&token[1]) < 0)
                    showVariable(vars, &token[1]);
                //if it doesnt find the variable, nothing gets output
            }
                //else if it is a ", print output exactly
            else if (token[0] == '"')
            {
                if(firstToken == 1)
                {
                    int i=1;
                    if(token[strlen(token)-1]=='"')//if last character is a " it means that only one word is within the "
                    {
                        while(token[i]!='"')
                        {
                            if(token[i]=='\0')//if found null terminator by any chance, end function
                            {
                                token = NULL;
                                break;
                            }
                            else
                                printf("%c", token[i]);//else print character by character
                            i++;
                        }
                        firstToken = 0;
                    }
                    else
                    {
                        printf("%s ", &token[1]);
                        token = strtok(NULL, "\"");
                        if (token != NULL)
                            printf("%s ", token);
                    }
                }
                else
                {
                    printf("%s ", &token[1]);//print without first "
                    token = strtok(NULL, "\"");//parse again to get the end of the literal
                    if (token != NULL)
                        printf("%s ", token);
                }
            }
                //else just print the token
            else {
                printf("%s ", token);
            }
        }
    }
    printf("\n");
}