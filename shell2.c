#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>

#define MAX_VARIABLES 1024
#define MAX_VARIABLE_NAME_LEN 128
#define MAX_VARIABLE_VALUE_LEN 1024

typedef struct
{
    char name[MAX_VARIABLE_NAME_LEN];
    char value[MAX_VARIABLE_VALUE_LEN];
} variable;

variable variables[MAX_VARIABLES];
int num_variables = 0;

// Function to save a variable
void save_variable(char *name, char *value)
{
    if (num_variables < MAX_VARIABLES)
    {
        strcpy(variables[num_variables].name, name);
        strcpy(variables[num_variables].value, value);
        num_variables++;
        printf("Variable saved.\n");
    }
    else
    {
        printf("Maximum number of variables reached.\n");
    }
}

// Function to retrieve a variable
char *get_variable(char *name)
{
    int i;
    for (i = 0; i < num_variables; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            char *ans = (char *)malloc(strlen(variables[i].value) + 1);
            strcpy(ans, variables[i].value);
            return ans;
        }
    }
    return "";
}

void changeVarsToData(char *src[128])
{
    int i;
    for (i = 0; i < 128; i++)
    {
        if (src[i] != NULL && strlen(src[i]) > 1 && src[i][0] == '$' && src[i][1] != '?')
        {
            // printf("Argument %d: %s\n", i, src[i]);
            char *var = get_variable(src[i]);
            if (strcmp(var, ""))
            {
                memset(src[i], 0, strlen(src[i]));
                strcpy(src[i], var);
            }
        }
    }
}

void handle_tstp(int s)
{
    return;
}

void handle_signal(int s)
{
    pid_t pgid = getpgrp(); // get process group ID of current process
    signal(SIGTSTP, handle_tstp);
    killpg(pgid, SIGTSTP); // send SIGTERM to every process in the group
    printf("You typed Control-C!\n");
}

typedef struct Node
{
    char *data[128];
    struct Node *prev;
    struct Node *next;
} node, *pnode;

void deepCopyArgv(char *src[128], char *dst[128])
{
    int i;
    for (i = 0; i < 128; i++)
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

void freeCopyArgv(char *src[128])
{
    int i;
    for (i = 0; i < 128; i++)
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

void copyArgv(char *src[128], char *dst[128])
{
    int i;
    for (i = 0; i < 128; i++)
    {
        if (src[i] != NULL)
        {
            printf("%d,",i);
            fflush(stdout);
            strcpy(dst[i], src[i]); // copy from the source string to the destination string
        }
        else
        {
            break; // stop looping if there are no more arguments
        }
    }
}

void printArgv(char *src[128])
{
    int i;
    for (i = 0; i < 128; i++)
    {
        if (src[i] != NULL)
        {
            printf("Argument %d: %s       and len= %ld\n", i, src[i], strlen(src[i]));
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

int flagSeenIf = 0, flagSeenThen = 0, flagDoThen = 0, flagSeenFi = 0; 
char fullIfCommend[1024];


int main()
{

    signal(SIGINT, handle_signal);
    char command[1024];
    char *token;
    char *outfile;
    char prompt[1024];
    int i, fd, amper, redirect, err, append, retid, status, flag, haveJobFlag;
    char *argv[128];

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

        if (!strcmp(argv[0], "if"))
        {
            flagSeenIf = 1;
            printf("%s", ">");
            char *commends[128];
            memset(fullIfCommend, 0, strlen(fullIfCommend));
            int i;
            for (i = 0; i < 128; i++)
            {
                if (argv[i] != NULL)
                {
                    // printf("%d, %ld,", i, sizeof(command));
                    // fflush(stdout);
                    commends[i] = (char*)malloc(sizeof(command));
                    strcpy(commends[i], argv[i]);
                    strcat(fullIfCommend, argv[i]);
                    fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';
                    strcat(fullIfCommend, " ");
                }
                else
                {
                    break; // stop looping if there are no more arguments
                }
            }
            // printf("\n\n%s", commends[3]);
            // fflush(stdout);
            strcat(fullIfCommend, "; ");

            char commend[1024];
            memset(commend, 0, strlen(commend));
            fgets(commend, 1024, stdin);
            // i = 0;
            if (!strcmp(commend, "then\n"))
            {
                strcat(fullIfCommend, "then ");
                // commends[i] = malloc(sizeof(command));
                // strcpy(commends[i++], "then");
                
                printf("%s", ">");
                // strcat(fullIfCommend, ")\" != \"\" ]"); // continue here
                int flagElse = 0;
                while (fgets(commend, 1024, stdin) && strcmp(commend, "fi\n"))
                {
                    printf("%s", ">");
                    if (!strcmp(commend, "else\n"))
                    {
                        // commends[i] = (char *)malloc(sizeof(command));
                        // strcpy(commends[i], "else");
                        strcat(fullIfCommend, " else ");
                        flagElse = 1;
                    }else{
                        strcat(fullIfCommend, commend);
                        fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';
                        strcat(fullIfCommend, ";");

                        // commends[i] = (char *)malloc(sizeof(command));
                        // strcpy(commends[i], commend);
                        // commends[i][strcspn(commends[i], "\n")] = '\0';
                        // strcat(commends[i], ";");
                    }
                    i++;
                    memset(commend, 0, strlen(commend));
                    
                }
                if (flagElse == 0)
                {
                    // think what to do about wrong syntax && save coomend
                    printf("syntax error near unexpected token `fi'\n");
                    continue;
                }
                strcat(fullIfCommend, " ");
                strcat(fullIfCommend, commend);
                fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';


                // printf("fullIfCommend= %s\n\n",fullIfCommend);

                system(fullIfCommend);

                // fflush(stdout);
                // commends[i] = (char *)malloc(strlen(commend));
                // strcpy(commends[i], commend);
                // commends[i][strcspn(commends[i], "\n")] = '\0';
                // // printf("commends[10]= %s", commends[10]);
                // printArgv(commends);
                // fflush(stdout);
                // // freeCopyArgv(argv);
                // deepCopyArgv(commends, argv);
            }
            else
            {
                // think what to do about wrong syntax && save coomend
                printf("syntax error near unexpected token `fi'\n");
                continue;
            }
        }
        else
        {
            deepCopyArgv(argv, current->data);
        }
        printArgv(argv);
        // // copyArgv(argv);

        current->next = (pnode)malloc(sizeof(node));
        current->next->prev = current;
        current = current->next;

        if (flag2)
        {
            printf("curr data= %s, %s\n", *current->prev->data, *current->prev->prev->data);
        }
        flag2 = 1;

        if (!strcmp(argv[0], "quit"))
        {
            printf("\ngood bye\n");
            fflush(stdout);
            exit(0);
        }

        if (i == 2 && !(strcmp(argv[0], "read")))
        {
            char str[1022];
            char str2[2] = "$";

            fgets(str, sizeof(str), stdin);
            str[strlen(str) - 1] = '\0';

            printf("str= %s", str);
            fflush(stdout);
            save_variable(strcat(str2, argv[1]), str);
        }

        if (i == 3 && argv[0][0] == '$' && !strcmp(argv[1], "="))
        {
            printf("argv[0]= %s, argv[2]= %s", argv[0], argv[2]);
            fflush(stdout);
            save_variable(argv[0], argv[2]);
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

        if (i == 2 && !strcmp(argv[0], "cd"))
        {
            int result = chdir(argv[1]);
            if (result != 0)
            {
                perror("cd");
            }
            else
            {
                char cwd[10000];
                getcwd(cwd, sizeof(cwd));
                printf("curr path: %s\n", cwd);
            }
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
            if (!strcmp(argv[1], "$?"))
            {
                int status;
                int counter = 0;
                char *ch = (char *)malloc(sizeof(char));
                while (current->prev->prev->data[counter] != NULL)
                {
                    ch = strcat(ch, strcat(current->prev->prev->data[counter], " "));
                    counter++;
                }

                printf("command: %s\n", ch);
                status = WEXITSTATUS(system(ch));
                printf("status: %d\n", status);
            }
            changeVarsToData(argv);
        }

        if (flag)
        {
            argv[i - 2] = NULL;
        }

        /* for commands not part of the shell command language */
        if (flagSeenIf == 0 || flagSeenIf == 1 && flagSeenThen == 1 && flagSeenFi == 1){
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
}