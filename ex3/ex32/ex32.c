/*************************************
 * Renana Yanovsky Eichenwald
 * 308003862
 */
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <wait.h>

#define MAX_PATH 151
#define MAX_FILE 460
#define NUM_OF_LINES 3
#define OUTPUT_FILE "output.txt"
#define RESULT_FILE "results.csv"
#define TEMP_OUTPUT_FILE "outCFile.out"
#define RUN_FILE_C "./outCFile.out"
#define TIMEOUT 40
#define MAX_TIME_PASS 5
#define RUN_EX31 "./comp.out"


void errorInSysCall(){
    fprintf(stderr, "Error: system call\n");
    exit(EXIT_FAILURE);
}
/**
 * remove All files that not in used
 */
void cleanAll(){
    unlink(OUTPUT_FILE);
    unlink(TEMP_OUTPUT_FILE);
}
/*
checkDi(char oriPath[MAX_PATH]) {
    DIR *pointToDir;
    DIR *checkDir;
    char path[MAX_PATH];
    struct dirent *pDirent;
    struct dirent *pDirentToCheck;
    pointToDir = opendir(oriPath);
    if (pointToDir == NULL) {
        fprintf(stderr, "Error: open file\n");
        exit(EXIT_FAILURE);
    }
    pDirent = readdir(pointToDir);
    strcat(path, oriPath);
    while (pDirent != NULL) {

if(pDirent->d_type==(unsigned char)'8'){
    printf("%u\n", pDirent->d_type);
}

        strcat(path, "/");
        printf("%s\n", pDirent->d_name);
        strcat(path, pDirent->d_name);
        printf("%s\n", path);
        pDirent = readdir(pointToDir);
        strcpy(path, oriPath);
    }
    closedir(pointToDir);
}
 */

char* addTheFileToThePath(char* path, char* nameOfFile){
    char newPath [MAX_PATH];
    char tempPath [MAX_PATH];
    strcpy(tempPath, path);
    newPath[0]='\0';
    strcat(newPath, tempPath);
    strcat(newPath, "/");
    strcat(newPath, nameOfFile);
    //printf("%s\n", newPath);
    char* returnPath=newPath;
    return returnPath;
}
/**
 * the function cheks if the file is '.' or '..'
 * @param fileName the file name to check
 * @return true if the file is '.' or '..', false otherwise
 */
bool fileOrDot(char* fileName){
    char* tempFileName=fileName;
    if((tempFileName[0]=='.')&&(strlen(tempFileName)==1)){
        return true;
    }
    if((strcmp(fileName, "..")==0)&&(strlen(fileName)==2))
        return true;
    int len=strlen(fileName)-2;
    if((fileName[len]=='/')||(fileName[len]=='.')){
        if(fileName[len]+1=='.'){
            return true;
        }
    }
    return false;
}

/**
 * the function checks if the file ends with '.c'
 * @param fileName the file to check
 * @return true if ends with '.c' or '.C', false otherwise
 */
bool chekCFile(char* fileName){
    int len = strlen(fileName)-1;
    if(len>0) {
        if (fileName[len - 1] == '.') {
            if ((fileName[len] == 'c') || (fileName[len] == 'C')) {
                return true;
            }
        }
    }
    return false;
}

/**
 * the function initiolize a string
 * @param str the string to initiolize
 */
void initStr(char* str){
    str[0]='\0';
}


/**
 * the function return the mode of the path
 * @param path the path to check
 * @return the mode of the path
 */
int checkDirectory(const char* path){
    struct stat buf;
    int returnValue;
    if(stat(path, &buf)!=0) return 0; //false
    returnValue = S_ISDIR(buf.st_mode);
    return returnValue;
}

/**
 * the function checks if the string is empty
 * @param str the string to check
 * @return if the string is empty-true, false otherwise
 */
bool isStringEmpty (char* str){
    if(str[0]=='\0')
        return true;
    return false;
}

/**
 * the function checks if there is a c file
 * @param path the path
 * @param name the name of the file
 * @param returnValue
 */
void lookForCFile (char* path, char* name, char returnValue[MAX_PATH]){
    char savePath[MAX_PATH];
    strcpy(savePath, path);
    DIR* pdir;
    char speciName[MAX_PATH];

    //check if there is c file
    if(chekCFile(name)){
        //addTheFileToThePath(savePath, name);
        //strcpy(returnValue, savePath);
        strcpy(path, addTheFileToThePath(path, name));
        strcpy(returnValue, path);
        return;
    }
    if(!(pdir=opendir(path))){
        errorInSysCall();
        closedir(pdir);
    }
    //check all files in directory
    struct dirent* dirFile;
    dirFile=readdir(pdir);
    while (dirFile!=NULL){
        strcpy(speciName, dirFile->d_name);
        //if not . or ..
        if(!(fileOrDot(speciName))) {
            //check if directory
            if (dirFile->d_type == DT_DIR) {
                strcpy(savePath, addTheFileToThePath(savePath, speciName));
                lookForCFile(savePath, speciName, returnValue);

                //if there is nothing in the return value
                if (!isStringEmpty(returnValue)) {
                    closedir(pdir);
                    return;
                }
                strcpy(savePath, path);
            } else if (chekCFile(speciName)) {
                closedir(pdir);
                strcpy(savePath ,addTheFileToThePath(savePath, speciName));
                strcpy(returnValue, savePath);
                return;
            }
        }else{ dirFile=readdir(pdir);
            continue; }

        dirFile=readdir(pdir);
    }

    //if there is no c file
    initStr(returnValue);
    closedir(pdir);
}
/**
 *
 * @param grade
 * @return the comment according to the grade
 */
char* getComment(int grade){
    char* comment="";
    switch (grade){
        case 0: comment= "NO_C_FILE";
            break;
        case 20: comment = "COMPILATION_ERROR";
            break;
        case 40: comment = "TIMEOUT";
            break;
        case 60: comment = "BAD_OUTPUT";
            break;
        case 80: comment = "SIMILAR_OUTPUT";
            break;
        case 100: comment = "GREAT_JOB";
            break;
        default: comment = "NO_C_FILE";
    }
    return comment;
}

int runTheCFile(char* fileToRun, char* inPath, char* outPath){
    int fdIn, fdOut, status=1;
    fdIn=open(inPath, O_RDONLY);
    if(fdIn<0)
        errorInSysCall();
    fdOut=open(OUTPUT_FILE, O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IRGRP| S_IWGRP| S_IWUSR);
    if(fdOut<0)
        errorInSysCall();
    dup2(fdIn, 0);
    dup2(fdOut, 1);
    close(fdIn);
    close(fdOut);
    pid_t pid;
    pid=fork();
    if(pid<0)
        errorInSysCall();
    if(!pid){
        char* argv[MAX_PATH]={"gcc", "-o", TEMP_OUTPUT_FILE, fileToRun, NULL};
        status=execvp(argv[0], argv);
        errorInSysCall();
    }else{
        waitpid(pid, &status, WCONTINUED);
        //change!!
        if(status>=1)
            return 20;
        pid_t pid1;
        pid1=fork();
        if(pid1<0)
            errorInSysCall();
        else if(!pid1){
            char* argv[MAX_PATH]={RUN_FILE_C, NULL};
            execvp(argv[0], argv);
            errorInSysCall();
        }else{
            int counter=0;
            while((counter<=MAX_TIME_PASS)&&(waitpid(pid1, &status, WNOHANG)==0)){
                sleep(1);
                counter++;
            }
            if(counter>=MAX_TIME_PASS)
                return TIMEOUT;
            if (status==1)
                return TIMEOUT;
        }
        return 0;
    }

}

/**
 * the function put all the details into 1 string
 * @param first
 * @param name
 * @param grade
 * @param comment
 * @param returnValue
 * @return
 */
char* allDetaisTogether(bool first, char* name, char* grade, char* comment, char* returnValue){
    if(!first){
        strcat(returnValue, "\n");
    }
    strcat(returnValue, name);
    strcat(returnValue, ", ");
    strcat(returnValue, grade);
    strcat(returnValue, ", ");
    strcat(returnValue, comment);
    return returnValue;
}

/**
 * get all details to write to the file
 * @param first
 * @param grade
 * @param name
 */
void getGrade(bool first, int grade, char* name){
    char gradeToWrite[NUM_OF_LINES];
    char line[2*MAX_PATH]={0};
    //open the result file that we create already
    int fdInOut = open(RESULT_FILE, O_APPEND|O_WRONLY);
    sprintf(gradeToWrite, "%d", grade);

    //get the comment
    char* comment=getComment(grade);

    //get all the details
    allDetaisTogether(first, name, gradeToWrite, comment, line);

    write(fdInOut, line, strlen(line));
    close(fdInOut);
}
/**
 * compare between files according to ex31
 * @param pathOfFirst
 * @param pathOfC
 * @return
 */
int compareBetweenFiles(char* pathOfFirst, char* pathOfC){
    sleep(0.1);
    int returnValue=0;
    char* argv[MAX_FILE]={RUN_EX31, pathOfFirst, pathOfC, NULL};
    pid_t pid=fork();
    if(pid<0)
        errorInSysCall();
    if(!pid){
        execvp(argv[0], argv);
        errorInSysCall();
    }
    else{
        waitpid(pid, &returnValue, WCONTINUED);
        int result = WEXITSTATUS(returnValue);
        //int r = RUN_EX31 pathOfFirst pathOfC;
        switch (result){
            case 1: returnValue = 100;
                break;
            case 2: returnValue = 60;
                break;
            case 3: returnValue = 80;
                break;
            default: returnValue = 80;
        }
        return returnValue;
    }
}

int compareBetweenFiles1(char* pathOfFirst, char* pathOfC){
    sleep(0.1);
    int returnValue=0;
    int status;
    char* argv[MAX_FILE]={RUN_EX31, pathOfC, pathOfFirst, NULL};
    pid_t pid=fork();
    if(pid<0)
        errorInSysCall();
    if(!pid){
        execvp(argv[0], argv);
        errorInSysCall();
    }
    else{
        while(waitpid(pid, &status, 0)<0);
        int result = WEXITSTATUS(status);
        //char tempPath(MAX_PATH);

        switch (result){
            case 1: returnValue = 100;
                break;
            case 2: returnValue = 60;
                break;
            case 3: returnValue = 80;
                break;
            default: returnValue = 80;
        }
        return returnValue;
    }
}


/**
 * the function write the grade to the file
 * @param first
 * @param cFile
 * @param dire
 * @param lines
 * @return
 */
int writeGradeToFile(bool first, char cFile[MAX_PATH], char dire[], char lines[NUM_OF_LINES][MAX_PATH]){
    int fdOut=0;
    char* output;
    int fdIn;
    int returnValue;
    fdOut=open(OUTPUT_FILE, O_WRONLY|S_IRGRP|S_IWGRP|O_CREAT, S_IRUSR|O_TRUNC|S_IWUSR);
    if(fdOut<0){
        errorInSysCall();
    }
    /*****************************/
    //no c file
    if(isStringEmpty(cFile)){
        getGrade(first, 0, dire);
        return -1;
    }
    output= lines[2];
    fdIn=open(lines[1], O_RDONLY);
    if(fdIn<0)
        errorInSysCall();

    //run the file
    returnValue = runTheCFile(cFile, lines[1], output);
    if(returnValue!=0){
        getGrade(first, returnValue, dire);
        return returnValue;
    }

    //compare between the files
    returnValue = compareBetweenFiles(lines[2], OUTPUT_FILE);
    getGrade(first, returnValue, dire);
    return returnValue;


}

/**
 * the function checks if there a c file in the path
 * @param path the path to check
 */
void getFiles(char lines[NUM_OF_LINES][MAX_PATH]){
    //get the current path by line [0]
    char pathOfDir[MAX_PATH];
    char pathTemp[MAX_PATH];
    char pathOfFile[MAX_PATH];
    bool flagWritten=true;
    char newPath[MAX_PATH];
    char saveCFile[MAX_PATH];
    char saveName[MAX_PATH];

    strcpy(pathOfDir, lines[0]);
    DIR *pointToDir;
    struct dirent *pDirent;
    strcpy(pathTemp, pathOfDir);
    //open the path
    //***********************
    pointToDir = opendir(lines[0]);
    if (pointToDir == NULL) {
        errorInSysCall();
    }
    pDirent=readdir(pointToDir);

    while (pDirent != NULL){
        //save the current path in temp path
        strcpy(pathTemp, pathOfDir);
        //save the path of the file
        char savePath[MAX_PATH];
        if(fileOrDot(pDirent->d_name)){
            pDirent=readdir(pointToDir);
            continue;
        }
        //!!!!!
        //strcpy(savePath, pathOfDir);
        strcpy(savePath, addTheFileToThePath(pathTemp, pDirent->d_name));
        //printf("%s\n", newPath);

        //if the file is '.' or '..'

        if(chekCFile(savePath)){
            strcpy(saveCFile, savePath);
        }else if(checkDirectory(savePath)){
            lookForCFile(savePath, pDirent->d_name, saveCFile);
        }
        //if there is c file exist
        strcpy(saveName, pDirent->d_name);
        writeGradeToFile(flagWritten, saveCFile, saveName, lines);
        flagWritten = false;

        pDirent=readdir(pointToDir);
        strcpy(pathTemp, pathOfDir);
    }
    closedir(pointToDir);
    //return 1;

}

void getLines(char text[MAX_FILE], int size) {
    int i = 0;
    int numOfLine = 0, numInLine = 0;
    char lines[NUM_OF_LINES][MAX_PATH];
    for (; i < size; ++i) {
        if (text[i] == '\n') {
            lines[numOfLine][numInLine] = '\0';
            numOfLine++;
            numInLine = 0;
            i++;
        }
        lines[numOfLine][numInLine++] = text[i];
    }
    getFiles(lines);

    /*
    for(; j<NUM_OF_LINES; ++j){
        printf("%s\n", lines[j]);
    }
     */
    //return lines;
}

/**
 * the function reads the file
 * @param fileName the file to read
 */
void readFile(char* fileName){
    char *file = fileName;
    struct stat buf;
    if (stat(file, &buf) == -1)
        errorInSysCall();
    int fdIn;
    char buff[MAX_PATH];
    fdIn=open(file, O_RDONLY);
    if(fdIn<0)
        errorInSysCall();
    read(fdIn, buff, MAX_FILE);
    getLines(buff, buf.st_size);
    //printf("%s\n", buff);
    close(fdIn);
}
/**
 * create the result file
 * @return file description of results.csv
 */
int createFileForResults(){
    int fdIn=0;
    fdIn = open(RESULT_FILE, O_RDONLY|O_TRUNC|O_CREAT, S_IRUSR| S_IWGRP| S_IWUSR| S_IRGRP);
    if(fdIn<0){
        errorInSysCall();
    }
    close(fdIn);
    return fdIn;
}

/**
 * main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[]) {
    if(argv[1]==NULL){
        fprintf(stderr, "Error: arguments\n");
        exit(EXIT_FAILURE);
    }
    if(createFileForResults()<0) {
        return 0;
    }

    readFile(argv[1]);

    cleanAll();

    //checkDi(argv[1]);
    printf("Hello, World!\n");
    return 0;
}