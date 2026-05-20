CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/server.c 

SRC_FILES_CLIENT = $(SRC_DIR)/client.c 

TARGET = ../bin

all:
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/tcpser
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/tcpcli

server: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/tcpser

client:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/tcpcli
