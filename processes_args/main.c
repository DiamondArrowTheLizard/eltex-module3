#include <stdio.h>
#include "process_arg.h"

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        fprintf(stderr, "Error: no arguments given.\n");
        return -1;
    }

    for(int i = 0; i < argc; i++)
    {
        process_arg(argv[i]);
    }

    return 0;
}
