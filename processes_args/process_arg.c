#include "process_arg.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void process_arg(char* arg)
{
    if(!isdigit(arg[0]))
    {
        fprintf(stdout, "String: %s\n", arg);
        return;
    }

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
            fprintf(stdout, "String: %s\n", arg);
            return;
        }
    }

    if(is_float == 1)
    {
        double num = atof(arg);
        fprintf(stdout, "Double: %lf %lf\n", num, num*2);

    } else if(is_float == 0) {

        int num = atoi(arg);
        fprintf(stdout, "Int: %d %d\n", num, num*2);

    } else {

        fprintf(stdout, "String: %s\n", arg);
    }

}
