#include "executable.h"
#include "executable_list.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>

#define MAX_INPUT 1024

int main()
{
    char hostname[MAX_INPUT];
    gethostname(hostname, MAX_INPUT);


    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];

    while (1)
    {
        char* working_directory = NULL;
        working_directory = getcwd(working_directory, MAX_INPUT);
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL) 
        {
            fprintf(stdout, "%s@%s %s> ", pw->pw_name, hostname, working_directory);

        } else {
            fprintf(stdout, "%d@%s %s> ", getuid(), hostname, working_directory);
        }

        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            return 0;

        if (strlen(input) == 0)
            continue;

        strncpy(input_copy, input, sizeof(input_copy) - 1);
        input_copy[sizeof(input_copy) - 1] = '\0';

        exec_list* list = exec_list_init();

        int status = exec_list_form(list, input_copy);

        if (status == EXECUTABLE_OK)
        {
            int pid = fork();
            if(pid == 0)
            {
                exec_list_exec(list);
                exec_list_destroy(list);
                _exit(0);
            } else {
                wait(NULL);
            }
        }
        else
        {
            fprintf(stderr, "Bad command format\n");
        }

        exec_list_destroy(list);
    }

    return 0;
}
