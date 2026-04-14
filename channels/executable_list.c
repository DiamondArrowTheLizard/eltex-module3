#include "executable_list.h"
#include "executable.h"
#include "string_array.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

node* node_init(executable* ex, node* parent)
{
    node* n = (node*)malloc(sizeof(*n));
    n->ex = ex;
    n->parent = parent;
    n->child = NULL;

    return n;
}

void node_destroy(node* node)
{
    executable_destroy(node->ex);

    node->child = NULL;
    node->parent = NULL;

    free(node);
    node = NULL;
}



exec_list* exec_list_init()
{
    exec_list* list = (exec_list*)malloc(sizeof(*list));
    list->head = NULL;

    return list;
}

void exec_list_destroy(exec_list* list)
{
    node* node = list->head;    
    while(list->head != NULL)
    {
        list->head = list->head->child;

        node_destroy(node);
        node = list->head;
    }

    if(list->head != NULL)
    {
        node_destroy(list->head);
        list->head = NULL;
    }

    free(list);
    list = NULL;
}

int exec_list_append(exec_list* list, executable* ex)
{
    if(list == NULL) return EXEC_LIST_BAD;
    if(list->head == NULL)
    {
        list->head = node_init(ex, NULL);
        return EXEC_LIST_OK;
    }

    node* n = list->head;
    while(n->child != NULL)
    {
        n = n->child;
    }
    n->child = node_init(ex, n);

    return EXEC_LIST_OK;
}

int exec_list_form(exec_list* list, char* shell_string)
{
    if(list == NULL || shell_string == NULL) return EXEC_LIST_BAD;
    char delimeter[4] = "|";

    char* tok = strtok(shell_string, delimeter);
    if(tok == NULL) return EXEC_LIST_BAD;

    string_array* strings = string_array_init();
    if(string_array_append(strings, tok) == 0)
    {
        string_array_destroy(strings);
        return EXEC_LIST_BAD;
    }

    tok = strtok(NULL, delimeter);
    while(tok != NULL)
    {
        if(string_array_append(strings, tok) == 0)
        {
            string_array_destroy(strings);
            return EXEC_LIST_BAD;
        }
        tok = strtok(NULL, delimeter);
    }

    for(size_t i = 0; i < strings->count; i++)
    {
        executable* ex = executable_init();
        if(executable_form(ex, strings->strings[i]) == EXECUTABLE_OK)
        {
            exec_list_append(list, ex);

        } else {
            executable_destroy(ex);
            string_array_destroy(strings);
            return EXEC_LIST_BAD;
        }
    }

    string_array_destroy(strings);
    return EXEC_LIST_OK;
}

void exec_list_exec(exec_list* list)
{
    if(list == NULL || list->head == NULL) 
    {
        return;
    }

    node* current = list->head;
    int prev_pipe_read = -1;
    pid_t pid;

    while(current != NULL)
    {
        int pipe_fds[2] = {-1, -1};
        int is_first = (current->parent == NULL);
        int is_last = (current->child == NULL);

        if(!is_last)
        {
            if(pipe(pipe_fds) == -1)
            {
                perror("pipe");
                return;
            }
        }

        pid = fork();
        if(pid == -1)
        {
            perror("fork");
            return;
        }

        switch(pid)
        {
            case 0:
                if(!is_first)
                {
                    if(dup2(prev_pipe_read, STDIN_FILENO) == -1)
                    {
                        perror("dup2 Child process (stdin)");
                        return;
                    }
                }
                if(!is_last)
                {
                    if(dup2(pipe_fds[1], STDOUT_FILENO) == -1)
                    {
                        perror("dup2 Child process (stdout)");
                        return;
                    }
                }
                if(prev_pipe_read != -1) close(prev_pipe_read);
                if(!is_last)
                {
                    close(pipe_fds[0]);
                    close(pipe_fds[1]);
                }
                executable_exec(current->ex);

                return;
                break;

            default:
                if(prev_pipe_read != -1) close(prev_pipe_read);
                if(!is_last)
                {
                    close(pipe_fds[1]);
                    prev_pipe_read = pipe_fds[0];
                } else {
                    prev_pipe_read = -1;
                }
                current = current->child;
        }
    }

    while(wait(NULL) > 0)
    {

    }

}

void exec_list_print(FILE* stream, exec_list* list)
{
    if(list == NULL) return;
    for(node* n = list->head; n != NULL; n = n->child)
    {
        if(n->parent != NULL) fprintf(stream, "Parent: %s\n", n->parent->ex->name);
        executable_print(stream, n->ex);
        if(n->child != NULL) fprintf(stream, "Child: %s\n", n->child->ex->name);
    }
}
