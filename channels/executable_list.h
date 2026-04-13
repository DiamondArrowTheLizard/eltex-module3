#ifndef EXECUTABLE_LIST_H
#define EXECUTABLE_LIST_H

#include "executable.h"

#define EXEC_LIST_HEAD NULL

#define EXEC_LIST_OK 0
#define EXEC_LIST_BAD -1

typedef struct executable_doubly_linked_list_node
{
    executable* ex;
    struct executable_doubly_linked_list_node* child;
    struct executable_doubly_linked_list_node* parent;

} node;

node* node_init(executable* ex, node* parent);
void node_destroy(node* node);

typedef struct executable_doubly_linked_list
{
    node* head;
} exec_list;

exec_list* exec_list_init();
void exec_list_destroy(exec_list* list);

int exec_list_append(exec_list* list, executable* ex);
int exec_list_form(exec_list* list, char* shell_string);
void exec_list_exec(exec_list* list);

void exec_list_print(FILE* stream, exec_list* list);

#endif // !EXECUTABLE_LIST_H

