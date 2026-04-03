CC = gcc
CFLAGS = -Wall -Wextra 

SRC_DIR = .

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/process_arg.c

TARGET = ../bin/argument_processor

all: 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) 

debug:
	$(CC) $(CFLAGS) -g -o $(TARGET) $(SRC_FILES) 
