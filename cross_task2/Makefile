CC = gcc
CFLAGS = -Wall -Wextra 

SRC_DIR = .

SRC_FILES =  $(SRC_DIR)/main.c

TARGET = ../bin/taxi

all: 
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) -g $(SRC_FILES) -o $(TARGET)
