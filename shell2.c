#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

// not complete <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void handle_signal(int signal)
{
    // pid_t pgid = getpgrp(); // get process group ID of current process
    // kill(-pgid, SIGTERM);   // send SIGTERM to every process in the group
    printf("You typed Control-C!\n");
}

typedef struct Node
{
    char *data[10];
    struct Node *prev;
    struct Node *next;
} node, *pnode;

void deepCopyArgv(char *src[10], char *dst[10])
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (src[i] != NULL)
        {
            dst[i] = malloc(strlen(src[i]) + 1); // allocate memory for the destination string
            strcpy(dst[i], src[i]);              // copy from the source string to the destination string
        }
        else
        {
            break; // stop looping if there are no more arguments
        }
    }
}

void freeCopyArgv(char *src[10])
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (src[i] != NULL)
        {
            free(src[i]);
        }
        else
        {
            break; // stop looping if there are no more arguments
        }
    }
}

void copyArgv(char *src[10], char *dst[10])
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (src[i] != NULL)
        {
            strcpy(dst[i], src[i]); // copy from the source string to the destination string
        }
        else
        {
            break; // stop looping if there are no more arguments
        }
    }
}

void printArgv(char *src[10])
{
    int i;
    for (i = 0; i < 10; i++)
    {
        if (src[i] != NULL)
        {
            printf("Argument %d: %s\n", i, src[i]);
        }
        else
        {
            break; // stop looping if there are no more arguments
        }
    }
}

// void saveCommendData(pnode current, char* argv[10]){
//     deepCopyArgv(argv, current->data);
//     printArgv(current->data);
//     // // copyArgv(argv);

//     current->next = (pnode)malloc(sizeof(node));
//     current->next->prev = current;
//     printf("\nim here\n");
//     fflush(stdout);

//     printf("\nim here2\n");
//     fflush(stdout);
//     current = current->next;
//     printf("\nim here3\n");
//     fflush(stdout);

// }

int main()
{

    signal(SIGINT, handle_signal);
    char command[1024];
    char *token;
    char *outfile;
    char prompt[1024];
    int i, fd, amper, redirect, err, append, retid, status, flag, haveJobFlag;
    char *argv[10];

    pnode root = (pnode)malloc(sizeof(node));
    root->prev = NULL;
    root->next = NULL;

    pnode current = root;
    strcpy(prompt, "hello: ");

    int flag2 = 0;

    while (1)
    {
        haveJobFlag = 0;
        flag = 0;
        printf("%s", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';

        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        // char* tmp;
        while (token != NULL)
        {
            argv[i] = token;
            // current->data[i] = (char*)malloc(sizeof(token));
            // strcpy(current->data[i], token);
            current->data[i] = token;

            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        deepCopyArgv(argv, current->data);
        printArgv(current->data);
        // // copyArgv(argv);

        current->next = (pnode)malloc(sizeof(node));
        current->next->prev = current;
        printf("\nim here\n");
        fflush(stdout);

        printf("\nim here2\n");
        fflush(stdout);
        current = current->next;
        printf("\nim here3\n");
        fflush(stdout);

        if (flag2)
        {
            printf("curr data= %s, %s", *current->prev->data, *current->prev->prev->data);
        }
        flag2 = 1;

        if (!strcmp(argv[0], "quit"))
        {
            printf("\ngood bye\n");
            fflush(stdout);
            exit(0);
        }

        if (!strcmp(argv[0], "!!"))
        {
            pnode temp = current->prev->prev;

            if (temp != NULL)
            {
                printf("\nim lost1\n");
                fflush(stdout);
                while (temp != NULL && !strcmp(*temp->data, "!!"))
                {
                    printf("\nim lost2\n");
                    fflush(stdout);
                    temp = temp->prev;
                    printf("\nim lost3\n");
                    fflush(stdout);
                }
            }

            if (temp == NULL)
            {
                printf("\nim lost4\n");
                fflush(stdout);

                continue;
            }
            printf("\nim lost5\n");
            fflush(stdout);
            deepCopyArgv(temp->data, argv);
        }

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
        if (!haveJobFlag && !strcmp(argv[i - 1], "&"))
        {
            amper = 1;
            argv[i - 1] = NULL;
            haveJobFlag = 1;
        }
        else
            amper = 0;

        if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">"))
        {
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

        if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], "2>"))
        {
            // printf("\nim here 2>\n");
            // fflush(stdout);
            err = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            err = 0;

        if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">>"))
        {
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

        if (!haveJobFlag && i > 1 && !strcmp(argv[0], "echo"))
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

        if (fork() == 0)
        {
            /* redirection of IO ? */
            if (redirect)
            {
                // printf("im here");
                // fflush(stdout);
                fd = open(outfile, O_WRONLY | (append ? O_APPEND : O_TRUNC) | O_CREAT, 0660);
                if (fd == -1)
                {
                    perror("open");
                    exit(1);
                }
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
            }
            else if (err)
            {
                // printf("im here err");
                // fflush(stdout);
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
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
