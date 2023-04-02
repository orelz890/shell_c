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

int inHistory = 0;

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
    if (!inHistory)
    {
        pid_t pgid = getpgrp(); // get process group ID of current process
        signal(SIGTSTP, handle_tstp);
        killpg(pgid, SIGTSTP); // send SIGTERM to every process in the group
        printf("You typed Control-C!\n");
    }
    else
    {
        // exit from history mode
    }
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

void deleteFirstFromArgv(char *src[128])
{
    int i;
    for (i = 0; i < 127; i++)
    {
        if (src[i + 1] != NULL)
        {
            // printf("%d,", i);
            // fflush(stdout);
            memset(src[i], 0, strlen(src[i]));
            strcpy(src[i], src[i + 1]);
        }
        else
        {
            src[i] = NULL;
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
            printf("%d,", i);
            fflush(stdout);
            strcpy(dst[i], src[i]);
        }
        else
        {
            break;
        }
    }
}

void printArgv(char *src[128])
{
    int i;
    for (i = 1; i < 128; i++)
    {
        if (src[i] != NULL)
        {
            // printf("Argument %d: %s       and len= %ld\n", i, src[i], strlen(src[i]));
            if (src[i + 1] != NULL)
            {
                printf("%s\n", src[i]);
            }
            else
            {
                printf("%s\n", src[i]);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void printCmd(char *src[128])
{
    int i;
    for (i = 0; i < 128; i++)
    {
        if (src[i] != NULL)
        {
            // printf("Argument %d: %s       and len= %ld\n", i, src[i], strlen(src[i]));
            if (src[i + 1] != NULL)
            {
                printf("%s ", src[i]);
            }
            else
            {
                printf("%s\n", src[i]);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

void split(char **argv, int *pn, char *cmd, char *s)
{
    char *token;
    token = strtok(cmd, s);
    int i = 0;
    while (token != NULL)
    {
        argv[i] = malloc(sizeof(token));
        strcpy(argv[i], token);
        token = strtok(NULL, s);
        i++;
    }
    argv[i] = NULL;
    *pn = i;
}

void handlePipes(char **argv, char *cmd)
{
    int nr = 0;
    split(argv, &nr, cmd, "|");

    int *fd[2];
    int pc;

    char *tmp[128];

    for (int i = 0; i < nr; i++)
    {
        fd[i] = (int *)malloc(sizeof(int));
        split(tmp, &pc, argv[i], " ");
        if (i != nr - 1)
        {
            if (pipe(fd[i]) < 0)
            {
                perror("Error on creating pipes\n");
                return;
            }
        }
        if (fork() == 0)
        {
            if (i != nr - 1)
            {
                if (dup2(fd[i][1], 1) < 0)
                {
                    perror("Error duplicating file descriptor\n");
                    exit(EXIT_FAILURE);
                }
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if (i != 0)
            {
                if (dup2(fd[i - 1][0], 0) < 0)
                {
                    perror("Error duplicating file descriptor\n");
                    exit(EXIT_FAILURE);
                }
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            execvp(tmp[0], tmp);
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }

        if (i != 0)
        {
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
    }
}

char *merge(char *argv[128])
{
    char *result = (char *)malloc(sizeof(char)); // Allocate memory for the result string
    *result = '\0';                              // Initialize the result string as an empty string

    // Loop through each string in the array and concatenate it to the result string
    for (int i = 0; argv[i] != NULL; i++)
    {
        strcat(result, argv[i]);
        strcat(result, " ");
    }

    return result;
}

int main()
{

    signal(SIGINT, handle_signal);
    char command[1024];
    char *token;
    char *outfile;
    char prompt[1024];
    int i, fd, amper, redirect, err, append, retid, flag, haveJobFlag, status = 0,
        pipesNum = 0, flagSeenIf = 0, flagSeenThen = 0, flagDoThen = -1, flagSeenFi = 0,
        flagSeenElse = 0, flagIsStream = 0, flagCanEnter = 1, flag2 = 0;
    char *argv[128];
    char *argv2[128];

    // Init our history storage
    pnode root = (pnode)malloc(sizeof(node));
    root->prev = NULL;
    root->next = NULL;
    pnode current = root;

    // Print the prompt
    strcpy(prompt, "hello: ");

    while (1)
    {
        flagIsStream = 0;
        haveJobFlag = 0;
        flag = 0;
        printf("%s", prompt);
        fgets(command, 1024, stdin);
        command[strlen(command) - 1] = '\0';


        // Get last commends handler

        if (!strcmp(command, "\033[A") || !strcmp(command, "\033[B"))
        {
            while (1)
            {
                pnode curr = NULL;
                if (!strcmp(command, "\033[A"))
                {
                    curr = current->prev;
                } else // down
                {
                    curr = current->next;

                }
                if (curr != NULL)
                {

                    printCmd(curr->data);
                    char *tmp = merge(curr->data);
                    strcpy(command, tmp);
                    inHistory = 1;
                    if (curr)
                    {
                        current = curr;
                    }
                }else {
                    printf("Error: there is no command exists\n");
                }
                

                break;
            }

        }

        if (strchr(command, '|'))
        {
            if (!inHistory)
            {
                current->data[0] = (char *)malloc(sizeof(command) + 1);
                strcpy(current->data[0], command);
                pnode next = (pnode)malloc(sizeof(node));
                current->next = next;
                current->next->prev = current;
                current = current->next;
                current->next = NULL;
            }

            if (!strncmp(command, "if", 2))
            {
                char *output_string = strchr(command, ' ') + 1;
                handlePipes(argv2, output_string);
            }
            else
            {
                handlePipes(argv2, command);
                continue;
            }
        }


        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        // char* tmp;
        while (token != NULL)
        {
            argv[i] = token;
            current->data[i] = (char *)malloc(sizeof(token));
            strcpy(current->data[i], token);
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        // Handle if command
        if (!strcmp(argv[0], "if"))
        {
            flagSeenIf = 1;
            flagIsStream = 1;
            i--;

            deleteFirstFromArgv(argv);
        }

        // Handle then command   
        if (!strcmp(argv[0], "then"))
        {
            if (flagSeenIf == 1)
            {
                if (flagDoThen == 1)
                {

                    flagCanEnter = 1;
                }
                else
                {
                    flagCanEnter = 0;
                }

                flagSeenThen = 1;
                flagIsStream = 1;
            }
        }

        // Handle else command
        if (!strcmp(argv[0], "else"))
        {
            if (flagSeenThen == 1)
            {
                if (flagDoThen == 0)
                {
                    flagCanEnter = 1;
                }
                else
                {
                    flagCanEnter = 0;
                }
                flagSeenElse = 1;
                flagIsStream = 1;
            }
        }

        // Handle fi command
        if (!strcmp(argv[0], "fi"))
        {
            if (flagSeenElse == 1)
            {
                flagSeenIf = 0;
                flagSeenThen = 0;
                flagDoThen = -1;
                flagSeenElse = 0;
                flagIsStream = 1;
                flagCanEnter = 1;
            }
        }

        // If we didnt save the data yet, do so
        if (!inHistory)
        {

            deepCopyArgv(argv, current->data);
            pnode next = (pnode)malloc(sizeof(node));
            current->next = next;
            current->next->prev = current;
            current = current->next;
            current->next = NULL;
        }

        inHistory = 0;

        // Handle the quit case
        if (!strcmp(argv[0], "quit"))
        {
            printf("\ngood bye\n");
            fflush(stdout);
            exit(0);
        }

        // Handle the read case
        if (flagCanEnter == 1 && i == 2 && !(strcmp(argv[0], "read")))
        {

            char str[1022];
            char str2[2] = "$";

            fgets(str, sizeof(str), stdin);
            str[strlen(str) - 1] = '\0';
            save_variable(strcat(str2, argv[1]), str);
        }

        // Handle save a var
        if (flagCanEnter == 1 && i == 3 && argv[0][0] == '$' && !strcmp(argv[1], "="))
        {
            save_variable(argv[0], argv[2]);
        }

        // Handle the repeat last command
        if (flagCanEnter == 1 && !strcmp(argv[0], "!!"))
        {

            pnode temp = current->prev->prev;

            if (temp != NULL)
            {
                while (temp != NULL && !strcmp(*temp->data, "!!"))
                {
                    temp = temp->prev;
                }
            }
            if (temp == NULL)
            {
                continue;
            }

            deepCopyArgv(temp->data, argv);
        }

        // Handle change prompt
        if (flagCanEnter == 1 && i == 3 && !strcmp(argv[0], "prompt") && !strcmp(argv[1], "="))
        {
            strcpy(prompt, argv[2]);
            strcat(prompt, ": ");
            continue;
        }

        /* Does command line end with & */
        if (flagCanEnter == 1 && !haveJobFlag && !strcmp(argv[i - 1], "&"))
        {
            amper = 1;
            argv[i - 1] = NULL;
            haveJobFlag = 1;
        }
        else
        {
            amper = 0;
        }

        // 
        if (flagCanEnter == 1 && !haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">"))
        {
            redirect = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            redirect = 0;

        // handle change dir
        if (flagCanEnter == 1 && i == 2 && !strcmp(argv[0], "cd"))

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

        // handle Redirrect strerr
        if (flagCanEnter == 1 && !haveJobFlag && i > 1 && !strcmp(argv[i - 2], "2>"))
        {
            err = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            err = 0;

        // Handle the read case
        if (flagCanEnter == 1 && !haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">>"))
        {

            redirect = 1;
            append = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            append = 0;

        if (flagCanEnter == 1 && !haveJobFlag && i > 1 && !strcmp(argv[0], "echo"))
        {
            if (!strcmp(argv[1], "$?"))
            {
                int counter = 0;
                char *ch = (char *)malloc(sizeof(char));
                while (current->prev->prev != NULL && current->prev->prev->data[counter] != NULL)
                {
                    ch = strcat(ch, strcat(current->prev->prev->data[counter], " "));
                    counter++;
                }

                // printf("command: %s\n", ch);
                // status = WEXITSTATUS(system(ch));
                printf("status: %d\n", status);
                continue;
            }
            else
            {
                changeVarsToData(argv);
                printArgv(argv);
                continue;
            }
        }

        if (flag)
        {
            argv[i - 2] = NULL;
        }

        /* for commands not part of the shell command language */
        pid_t pid = fork();

        if (pid == 0)
        {
            // If we seen if before & we got a ragular commend, check that you have seen all the necessary syntax
            if (flagSeenIf == 1 && flagIsStream == 0 && flagSeenThen == 0)
            {
                flagSeenIf = 0;
                flagSeenThen = 0;
                flagDoThen = -1;
                flagSeenElse = 0;
                printf("syntax error near unexpected token `fi'\n");
                fflush(stdout);
                continue;
            }

            /* redirection of IO ? */
            if (redirect)
            {
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
                fd = creat(outfile, 0660);
                close(STDERR_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            if (flagSeenIf == 0 || (flagSeenIf == 1 && flagSeenThen == 0) ||
                (flagIsStream == 0 && ((flagDoThen == 1 && flagSeenThen == 1 && flagSeenElse == 0) || (flagDoThen == 0 && flagSeenElse == 1))))
            {
                // printf("flagSeenIf= %d, flagIsStream= %d, flagDoThen= %d, flagSeenThen= %d, flagSeenElse= %d",flagSeenIf,flagIsStream, flagDoThen, flagSeenThen, flagSeenElse);
                // fflush(stdout);
                execvp(argv[0], argv);
            }
        }

        /* parent continues here */
        if (amper == 0)
        {
            retid = wait(&status);
            if (flagSeenIf == 1 && flagDoThen == -1 && flagSeenThen == 0)
            {
                flagDoThen = 1 - WEXITSTATUS(status);
            }
        }
    }
}