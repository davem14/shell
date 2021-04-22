// David Emanuel 203965520

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT '$'
//#define BACK '&'
#define MAX 100
#define DONE 0
#define FORE 1
#define BACK 2


typedef struct job{
    char command[MAX];
    int status;
}job;

size_t jid = 0;
job jobsList[MAX];

char* status(int s)
{
    switch (s)
    {
    case DONE:
        return "DONE";
        break;
    case FORE:
    case BACK:
        return "RUNNING";
        break;
    default:
        return "";
        break;
    }
}

void jobs(job* jobEntry)
{
    job* jobIter = jobsList;
    while (jobIter <= jobEntry)
    {
        if (jobIter->status == BACK)
            printf("%s %s\n", jobIter->command, status(BACK));
        jobIter++;
    }
    jobEntry->status = DONE;
}

void history(job* jobEntry)
{
    job* jobIter = jobsList;
    while (jobIter <= jobEntry)
    {
        printf("%s %s\n", jobIter->command, status(jobIter->status));
        jobIter++;
    }
    jobEntry->status = DONE;
}

void cd(job* jobEntry, char* arg)
{
    printf("cd command\n");
    jobEntry->status = DONE;
}

void execJob(job* jobEntry, char** argv, int fore)
{
    int stat, waited;
    //pid_t pid = fork();
    switch (fork())
    {
    // fork fail
    case -1:
        printf("fork failed\n");
        break;
    // son process
    case 0:
        if (execvp(argv[0], argv) == -1)
        {
            printf("exec failed\n");
            exit(-1);
        }
        break;
    // father process
    default:
        if (fore)
            waited = wait(&stat);
        jobEntry->status = DONE;
        break;
    }
}

/**
 * @brief execute command
 * 
 * @param argc amount of arguments
 * @param argv arguments values
 */
void execute(int argc, char* argv[])
{
    // chaeck '&' - fore/back
    int fore = strcmp(argv[argc - 1], "&");
    char* command = argv[0];

    // set jobEntry
    job* jobEntry = &jobsList[jid];
    strcpy(jobEntry->command, command);
    jobEntry->status = fore ? FORE : BACK;
    jid++;

    // switch command
    if(strcmp(command, "exit") == 0)
        exit(0);
    else if(strcmp(command, "jobs") == 0)
        jobs(jobEntry);
    else if(strcmp(command, "history") == 0)
        history(jobEntry);
    else if(strcmp(command, "cd") == 0)
        if(argc > 2)
            printf("Too many argument");
        else
            cd(jobEntry, argv[1]);
    else
        execJob(jobEntry, argv, fore);
}

/**
 * @brief shell main - I/O
 * 
 * @return int - exit code
 */
int main()
{
    char buffer[MAX][MAX];
    char* argv[MAX];
    for(int i = 0; i < MAX; i++)
        argv[i] = buffer[i];
    while(1)
    {
        printf("%c ", PROMPT);
        fflush(stdout);
        int argc = 0;
        char c = 'c';
        while(c != '\n')
        {
            if (scanf("%[^ \"\n]", argv[argc]))
                argc++;
            scanf("%c", &c);
            if (c == '\"')
            {
                scanf("%[^\"]", argv[argc]);
                scanf("%c", &c);
                argc++;
            }
        }
        argv[argc] = NULL;
        if(argc > 0)
            execute(argc, argv);/*
        for(;argc-->0;)
            printf("|%s|\n",argv[argc]);*/
        fflush(stdin);
    }
}