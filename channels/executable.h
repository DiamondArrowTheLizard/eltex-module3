#ifndef EXECUTABLE_H
#define EXECUTABLE_H
#include "string_array.h"
#include <stdio.h>
#include <stdlib.h>

#define EXECUTABLE_NAME_MAX_SIZE 100

#define EXECUTABLE_OK 0
#define EXECUTABLE_BAD -1

typedef struct shell_executable_process
{
    char name[EXECUTABLE_NAME_MAX_SIZE];
    string_array* args;

} executable;

executable* executable_init();
void executable_destroy(executable* ex);

int executable_form(executable* ex, char* ex_string);
void executable_exec(executable* ex);

void executable_print(FILE* stream, executable* ex);

#endif // !EXECUTABLE_H
