CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES_SERVER = $(SRC_DIR)/producer.c 

SRC_FILES_CLIENT = $(SRC_DIR)/consumer.c 

TARGET = ../bin

all:
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/svs_prod
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/svs_con

producer: 
	$(CC) $(CFLAGS) $(SRC_FILES_SERVER) -o $(TARGET)/svq_server

consumer:
	$(CC) $(CFLAGS) -g $(SRC_FILES_CLIENT) -o $(TARGET)/svq_client
