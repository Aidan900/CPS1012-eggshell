#include <memory.h>
#include <printf.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "sizes.h"

char* catenateFileInput(char* line);
int redirStream(char* newFilename, FILE* streamToReplace, char* readOrWrite);
char* extractFileName(const char* line, const char* token);
char* getLineWithoutFilename(char* line, const char* token);
int tokenCount(const char* str, const char* token);
char* hereStringHandler(char* line);

FILE* redirInOut = NULL;
int saveStream = 0;
int isStreamChanged = 0;
int streamToReset = 0;
int hereStringFound = 0;

//this method returns the number of occurrences of token in the string
//used to determine how many > or < there are in str
int tokenCount(const char* str, const char* token)
{
    char* index = strstr(str, token);
    int i = 0;
    if(index != NULL)
    {
        while(*(index+i) == *token)
            i++;

        return i;
    }
    else return 0;
}

//replaces the file stream with the one indicated by > or < and changes line without said characters
char* redirHandler(char* line)
{
    char* token;
    char* newLine = line;
    char* filename;
    char lineCopy[stringSize]={};

    strcpy(lineCopy, line);
    int occurrences = 0;
    //parsing with >. if token and line are not the same after parsing, it means a > was found
    token = strtok(lineCopy, ">");
    if(strncmp(token, line, stringSize) != 0)
    {
        char* writeOrAppend;
        occurrences = tokenCount(line, ">");
        if(occurrences == 1) writeOrAppend = "w";
        else writeOrAppend = "a";
        filename = extractFileName(line, ">");
        //replacing stdout with the filename
        if(redirStream(filename, stdout, writeOrAppend)!=-1)
        {
            newLine = getLineWithoutFilename(line, ">");
            if (newLine[strlen(newLine) - 1] == '\n')
                newLine[strlen(newLine) -
                        1] = ' ';//replacing last character with a space because \n was generating errors in external commands
        }
    }
    else
    {
        strcpy(lineCopy, line);//refresh linecopy after being modified by strtok
        token = strtok(lineCopy, "<");
        //parsing with < this time. if token and line are not the same after parsing, it means a < was found
        if(strncmp(token, line, stringSize) != 0)
        {

            occurrences = tokenCount(line, "<");
            if(occurrences == 1)//if < appears once, read from file
            {
                filename = extractFileName(line, "<");
                //replacing stdin with the filename
                if(redirStream(filename, stdin, "r") != -1) {
                    newLine = getLineWithoutFilename(line, "<");
                    newLine = catenateFileInput(newLine);
                }
            }
            else if(occurrences == 3)//else it is a here string
            {
                newLine = hereStringHandler(newLine);
            }
        }
        if(newLine[strlen(newLine)-1]=='\n')
            newLine[strlen(newLine)-1] = ' ';//replacing last character with a space because \n was generating errors in external commands
    }
    return newLine;
}

char* hereStringHandler(char* line)
{
    char hereString[stringSize] = {};
    char* offset = strstr(line, "<");
    int i = 0;

    //getting the here string
    while(*(offset + i) != '\0')
    {
        if(*(offset + i) != '<')
            strncat(hereString, (offset + i), 1);

        i++;
    }
    strncat(hereString, (offset + i), 1); //catenating the terminating character at the end
    //open temporary file to write to
    FILE* tempFile = fopen("hereTemp.txt", "w");
    if(tempFile == NULL)
    {
        perror("Error executing command: ");
    }
    else fputs(hereString, tempFile);
    fclose(tempFile);
    //read from temp file
    redirStream("hereTemp.txt", stdin, "r");
    hereStringFound = 1;
    return getLineWithoutFilename(line, "<");
}

//replacing standard stream with the given file name
int redirStream(char* newFilename, FILE* streamToReplace, char* readOrWrite)
{
    //opening file
    redirInOut = fopen(newFilename, readOrWrite);
    if(redirInOut == NULL)
    {
        perror("Redirection error 1: ");
        return -1;
    }
    saveStream = dup(fileno(streamToReplace));//saving a copy of stdout
    if (dup2(fileno(redirInOut), fileno(streamToReplace)) < 0)//replacing stream with file
    {
        perror("Redirection error 2: ");
        return 0;
    }
    isStreamChanged = 1;
    streamToReset = fileno(streamToReplace);
}

char* extractFileName(const char* line, const char* token)
{
    char* filename = calloc(stringSize, sizeof(char));
    char* index = strstr(line, token);
    int i = 0;

    //incrementing i so that index + i point to the next non token character in the line
    while(*(index+i) == *token)
        i++;
    //i should now point to a space
    strcat(filename, (index+i+1));//+1 to remove initial space
    return filename;
}

char* getLineWithoutFilename(char* line, const char* token)
{
        char *newLine = calloc(stringSize, sizeof(char));
        if (newLine == NULL) {
            perror("Malloc error: ");
            return line;
        }
        int i = 0;

        while (*(line + i) != *token) {
            i++;
        }

        strncat(newLine, line, (size_t) i);//catenating the command up until the token
        newLine[i] = '\n';//terminating character at end of command
        free(line);
        return newLine;
}

//reset the changed stream back to the original file descriptor
void closeRedir()
{
    if(isStreamChanged == 1)
    {
        //the respective file streams need to be called since streamToReset is a fd not stream and fflush accepts streams
        if(streamToReset == fileno(stdout))fflush(stdout);
        else if(streamToReset == fileno(stderr))fflush(stderr);
        else if(streamToReset ==  fileno(stdin))fflush(stdin);
        fclose(redirInOut);
        if (dup2(saveStream, streamToReset) < 0) perror("Error restoring stream: ");
        close(saveStream);
        isStreamChanged = 0;
        redirInOut = NULL;
        streamToReset = 0;
    }
    if(hereStringFound == 1)
    {
        if(remove("hereTemp.txt")< 0)
        {
            perror("Error deleting temp file:");
        }
        hereStringFound = 0;
    }
}

//reading from file
char* catenateFileInput(char* line)
{
    int offset = 0;
    char* strBuffer = calloc(bufferSize, sizeof(char));//to hold entire file input
    if(strBuffer == NULL)perror("Malloc error: ");
    else
    {
        if(line[strlen(line)-1]=='\n')
            line[strlen(line)-1] = ' ';//replacing last character with a space because \n was generating errors in external commands
        strncpy(strBuffer, line, strlen(line));
        offset =(int) strlen(line);
        char tempBuffer[stringSize] = {};//buffer to catenate string with strBuffer (which holds entire command)
        while(!feof(redirInOut))
        {
            if(fgets(tempBuffer, stringSize, redirInOut)!= NULL)
            {
                tempBuffer[(strlen(tempBuffer))-1] = ' '; //placing space at the end of line to separate different lines
                strncat((&strBuffer[0] + offset), tempBuffer, strlen(tempBuffer));
                offset += (int) strlen(tempBuffer);
            }
        }
        strBuffer[strlen(strBuffer)-1] = '\n';//putting end of line
    }
    return strBuffer;
}