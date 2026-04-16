CC = gcc
CFLAGS = -Wall -Wextra 

SRC_DIR = .

SRC_FILES = $(SRC_DIR)/params.c $(SRC_DIR)/user.c

TARGET = ../bin

all: 
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(TARGET)/pq

debug:
	$(CC) $(CFLAGS) -g $(SRC_FILES) -o $(TARGET)/pq
