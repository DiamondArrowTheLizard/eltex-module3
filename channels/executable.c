#include "executable.h"
#include "string_array.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define STREAM_NONE -1
#define STREAM_IN 0
#define STREAM_OUT 1
#define STREAM_OUT_TRUNC 2

executable* executable_init()
{
    executable* ex = (executable*)malloc(sizeof(*ex));

    ex->args = string_array_init();
    return ex;
}

void executable_destroy(executable* ex)
{
    if(ex == NULL) return;

    string_array_destroy(ex->args); 

    free(ex);
    ex = NULL;
}

int executable_form(executable* ex, char* ex_string)
{
    if(ex == NULL || ex_string == NULL) return EXECUTABLE_BAD;

    char delimeter[4] = " ";
    char* token = strtok(ex_string, delimeter);

    if(token == NULL) return EXECUTABLE_BAD;

    strncpy(ex->name, token, EXECUTABLE_NAME_MAX_SIZE);

    if(string_array_append(ex->args, ex->name) == 0)
        return EXECUTABLE_BAD;

    token = strtok(NULL, delimeter);
    while(token != NULL)
    {
        if(string_array_append(ex->args, token) == 0)
            return EXECUTABLE_BAD;
        token = strtok(NULL, delimeter);
    }

    if(string_array_append(ex->args, NULL) == 0)
        return EXECUTABLE_BAD;

    return EXECUTABLE_OK;
}

static int check_stream(string_array* args)
{
    for(size_t i = 0; i < args->count; i++)
    {
        if(args->strings[i] == NULL) continue;
        if(strcmp(args->strings[i], "<") == 0) return STREAM_IN;
        if(strcmp(args->strings[i], ">>") == 0) return STREAM_OUT;
        if(strcmp(args->strings[i], ">") == 0) return STREAM_OUT_TRUNC;
    }

    return STREAM_NONE;
}

static void exec_no_stream(executable* ex)
{
    if(execvp(ex->name, ex->args->strings) == -1)
    {
        perror(ex->name);
    }
}

static void exec_redir_input(executable* ex)
{
    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
        return;
    }
    
    switch(pid)
    {
        case 0:
            char* file = ex->args->strings[ex->args->count-2];
            int fd0 = open(file, O_RDONLY);
            if(fd0 == -1)
            {
                perror("File read error");
                return;
            }
            dup2(fd0, STDIN_FILENO);
            close(fd0);

            string_array_remove(ex->args, ex->args->count-2);
            string_array_remove(ex->args, ex->args->count-2);

            exec_no_stream(ex);    
            break;

        default:
            wait(NULL);
    }


}

static void exec_to_out(executable* ex)
{
    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
        return;
    }
    
    switch(pid)
    {
        case 0:
            char* file = ex->args->strings[ex->args->count-2];
            int fd0 = open(file, O_RDWR | O_CREAT | O_APPEND);
            if(fd0 == -1)
            {
                perror("File read error");
                return;
            }
            dup2(fd0, STDOUT_FILENO);
            close(fd0);

            string_array_remove(ex->args, ex->args->count-2);
            string_array_remove(ex->args, ex->args->count-2);

            exec_no_stream(ex);    
            break;

        default:
            wait(NULL);
    }

}

static void exec_to_out_trunc(executable* ex)
{

    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
        return;
    }
    
    switch(pid)
    {
        case 0:
            char* file = ex->args->strings[ex->args->count-2];
            int fd0 = open(file, O_RDWR | O_CREAT | O_TRUNC);
            if(fd0 == -1)
            {
                perror("File read error");
                return;
            }
            dup2(fd0, STDOUT_FILENO);
            close(fd0);

            string_array_remove(ex->args, ex->args->count-2);
            string_array_remove(ex->args, ex->args->count-2);

            exec_no_stream(ex);    
            break;

        default:
            wait(NULL);
    }

}

void executable_exec(executable* ex)
{
    int stream = check_stream(ex->args);
    switch(stream)
    {
        case STREAM_NONE:
            exec_no_stream(ex);
            break;

        case STREAM_IN:
            exec_redir_input(ex);
            break;

        case STREAM_OUT:
            exec_to_out(ex);
            break;

        case STREAM_OUT_TRUNC:
            exec_to_out_trunc(ex);
            break;

        default:
            fprintf(stderr, "Error: unknown stream type.\n");
            break;
    }
}

void executable_print(FILE* stream, executable* ex)
{
    fprintf(stream, "Executable: %s\n", ex->name);
    fprintf(stream, "Args:\n");
    string_array_print_all(ex->args);
}
