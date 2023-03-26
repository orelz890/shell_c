#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>


int main() {
char command[1024];
char *token;
char *outfile;
char prompt[1024];
int i, fd, amper, redirect, err, append ,retid, status, flag, haveJobFlag;
char *argv[10];

strcpy(prompt, "hello: ");

while (1)
{
    haveJobFlag = 0;
    flag = 0;
    printf("%s", prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    // printf("\ni= %d, 1= %s, 2= %s, 3= %s\n", i, argv[0], argv[1], argv[2]);
    // fflush(stdout);
    if (i == 3 && !strcmp(argv[0], "prompt") && !strcmp(argv[1], "="))
    {
        // printf("\nim here prompt\n");
        // fflush(stdout);
        strcpy(prompt, argv[2]);
        strcat(prompt, ": ");
        continue;
    }
    
    /* Does command line end with & */ 
    if (!haveJobFlag && ! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
        haveJobFlag = 1;
    }
    else 
        amper = 0;

    if (!haveJobFlag && i > 1  && !strcmp(argv[i - 2], ">")) {
        // printf("\nim here >\n");
        // fflush(stdout);
        redirect = 1;
        outfile = argv[i - 1];
        flag = 1;
        haveJobFlag = 1;
        // printf("\nim here2 >\n");
        // fflush(stdout);
        }
    else 
        redirect = 0; 

    if (!haveJobFlag && i > 1 &&!strcmp(argv[i - 2], "2>")) {
        // printf("\nim here 2>\n");
        // fflush(stdout);
        err = 1;
        outfile = argv[i - 1];
        flag = 1;
        haveJobFlag = 1;
    }
    else 
        err = 0;
    
    if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">>")) {
        // printf("\nim here >>\n");
        // fflush(stdout);
        redirect = 1;
        append = 1;
        outfile = argv[i - 1];
        flag = 1;
        haveJobFlag = 1;
    }
    else 
        append = 0;

    if (!haveJobFlag && i > 1 && ! strcmp(argv[0], "echo"))
    {
        // int j = 1;

        // while (j < i)
        // {

        //     j++;
        // }
        
        
    }
    

    if (flag)
    {
        argv[i - 2] = NULL;
    }
    
    /* for commands not part of the shell command language */ 

    if (fork() == 0) { 
        /* redirection of IO ? */
        if (redirect) {
            // printf("im here");
            // fflush(stdout);
            fd = open(outfile, O_WRONLY | (append ? O_APPEND : O_TRUNC) | O_CREAT, 0660); 
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            close(STDOUT_FILENO);
            dup(fd);
            close(fd);
        }else if(err){
            // printf("im here err");
            // fflush(stdout);
            fd = creat(outfile, 0660); 
            close (STDERR_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execvp(argv[0], argv);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
