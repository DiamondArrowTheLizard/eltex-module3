#include "executable.h"
#include "string_array.h"
#include <string.h>
#include <unistd.h>

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

void executable_exec(executable* ex)
{
    char* name = ex->name;
    char** args = (ex->args->strings == NULL) ? NULL : ex->args->strings;
    if(execvp(name, args) == -1)
    {
        perror(name);
    }
}

void executable_print(FILE* stream, executable* ex)
{
    fprintf(stream, "Executable: %s\n", ex->name);
    fprintf(stream, "Args:\n");
    string_array_print_all(ex->args);
}
