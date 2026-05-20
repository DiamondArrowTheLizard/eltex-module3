CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/server.c 

SRC_FILES_CLIENT = $(SRC_DIR)/client.c 

TARGET = ../bin

server: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/svq_server

client:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/svq_client
