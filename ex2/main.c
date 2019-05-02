/*********
 * Renana Yanovsky Eichenwald
 * 308003862
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wait.h>

#define MAX_ARRAY 70
#define MAX_JOBS 512
#define MAX_ARG_LEN 70
#define ERROR_MESS "Error in system call\n"

/**
 * Job struct with the name and the id
 */
typedef struct {
    char* name;
    pid_t id;
}Job;

/**
 * global jobs array
 */
Job jobs[MAX_JOBS];

/**
 * global counter to the first empty place in the jobs array
 */
int placeInJobsArray=0;

/**
 * the function adds a job to the jobs array
 * @param name
 * @param id
 */
void addANewJob(char* name, pid_t id){
    if(placeInJobsArray<MAX_JOBS){
        //jobs[placeInJobsArray].name=strdup(name);
        //jobs[placeInJobsArray].id=id;

        Job newJob;
        newJob.name=strdup(name);
        newJob.id=id;
        jobs[placeInJobsArray]=newJob;

        placeInJobsArray++;
    }
}

/**
 * the function will return the sub string
 * @param string the original
 * @param sub will hold the sub string
 * @param pos the position to start from
 * @param len the length of the sub string
 */
void substr (char string [], char sub [], int pos, int len){
    int c=0;
    while(c<len){
        sub[c]=string[pos+c-1];
        c++;
    }
    sub[c]='\0';
}


/**
 * combine 2 strings together
 * @param first the first string
 * @param second the second string to add
 */
void strTogether(char* first, char* second){
    if((first==NULL)||(second==NULL)) {

        printf(ERROR_MESS);
        //return "NULL";
    }
    int lenFirst = strlen(first);
    int lenSecond = strlen(second);
    int i = 0;
    for (; i < lenSecond; ++i) {
        first[lenFirst+i] = second[i];
    }
    first[lenFirst+lenSecond]='\0';
    //return first;
}

/**
 * the function removes a line
 * @param line to remove
 */
void removeLine(char* line){
    substr(line, line, 2, (strlen(line)-2));
}

char* cutUnnec(char* buffer, char toCut){
    int i=0;
    char* ret=buffer;
    if(buffer!=NULL) {
        while (buffer[i] != '\0') {
            if(buffer[i]==toCut){
                substr(buffer, buffer, 0, i-1);
                substr(ret, ret, i+1, strlen(ret-i));
            }
            i++;
        }
        return ret;
    }
}

/**
 * the function get the words in the "
 * @param jump how many places to jump
 * @param line the line
 * @return the words between the " "
 */
char* getWord(char* jump, char* line){
    char* substri = malloc(sizeof(char)*MAX_ARRAY);
    char* first =strchr(line, '\"');
    char* last = strchr(first+1, '\"');
    *jump = last-line+1;
    substr(line, substri, first-line+1, last-first+1);
    removeLine(substri);
    return substri;
}

/**
 * the function returns the arguments splits.
 * @param args will be the split arguments
 * @param buffer the arguments from the user
 * @param numOfWords number of words
 * @return
 */
char** getArgs(char** args, char* buffer, int* numOfWords){
    char* word;
    int flag=0;
    int i=0;
    char temp[MAX_ARRAY];
    char* save=buffer;
    char copyBuffer[MAX_ARRAY];
    char* args1=buffer;
    char saveArgs[MAX_ARRAY];
    char* args2;
    char getSub[MAX_ARRAY];
    strcpy(copyBuffer, save);
    strcpy(saveArgs, args1);

    //check if there " in the command and save the place
    for(; i<strlen(buffer); ++i){
        if(buffer[i]=='"') {
            flag = i;
            break;
        }
    }
    //get first token
    word = strtok(buffer, " ");
    int placeInArray=0;
    //walk through other token until the end
    while (word!=NULL){
        //if there is " in the command
        if(word[strlen(word)-1]=='"'){
            strcpy(saveArgs, args[placeInArray-1]);
            char* haveAp=0;
            int jump=0;
            //get the words in the " "
            haveAp=getWord(&jump, copyBuffer);
            strcpy(temp, haveAp);
            save+=jump;
            strcpy(copyBuffer, save);
            free(haveAp);
            args[placeInArray++]=strdup(temp);
            word = strtok(NULL, "\"");
            word = strtok(NULL, " ");
            //++*numOfWords;
            placeInArray--;
            //remove the first word in the "
            substr(saveArgs, getSub, 1, flag-3);
            //add the words in the " to the command
            strTogether(getSub, args[placeInArray]);

            //add to the array
            args[placeInArray-1]=getSub;
            continue;
        }
        else {
            args[placeInArray] = strdup(word);
        }
        //find the next word
        ++placeInArray;
        word=strtok(NULL, " ");
        ++*numOfWords;
    }
    /*
    if (flag){
        strcpy(temp, args[*numOfWords-1]);
        substr(temp, temp, 0, (strlen(temp)-2));
        temp[(strlen(temp))]='\"';
        temp[(strlen(temp))]='\0';
        //strTogether(temp, \"\);
        strTogether(args[*numOfWords-2], " ");
        strTogether(args[*numOfWords-2], temp);
        args[*numOfWords-1]=NULL;
        --*numOfWords;
    }*/

    /*
    //we got all the args
    while(placeInArray<MAX_ARRAY)
        args[placeInArray++]=0;
        */
    return args;

}


/**
 * return the first char
 * @param word the word to take
 * @return the first char
 */
char startWith (char* word){
    return word[0];
}
/**
 * the function for cd command
 * @param args the arguments
 */
void cdCommand(char** args, char* cdPre){
    printf("%d\n", getpid());
    char buffer [MAX_JOBS];
    char first='\0';
    char path[MAX_JOBS];
    char* combine;
    //if fail
    if (getcwd(buffer, MAX_JOBS)==NULL)
        fprintf(stderr, ERROR_MESS);
    if(args[1]!=NULL) {
        first = startWith(args[1]);

    }
    // cd ~ or " cd "
    if((first == '~')||(args[1])==NULL) {
        if ((strlen(args[1])==1)&&(args[2]==NULL)){
            chdir(getenv("HOME"));
        }else {
            substr(args[1], args[1], 2, (strlen(args[1]) - 1));
            if (chdir(getenv("HOME")))
                fprintf(stderr, ERROR_MESS);
            else {
                //get the path
                if (getcwd(path, sizeof(path)) == NULL) {
                    fprintf(stderr, ERROR_MESS);
                } else {
                    //add the other part of the path
                    path[strlen(path)] = '\0';
                    strTogether(path, args[1]);
                    chdir(path);
                }
            }
        }
        strcpy(cdPre, buffer);
    //"cd - "
    }else if (first == '-'){
        if(!strcmp(cdPre, "\0")){
            printf("bash: cd: OLDPWD not set\n");
        }else{
            if(chdir(cdPre)== -1){
                fprintf(stderr, ERROR_MESS);
            }else{
                printf("%s\n", cdPre);
                strcpy(cdPre, buffer);

            }
        }
    }else{
        //no specific flag before cd
        if(chdir(args[1])!=0){
            fprintf(stderr, ERROR_MESS);
        } else{
            strcpy(cdPre, buffer);
        }
    }

}

/**
 * the function removes a job from the array
 * @param pid the if of the job to delete
 */
void removeJob(pid_t pid) {
    int i = 0;
    for (; i < placeInJobsArray; ++i) {
        //find the job to remove
        if (jobs[i].id == pid) {
            //free
            free(jobs[i].name);
            break;
        }
        while (i < MAX_JOBS - 1) {
            jobs[i++] = jobs[i + 1];
        }


    }
    //set the last place in the jobs array
    Job j;
    j.name = NULL;
    j.id = 0;
    jobs[MAX_JOBS] = j;
    //placeInJobsArray--;
}

/**
 * the function removes all the zombies
 */
void removeZombies(){
    //char tempName[MAX_ARG_LEN];
    int i=0;
    for(; i<MAX_JOBS && jobs[i].name!=NULL; ++i){
        //char name[MAX_ARRAY];
        int killZomb=0;
        killZomb=waitpid(jobs[i].id, NULL, WNOHANG);
        if(killZomb==-1)
            removeJob(jobs[i].id);

    }
}

/**
 * the jobs command, prints all the commands
 */
void jobsCommand(){
    removeZombies();
    int i=0;
    for(; i<MAX_JOBS; ++i){
        if(jobs[i].name!=NULL)
            printf("%d %s\n", jobs[i].id, jobs[i].name);
        else break;
    }
}

/**
 * the function call the right function according to the command
 * @param args the arguments
 * @param waitOrNot if the father should wait or not
 * @param buffer the name of the command
 * @return
 */
int doCommand(char** args, bool* waitOrNot, char* buffer){
    if (args[0]==NULL)
        return 1;
    //remove the zombies
    removeZombies();
    pid_t pid;
    int toDo=1;
    pid=fork();
    //the father will print
    if(pid){
        printf("%d\n", pid);
        char name[MAX_ARG_LEN];
        //if the father should wait to his son
        if(*waitOrNot){
            //wait and remove
            wait(&toDo);
            removeJob(pid);
        }
        else{
            //add the job to the array of the jobs
            strcpy(name, buffer);
            addANewJob(name, pid);
        }
    }
    //the son
    if (!pid){
        //call the function
        toDo=execvp(args[0], args);
        exit (1);
    }
    return 1;

}

/**
 * the function start over with a new job
 * @param args the arguments array to initiolize
 * @param waitOrNot wait to the son or not
 */
void startOver(char** args, bool* waitOrNot){
    int i=0;
    for(; i<MAX_ARRAY; ++i){
        args[i]=0;
        //free (args[i]);
    }
    *waitOrNot=true;

}
/**
 * the function cut the space
 * @param buffer the string to cut the space from
 * @return the buffer without the space
 */
char* cutTheSpace(char* buffer){
    char* word=strtok(buffer, " ");
    return word;
}

/**
 * main function
 * @return 0 when finish
 */
int main() {
    bool waitOrNot = true;
    char **args = malloc(sizeof(char *) * MAX_ARRAY);
    char buffer[MAX_ARRAY]="/0";
    char nameOfJob[MAX_ARRAY];
    char tempName[MAX_ARG_LEN];
    bool status=true;
    char cdPre[MAX_JOBS]="\0";
    while(status) {
        printf("> ");
        int numOfArgs = 0;
        //get the command from the user
        fgets(buffer, MAX_ARRAY + 1, stdin);
        buffer[strlen(buffer)-1]='\0';
        //save the command
        strcpy(nameOfJob, buffer);
        //get the arguments
        args = getArgs(args, buffer, &numOfArgs);
        //check if the father should wait or not
        if ((args[numOfArgs - 1][0] == '&') && (args[numOfArgs - 1][1] == '\0')) {
            //if he shouldn't wait, add the command to the jobs array
            waitOrNot = false;
            char temp[MAX_ARRAY];
            strcpy(temp, args[numOfArgs-2]);
            args[numOfArgs-2]=cutTheSpace(temp);
            long int wordLen=strlen(args[numOfArgs-2]);
            args[numOfArgs-2][wordLen]=0;
            args[numOfArgs - 1]=NULL;
            nameOfJob[strlen(nameOfJob)-2]=0;

        }
        //get the name of the command
        char* command=args[0];
        char save[MAX_ARRAY];
        if(args[1]!=NULL)
            strcpy(save, args[1]);
        //if the command is cd
        if(!strcmp(command, "cd")){
            strcpy(args[1], save);
            cdCommand(args, cdPre);
        }
        //if the command is exit
        else if(!strcmp(command, "exit")){
            printf("%d\n", getpid());
            int i=0;
            for(; i<placeInJobsArray; ++i) {
                free(jobs[i].name);
                kill(jobs[i].id, 0);
            }
            free(args);
            break;
            //if the command is jobs
        } else if(!strcmp(command, "jobs")){
            jobsCommand();
            //for any other command
        }else {
            status = doCommand(args, &waitOrNot, nameOfJob);
        }
        //start over and get a new command
        startOver(args, &waitOrNot);

    }
    return 0;
}