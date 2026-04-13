CC = gcc
CFLAGS = -Wall -Wextra 

SRC_DIR = .

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/executable.c $(SRC_DIR)/string_array.c $(SRC_DIR)/executable_list.c

TARGET = ../bin/simple_shell

all: 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) 

debug:
	$(CC) $(CFLAGS) -g -o $(TARGET) $(SRC_FILES) 
