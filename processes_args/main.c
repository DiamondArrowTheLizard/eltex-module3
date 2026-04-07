#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "process_arg.h"

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        fprintf(stderr, "Error: no arguments given.\n");
        return -1;
    }

    pid_t ppid = getpid();
    fprintf(stdout, "PPID: %d\n", ppid);

    int n = argc - 1;               
    int split = n / 2;              
    pid_t pid = fork();

    if(pid == -1)
    {
        perror("fork");
        return -1;
    }

    if(pid == 0)  
    {
        for(int i = split; i < n; i++)
            process_arg(argv[i + 1]);
        exit(0);
    }
    else          
    {
        for(int i = 0; i < split; i++)
            process_arg(argv[i + 1]);
        wait(NULL);
    }

    return 0;
}
