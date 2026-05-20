CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/server.c 

SRC_FILES_CLIENT = $(SRC_DIR)/client.c 

TARGET = ../bin

all:
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/svq_server
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/svq_client

server: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/svq_server

client:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/svq_client
