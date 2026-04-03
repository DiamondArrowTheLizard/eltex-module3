#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "process_arg.h"

int main(int argc, char** argv)
{
    pid_t ppid = getpid();
    fprintf(stdout, "PPID: %d\n", ppid);

    if(argc == 1)
    {
        fprintf(stderr, "Error: no arguments given.\n");
        return -1;
    }

    for(int i = 1; i < argc; i++)
    {
        process_arg(argv[i], ppid);
    }

    return 0;
}
