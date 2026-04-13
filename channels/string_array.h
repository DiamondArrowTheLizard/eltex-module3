#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stddef.h>

typedef struct string_array
{
    char** strings;
    size_t count;
    size_t capacity;

} string_array;

string_array* string_array_init();
void string_array_destroy(string_array* array);

int string_array_append(string_array* array, char* string);
int string_array_remove(string_array* array, size_t index);

void string_array_print_all(string_array* array);
int string_array_print_at_index(string_array* array, size_t index);

#endif // !STRING_ARRAY_H
