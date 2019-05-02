#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <stdbool.h>
#include <string.h>

extern int errno;

int checkId(char* firstFile, char* secondFile, int sizeFirst, int sizeSecond){
    int retValue=1;
    int errnum;
    int fdInFirst, fdInSecond;
    char bufFirst[sizeFirst];
    char bufSecond[sizeSecond];
    fdInFirst=open(firstFile, O_RDONLY);
    fdInSecond=open(secondFile, O_RDONLY);
    int countFirst=0, countSecond=0;
    int valFirst, valSecond;

    if (fdInFirst < 0) {
        errnum=errno;
        fprintf(stderr, "Error system call: %s\n", strerror(errnum));
    	exit(EXIT_FAILURE);
    } else if (fdInSecond < 0) {
        errnum=errno;
        fprintf(stderr, "Error system call: %s\n", strerror(errnum));
    	exit(EXIT_FAILURE);
    } else {
        read(fdInFirst, bufFirst, sizeFirst);
        read(fdInSecond, bufSecond, sizeSecond);

        while ((countFirst < sizeFirst) && (countSecond < sizeSecond)) {
            valFirst = bufFirst[countFirst];
            valSecond = bufSecond[countSecond];
            if (valFirst != valSecond) {
                while ((valFirst == 32) || (valFirst == 10)) {
                    valFirst = bufFirst[++countFirst];
                }
                while ((valSecond == 32) || (valSecond == 10)) {
                    valSecond = bufSecond[++countSecond];
                }
                if ((valFirst - valSecond == 32) || (valFirst - valSecond == -32)) {
                    valFirst = bufFirst[++countFirst];
                    valSecond = bufSecond[++countSecond];
                    retValue = 3;
                    continue;
                }
                if (valFirst == valSecond) {
                    retValue = 3;
                    valFirst = bufFirst[++countFirst];
                    valSecond = bufSecond[++countSecond];
                    continue;
                }
                return 2;
            } else {
                valFirst = bufFirst[++countFirst];
                valSecond = bufSecond[++countSecond];
            }
        }
            close(fdInFirst);
            close(fdInSecond);
        }
    return retValue;
}

int main(int argc, char* argv[]) {
    int ret;
    char msg[40];
    if (argc!=3){
        char fail[10]="fail";
        sprintf(msg, "not enough args\n");
        perror(msg);
        exit(0);
    }
    else {
        char *firstFile = argv[1];
        char *secondFile = argv[2];
        struct stat buf;
        struct stat bufSecond;
        if (stat(firstFile, &buf) == -1) {
            sprintf(msg, " Error occurred attempting to stat %s\n", firstFile); /*printf*/
            perror(msg);
            exit(0);
        }
        if (stat(secondFile, &bufSecond) == -1) {
            sprintf(msg, " Error occurred attempting to stat %s\n", secondFile); /*printf*/
            perror(msg);
            exit(0);
        }
        if(buf.st_size==bufSecond.st_size){
            if((buf.st_dev==bufSecond.st_dev)&&(buf.st_ino==bufSecond.st_ino)){
                exit(1);
            }
            ret=checkId(firstFile, secondFile, buf.st_size, bufSecond.st_size);
            exit(ret);
        }
        ret=checkId(firstFile, secondFile, buf.st_size, bufSecond.st_size);
        exit(ret);
    }
}
