CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/server.c 

SRC_FILES_CLIENT = $(SRC_DIR)/client.c 

TARGET = ../bin

all:
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/ser
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/cli

server: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/ser

client:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/cli
