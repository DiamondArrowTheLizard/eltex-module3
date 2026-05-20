CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/server.c 

SRC_FILES_CLIENT = $(SRC_DIR)/client.c 

TARGET = ../bin

all:
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/ctser
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/ctcli

server: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/ctser

client:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/ctcli
