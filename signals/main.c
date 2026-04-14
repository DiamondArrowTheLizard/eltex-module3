#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FILE "counter.txt"
#define BUFFER 100
#define SLEEP_TIME 1

void sigint_hadler(int sig)
{
    static int sig_count = 0;
    int fd = open(FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
    if(fd == -1)
    {
        perror(FILE);
        return;
    }
    write(fd, "Recieved SIGINT\n", strlen("Recieved SIGINT\n"));
    close(fd);

    if(sig_count < 2)
    {
        ++sig_count;
    }
    else {
        exit(EXIT_SUCCESS);
    }
}

void sigquit_handler(int sig)
{
    int fd = open(FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
    if(fd == -1)
    {
        perror(FILE);
        return;
    }
    write(fd, "Recieved SIGQUIT\n", strlen("Recieved SIGQUIT\n"));
    close(fd);
}

int main()
{
    int counter = 1;
    signal(SIGINT, sigint_hadler);
    signal(SIGQUIT, sigquit_handler);
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
        close(fd);

        ++counter;

        sleep(SLEEP_TIME);
        
    }

    return 0;
}
