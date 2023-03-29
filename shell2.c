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

int flagSeenIf = 0, flagSeenThen = 0, flagDoThen = 0, flagSeenFi = 0;
char fullIfCommend[1024];

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
    int i, fd, amper, redirect, err, append, retid, status, flag, haveJobFlag;
    char *argv[128];

    int pipesNum = 0;

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

        if (strchr(command, '|'))
        {
            current->data[0] = (char *)malloc(strlen(command) + 1);
            strcpy(current->data[0], command);
            current->next = (pnode)malloc(sizeof(node));
            current->next->prev = current;
            current = current->next;
            handlePipes(argv, command);
            continue;
        }
        // printf("%s\n", command);
        if (!strcmp(command, "\033[A") || !strcmp(command, "\033[B"))
        {
            while (1)
            {
                if (!strcmp(command, "\033[A"))
                {
                    printCmd(current->prev->data);
                    char *tmp = merge(current->prev->data);
                    // printf("tmp: %s\n", tmp);
                    strcpy(command, tmp);

                    inHistory = 1;
                    if (current->prev)
                    {
                        current = current->prev;
                    }
                }
                else // down
                {
                    printCmd(current->next->data);
                    char *tmp = merge(current->next->data);
                    // printf("tmp: %s\n", tmp);
                    strcpy(command, tmp);
                    inHistory = 1;
                    if (current->next)
                    {
                        current = current->next;
                    }
                }
                break;
            }

            // continue;
        }

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
            // Copying all the commend parts together + getting all the if requierments from the user
            memset(fullIfCommend, 0, strlen(fullIfCommend));
            int i;
            for (i = 0; i < 128; i++)
            {
                if (argv[i] != NULL)
                {
                    strcat(fullIfCommend, argv[i]);
                    fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';
                    strcat(fullIfCommend, " ");
                }
                else
                {
                    break;
                }
            }
            strcat(fullIfCommend, "; ");

            memset(command, 0, strlen(command));
            fgets(command, 1024, stdin);
            if (!strcmp(command, "then\n"))
            {
                strcat(fullIfCommend, "then ");
                int flagElse = 0;
                while (fgets(command, 1024, stdin) && strcmp(command, "fi\n"))
                {
                    if (!strcmp(command, "else\n"))
                    {
                        strcat(fullIfCommend, " else ");
                        flagElse = 1;
                    }
                    else
                    {
                        strcat(fullIfCommend, command);
                        fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';
                        strcat(fullIfCommend, ";");
                    }
                    i++;
                    memset(command, 0, strlen(command));
                }
                strcat(fullIfCommend, " ");
                strcat(fullIfCommend, command);
                fullIfCommend[strcspn(fullIfCommend, "\n")] = '\0';
            }

            // Saving the proper if commend in our struct for future usage
            memset(argv[0], 0, strlen(argv[0]));
            strcpy(argv[0], "if");
            strcpy(argv[1], fullIfCommend);
            argv[2] = NULL;

            system(fullIfCommend);
        }
        if (!inHistory)
        {

            deepCopyArgv(argv, current->data);
            current->next = (pnode)malloc(sizeof(node));
            current->next->prev = current;
            current = current->next;
        }

        inHistory = 0;

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
            save_variable(strcat(str2, argv[1]), str);
        }

        if (i == 3 && argv[0][0] == '$' && !strcmp(argv[1], "="))
        {
            save_variable(argv[0], argv[2]);
        }

        if (!strcmp(argv[0], "!!"))
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
            if (!strcmp(temp->data[0], "if"))
            {
                system(temp->data[1]);
            }
            else
            {
                deepCopyArgv(temp->data, argv);
            }
        }

        if (i == 3 && !strcmp(argv[0], "prompt") && !strcmp(argv[1], "="))
        {
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
            redirect = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            redirect = 0;

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

        if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], "2>"))
        {
            err = 1;
            outfile = argv[i - 1];
            flag = 1;
            haveJobFlag = 1;
        }
        else
            err = 0;

        if (!haveJobFlag && i > 1 && !strcmp(argv[i - 2], ">>"))
        {

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
        if (fork() == 0)
        {
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
            execvp(argv[0], argv);
        }
        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
    }
}