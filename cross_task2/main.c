#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX_DRIVERS 100
#define BUF_SIZE 256

#define DRIVER_AVAILABLE 0
#define DRIVER_BUSY 1

#define CMD_TASK "TASK "
#define CMD_STATUS "STATUS"
#define CMD_QUIT "QUIT"
#define RESP_OK "OK\n"
#define RESP_ERROR "ERROR\n"

#define CMD_CREATE_DRIVER "create_driver"
#define CMD_SEND_TASK "send_task"
#define CMD_GET_STATUS "get_status"
#define CMD_GET_DRIVERS "get_drivers"
#define CMD_EXIT "exit"

typedef struct {
    pid_t pid;
    int fd;
} Driver;

Driver drivers[MAX_DRIVERS];
int driver_count = 0;
int listen_fd = -1;
uint16_t listen_port = 0;

void driver_process(int fd);
int  read_line(int fd, char *buf, size_t size);
int  write_all(int fd, const char *buf, size_t len);
void send_to_driver(int fd, const char *msg);
void cleanup_drivers();

void driver_process(int fd) {
    char buf[BUF_SIZE];
    int state = DRIVER_AVAILABLE;
    int remaining = 0;
    int timer_fd = -1;

    while (1) {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(fd, &readfs);
        int maxfd = fd;
        if (timer_fd != -1) {
            FD_SET(timer_fd, &readfs);
            if (timer_fd > maxfd) maxfd = timer_fd;
        }

        int ret = select(maxfd + 1, &readfs, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (FD_ISSET(fd, &readfs)) {
            int n = read_line(fd, buf, sizeof(buf));
            if (n <= 0) break;

            buf[strcspn(buf, "\n")] = '\0';

            if (strncmp(buf, CMD_TASK, sizeof(CMD_TASK) - 1) == 0) {
                int task_time = atoi(buf + sizeof(CMD_TASK) - 1);
                if (task_time <= 0) task_time = 1;

                if (state == DRIVER_AVAILABLE) {
                    state = DRIVER_BUSY;
                    remaining = task_time;

                    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
                    if (timer_fd == -1) {
                        write_all(fd, RESP_ERROR, sizeof(RESP_ERROR) - 1);
                        state = DRIVER_AVAILABLE;
                        remaining = 0;
                    } else {
                        struct itimerspec its = {0};
                        its.it_value.tv_sec = task_time;
                        its.it_value.tv_nsec = 0;
                        if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
                            close(timer_fd);
                            timer_fd = -1;
                            state = DRIVER_AVAILABLE;
                            remaining = 0;
                            write_all(fd, RESP_ERROR, sizeof(RESP_ERROR) - 1);
                        } else {
                            write_all(fd, RESP_OK, sizeof(RESP_OK) - 1);
                        }
                    }
                } else {
                    char resp[64];
                    snprintf(resp, sizeof(resp), "Busy %d\n", remaining);
                    write_all(fd, resp, strlen(resp));
                }
            }
            else if (strcmp(buf, CMD_STATUS) == 0) {
                char resp[64];
                if (state == DRIVER_AVAILABLE)
                    snprintf(resp, sizeof(resp), "Available\n");
                else
                    snprintf(resp, sizeof(resp), "Busy %d\n", remaining);
                write_all(fd, resp, strlen(resp));
            }
            else if (strcmp(buf, CMD_QUIT) == 0) {
                break;
            }
        }

        if (timer_fd != -1 && FD_ISSET(timer_fd, &readfs)) {
            uint64_t expirations;
            if (read(timer_fd, &expirations, sizeof(expirations)) > 0) {
                state = DRIVER_AVAILABLE;
                remaining = 0;
                close(timer_fd);
                timer_fd = -1;
            }
        }
    }

    if (timer_fd != -1) close(timer_fd);
    close(fd);
    exit(0);
}

int read_line(int fd, char *buf, size_t size) {
    size_t i = 0;
    while (i < size - 1) {
        ssize_t n = read(fd, buf + i, 1);
        if (n <= 0) {
            if (i == 0) return n;
            break;
        }
        if (buf[i] == '\n') {
            i++;
            break;
        }
        i++;
    }
    buf[i] = '\0';
    return i;
}

int write_all(int fd, const char *buf, size_t len) {
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(fd, buf + written, len - written);
        if (n <= 0) return -1;
        written += n;
    }
    return 0;
}

void send_to_driver(int fd, const char *msg) {
    char packet[BUF_SIZE];
    snprintf(packet, sizeof(packet), "%s\n", msg);
    write_all(fd, packet, strlen(packet));
}

void cleanup_drivers() {
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].fd != -1) {
            send_to_driver(drivers[i].fd, CMD_QUIT);
            close(drivers[i].fd);
        }
    }
    if (listen_fd != -1) close(listen_fd);
    while (wait(NULL) > 0);
}

void sigint_handler(int sig) {
    (void)sig;
    fprintf(stdout, "\nShutting down...\n");
    fflush(stdout);
    cleanup_drivers();
    exit(EXIT_SUCCESS);
}

int main() {
    char cmdline[BUF_SIZE];
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, SIG_IGN);

    fprintf(stdout, "Taxi dispatch CLI. Commands:\n");
    fprintf(stdout, "  create_driver\n");
    fprintf(stdout, "  send_task <pid> <seconds>\n");
    fprintf(stdout, "  get_status <pid>\n");
    fprintf(stdout, "  get_drivers\n");
    fprintf(stdout, "  exit\n");

    while (1) {
        fprintf(stdout, "> ");
        fflush(stdout);
        if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
            break;

        cmdline[strcspn(cmdline, "\n")] = '\0';

        if (strcmp(cmdline, CMD_EXIT) == 0) {
            break;
        }
        else if (strcmp(cmdline, CMD_CREATE_DRIVER) == 0) {
            if (driver_count >= MAX_DRIVERS) {
                fprintf(stderr, "Too many drivers\n");
                continue;
            }

            if (listen_fd == -1) {
                listen_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (listen_fd == -1) {
                    perror("socket");
                    continue;
                }
                struct sockaddr_in addr = {0};
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                addr.sin_port = 0;
                if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
                    perror("bind");
                    close(listen_fd);
                    listen_fd = -1;
                    continue;
                }
                if (listen(listen_fd, 5) == -1) {
                    perror("listen");
                    close(listen_fd);
                    listen_fd = -1;
                    continue;
                }
                struct sockaddr_in bound_addr;
                socklen_t len = sizeof(bound_addr);
                if (getsockname(listen_fd, (struct sockaddr*)&bound_addr, &len) == -1) {
                    perror("getsockname");
                    close(listen_fd);
                    listen_fd = -1;
                    continue;
                }
                listen_port = ntohs(bound_addr.sin_port);
            }

            int port_pipe[2];
            if (pipe(port_pipe) == -1) {
                perror("pipe");
                continue;
            }

            char port_str[16];
            snprintf(port_str, sizeof(port_str), "%u", listen_port);
            write_all(port_pipe[1], port_str, strlen(port_str) + 1);
            close(port_pipe[1]);

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                close(port_pipe[0]);
                continue;
            }

            if (pid == 0) {
                close(port_pipe[1]);
                char port_buf[16];
                read_line(port_pipe[0], port_buf, sizeof(port_buf));
                close(port_pipe[0]);
                close(listen_fd);

                int driver_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (driver_fd == -1) exit(EXIT_FAILURE);

                struct sockaddr_in srv_addr = {0};
                srv_addr.sin_family = AF_INET;
                srv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                srv_addr.sin_port = htons(atoi(port_buf));
                if (connect(driver_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
                    close(driver_fd);
                    exit(EXIT_FAILURE);
                }

                driver_process(driver_fd);
                exit(EXIT_FAILURE);
            } else {
                close(port_pipe[0]);

                int new_fd = accept(listen_fd, NULL, NULL);
                if (new_fd == -1) {
                    perror("accept");
                    continue;
                }
                drivers[driver_count].pid = pid;
                drivers[driver_count].fd  = new_fd;
                driver_count++;
                fprintf(stdout, "Driver created, pid = %d\n", pid);
            }
        }
        else if (strncmp(cmdline, CMD_SEND_TASK " ", sizeof(CMD_SEND_TASK)) == 0) {
            int pid, seconds;
            if (sscanf(cmdline + sizeof(CMD_SEND_TASK), "%d %d", &pid, &seconds) != 2) {
                fprintf(stdout, "Usage: send_task <pid> <seconds>\n");
                continue;
            }
            int found = 0;
            for (int i = 0; i < driver_count; i++) {
                if (drivers[i].pid == pid && drivers[i].fd != -1) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), CMD_TASK "%d", seconds);
                    send_to_driver(drivers[i].fd, msg);

                    char resp[BUF_SIZE];
                    int n = read_line(drivers[i].fd, resp, sizeof(resp));
                    if (n > 0) {
                        resp[strcspn(resp, "\n")] = '\0';
                        fprintf(stdout, "Response from %d: %s\n", pid, resp);
                    } else {
                        fprintf(stdout, "Driver %d disconnected\n", pid);
                        close(drivers[i].fd);
                        drivers[i].fd = -1;
                    }
                    found = 1;
                    break;
                }
            }
            if (!found)
                fprintf(stderr, "Driver %d not found\n", pid);
        }
        else if (strncmp(cmdline, CMD_GET_STATUS " ", sizeof(CMD_GET_STATUS)) == 0) {
            int pid;
            if (sscanf(cmdline + sizeof(CMD_GET_STATUS), "%d", &pid) != 1) {
                fprintf(stdout, "Usage: get_status <pid>\n");
                continue;
            }
            int found = 0;
            for (int i = 0; i < driver_count; i++) {
                if (drivers[i].pid == pid && drivers[i].fd != -1) {
                    send_to_driver(drivers[i].fd, CMD_STATUS);
                    char resp[BUF_SIZE];
                    int n = read_line(drivers[i].fd, resp, sizeof(resp));
                    if (n > 0) {
                        resp[strcspn(resp, "\n")] = '\0';
                        fprintf(stdout, "Driver %d: %s\n", pid, resp);
                    } else {
                        fprintf(stdout, "Driver %d disconnected\n", pid);
                        close(drivers[i].fd);
                        drivers[i].fd = -1;
                    }
                    found = 1;
                    break;
                }
            }
            if (!found)
                fprintf(stderr, "Driver %d not found\n", pid);
        }
        else if (strcmp(cmdline, CMD_GET_DRIVERS) == 0) {
            if (driver_count == 0) {
                fprintf(stdout, "No drivers\n");
                continue;
            }
            for (int i = 0; i < driver_count; i++) {
                if (drivers[i].fd != -1) {
                    send_to_driver(drivers[i].fd, CMD_STATUS);
                    char resp[BUF_SIZE];
                    int n = read_line(drivers[i].fd, resp, sizeof(resp));
                    if (n > 0) {
                        resp[strcspn(resp, "\n")] = '\0';
                        fprintf(stdout, "pid %d: %s\n", drivers[i].pid, resp);
                    } else {
                        fprintf(stdout, "pid %d: disconnected\n", drivers[i].pid);
                        close(drivers[i].fd);
                        drivers[i].fd = -1;
                    }
                } else {
                    fprintf(stdout, "pid %d: already disconnected\n", drivers[i].pid);
                }
            }
        }
        else {
            fprintf(stderr, "Unknown command\n");
        }
    }

    cleanup_drivers();
    return 0;
}
