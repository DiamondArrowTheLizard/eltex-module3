#include "process_arg.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void process_arg(char* arg, pid_t ppid)
{
    if(!isdigit(arg[0]))
    {
        fprintf(stdout, "String from PID %d [PPID: %d]: %s\n", getpid(), getppid(), arg);

        if(getpid() != ppid)
        {
            fprintf(stdout, "Killing process %d\n", getpid());
            kill(getpid(), SIGTERM);
        }
        return;
    }

    pid_t p = fork();
    if(p < 0)
    {
        fprintf(stderr, "Error: process forking failed.\n");
        return;
    }
    fprintf(stdout, "Forking child process %d from parent %d\n", getpid(), getppid());

    int is_float = 0;
    for(size_t i = 1; i < strlen(arg); i++)
    {
        if(arg[i] == '.')
        {
            is_float += 1;
            continue;
        }

        if(!isdigit(arg[i]))
        {
            fprintf(stdout, "String from PID %d [PPID: %d]: %s\n", getpid(), getppid(), arg);

            if(getpid() != ppid)
            {
                fprintf(stdout, "Killing process %d\n", getpid());
                kill(getpid(), SIGTERM);
            }
            return;
        }
    }

    if(is_float == 1)
    {
        double num = atof(arg);
        fprintf(stdout, "Double from PID %d [PPID: %d]: %lf %lf\n", getpid(), getppid(), num, num*2);

    } else if(is_float == 0) {

        int num = atoi(arg);
        fprintf(stdout, "Int from PID %d [PPID: %d]: %d %d\n", getpid(), getppid(), num, num*2);

    } else {

        fprintf(stdout, "String from PID %d [PPID: %d]: %s\n", getpid(), getppid(), arg);
    }

    if(getpid() != ppid)
    {
        fprintf(stdout, "Killing process %d\n", getpid());
        kill(getpid(), SIGTERM);
    }
}
