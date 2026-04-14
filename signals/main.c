#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FILE "counter.txt"
#define BUFFER 100
#define SLEEP_TIME 1

typedef int global_counter;
global_counter sigint_counter = 0;
global_counter sigquit_counter = 0;

void sigint_hadler(int sig)
{
    ++sigint_counter;
}

void sigquit_handler(int sig)
{
    ++sigquit_counter;
}

int main()
{
    int fd = open(FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
    close(fd);

    signal(SIGINT, sigint_hadler);
    signal(SIGQUIT, sigquit_handler);

    int counter = 1;
    int sigint_recieved_times = 0;

    while(1)
    {
        int fd = open(FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
        if(fd == -1)
        {
            perror(FILE);
            return -1;
        }

        char counter_str[BUFFER];
        sprintf(counter_str, "%d\n", counter);
        write(fd, counter_str, strlen(counter_str));

        while(sigquit_counter != 0)
        {
            write(fd, "Signal SIGQUIT recieved\n", strlen("Signal SIGQUIT recieved\n"));
            --sigquit_counter;
        }

        while(sigint_counter != 0)
        {
            write(fd, "Signal SIGINT recieved\n", strlen("Signal SIGINT recieved\n"));
            ++sigint_recieved_times;
            --sigint_counter;
            if(sigint_recieved_times >= 3)
            {
                close(fd);
                exit(EXIT_SUCCESS);
            }
        }

        ++counter;
        close(fd);

        sleep(SLEEP_TIME);
        
    }

    return 0;
}
