// David Emanuel 203965520

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define PROMPT '$'
#define MAX 100
#define DONE 0
#define FORE 1
#define BACK 2


typedef struct job{
    char command[MAX];
    int status;
    pid_t pid;
}job;

size_t jid = 0;     // id for current job
job jobsList[MAX];
char pwd[MAX];      // pre working directory
char cwd[MAX];      // current workig directory

/**
 * @brief updates background jobs to done if finished
 * 
 * @param j pointer to job for update
 * @return int - done (0) or running (!= 0)
 */
int updateStatus(job* j)
{
    int ret;
    waitpid(j->pid, &ret, WNOHANG);
    if (!ret)
        j->status = DONE;
    return ret;
}

/**
 * @brief builtin command - print all background running jobs
 * 
 */
void jobs()
{
    job* currentJob = jobsList + jid;
    job* jobIter = jobsList;
    while (jobIter <= currentJob)
    {
        if (jobIter->status == BACK && updateStatus(jobIter))
            printf("%s\n", jobIter->command);
        jobIter++;
    }
    currentJob->status = DONE;
}

/**
 * @brief builtin command - print all jobs, with indication for RUNNING/DONE
 * 
 */
void history()
{
    job* currentJob = jobsList + jid;
    job* jobIter = jobsList;
    while (jobIter <= currentJob)
    {
        if(jobIter->status == BACK)
            updateStatus(jobIter);
        printf("%s %s\n", jobIter->command, jobIter->status == DONE ? "DONE" : "RUNNING");
        jobIter++;
    }
    currentJob->status = DONE;
}

/**
 * @brief helper function for cd()
 * 
 * @param dir dest directory
 * @return int success (0) or fail (-1)
 */
int myChdir(char* dir)
{
    int fail;
    if(dir[1] == '\0' || dir[1] == '/')
    {
        if(dir[0] == '~')
            fail = chdir(getenv("HOME"));
        else if(dir[0] == '-')
            fail = chdir(pwd);
        if(fail)
            return fail;
        if (dir[1] == '\0')
            return 0;
        dir += 2;
    }
    return chdir(dir);
}

/**
 * @brief builtin command - change direction
 * 
 * @param dir dest directory
 */
void cd(char* dir)
{
    job* currentJob = jobsList + jid;
    if (!dir)
    {
        printf("An error occurred\n");
        currentJob->status = DONE;
        return;
    }
    if(myChdir(dir))
    {
        printf("chdir failed\n");
        chdir(cwd);
    }
    else
    {
        strcpy(pwd, cwd);
        getcwd(cwd, MAX);
    }
    currentJob->status = DONE;
}

/**
 * @brief execute not builtin commands
 * 
 * @param args command parsed to tokens
 */
void execJob(char** args)
{
    job* currentJob = jobsList + jid;
    pid_t pid = fork();
    // fork fail
    if(pid < 0)
        printf("fork failed\n");
    // child process
    else if (pid ==0)
    {
        if (execvp(args[0], args) == -1)
        {
            printf("exec failed\n");
            exit(-1);
        }
    }
    // parent process
    else
    {
        if (currentJob->status == FORE)
        {
            wait(NULL);
            currentJob->status = DONE;
        }
        else
            currentJob->pid = pid;
    }
}

/**
 * @brief controller to execute the commands
 * 
 * @param buffer input command
 */
void exec(char* buffer)
{
    // insert current job to jobs list
    job* currentJob = jobsList + jid;
    strcpy(currentJob->command, buffer);
    
    // set argv
    char* argv[MAX/2];
    int argc = 0;
    for(argv[argc] = strtok(buffer, " "); argv[++argc] = strtok(NULL, " "););
    
    // check '&' - fore/back
    if(strcmp(argv[argc-1], "&") == 0)
    {
        argv[--argc] = NULL;
        currentJob->command[strlen(currentJob->command)-2] = '\0';
        currentJob->status = BACK;
    }
    else
        currentJob->status = FORE;
    
    // switch command
    if(strcmp(currentJob->command, "exit") == 0)
        exit(0);
    else if(strcmp(currentJob->command, "jobs") == 0)
        jobs();
    else if(strcmp(currentJob->command, "history") == 0)
        history();
    else if(strcmp(argv[0], "cd") == 0)
        if(argc > 2)
            printf("Too many argument\n");
        else
            cd(argv[1]);
    else
        execJob(argv);
}

/**
 * @brief initialization and shell I/O loop
 * 
 * @return int
 */
int main()
{
    // init cwd - Current Working Directory
    getcwd(cwd, MAX);

    // shell - I/O loop
    char buffer[MAX];
    while(1)
    {
        printf("%c ", PROMPT);
        fflush(stdout);
        scanf(" %[^\n]", buffer);
        fflush(stdin);
        exec(buffer);
        jid++;
    }
}
