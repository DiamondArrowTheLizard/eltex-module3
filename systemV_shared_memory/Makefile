CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = .

SRC_FILES =  $(SRC_DIR)/main.c

TARGET = ../bin/svmem

all: 
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) -g $(SRC_FILES) -o $(TARGET)
