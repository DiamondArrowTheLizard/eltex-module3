#include "executable.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024

int main()
{
    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];

    while (1)
    {
        fprintf(stdout, "> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strlen(input) == 0)
            continue;

        strncpy(input_copy, input, sizeof(input_copy) - 1);
        input_copy[sizeof(input_copy) - 1] = '\0';

        executable* ex = executable_init();

        int status = executable_form(ex, input_copy);

        if (status == EXECUTABLE_OK)
        {
            int pid = fork();
            if(pid == 0)
            {
                executable_exec(ex);
                fprintf(stderr, "Error: execvp failed\n");
            } else {
                wait(NULL);
            }
        }
        else
        {
            fprintf(stderr, "Bad command format\n");
        }

        executable_destroy(ex);
    }

    return 0;
}
