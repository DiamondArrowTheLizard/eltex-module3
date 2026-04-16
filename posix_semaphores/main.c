#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define SEM_NAME "/file_sem"
#define FILE_NAME "data.txt"
#define MAX_NUMBERS 200
#define MAX_LINE 10000

typedef int flag_t;

flag_t sigint_recieved = 0;

void handle_sigint(int sig)
{
    sig = sig;
    ++sigint_recieved;
}

void handle_child(sem_t* sem) 
{
    FILE* file = fopen(FILE_NAME, "a+");
    if (!file) 
    {
        perror("child fopen");
        exit(1);
    }
    long pos = 0;
    char line[MAX_LINE];
    int running = 1;

    while (running) 
    {
        sem_wait(sem);
        fseek(file, 0, SEEK_END);
        long end = ftell(file);
        if (end > pos) 
        {
            fseek(file, pos, SEEK_SET);
            while (fgets(line, sizeof(line), file)) 
            {
                if (strcmp(line, "END\n") == 0) 
                {
                    running = 0;
                    break;
                }
                int num, first = 1, min, max;
                char *ptr = line;
                while (sscanf(ptr, "%d", &num) == 1) 
                {
                    if (first) 
                    {
                        min = max = num;
                        first = 0;

                    } else {

                        if (num < min) min = num;
                        if (num > max) max = num;
                    }
                    while (*ptr && *ptr != ' ') ptr++;
                    if (*ptr == ' ') ptr++;
                }
                if (!first) 
                {
                    fprintf(stdout, "Min: %d, Max: %d\n", min, max);
                }
            }
            pos = ftell(file);
        }
        sem_post(sem);
        usleep(500000);
    }
    fclose(file);
    sem_close(sem);
    exit(0);
}

void handle_parent(sem_t* sem)
{
    srand(time(NULL));
    FILE *file = fopen(FILE_NAME, "w");
    if (file) fclose(file);
    usleep(100000);

    while (sigint_recieved == 0) 
    {
        int num_count = rand() % MAX_NUMBERS + 1;
        char line[MAX_LINE] = "";
        for (int j = 0; j < num_count; j++) 
        {
            char buf[16];
            sprintf(buf, "%d ", rand() % 100);
            strncat(line, buf, strlen(buf));
        }
        line[strlen(line) - 1] = '\n';
        sem_wait(sem);
        file = fopen(FILE_NAME, "a");
        if (file) 
        {
            fputs(line, file);
            fclose(file);
        }
        sem_post(sem);
        usleep(500000);
    }

    sem_wait(sem);
    file = fopen(FILE_NAME, "a");
    if (file) 
    {
        fputs("END\n", file);
        fclose(file);
    }
    sem_post(sem);

    wait(NULL);
    sem_close(sem);
    sem_unlink(SEM_NAME);
}

int main() {
    signal(SIGINT, handle_sigint);
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) 
    {
        perror("sem_open");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) 
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0) 
    {
        handle_child(sem);

    } else {

        handle_parent(sem);
    }

    return 0;
}
