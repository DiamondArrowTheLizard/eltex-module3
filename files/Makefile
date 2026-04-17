CC = gcc
CFLAGS = -Wall -Wextra -Werror

SRC_DIR = .

SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/personal_info.c $(SRC_DIR)/array_contacts.c $(SRC_DIR)/ui.c

TARGET = ../bin/contact_book

all: 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES) 

debug:
	$(CC) $(CFLAGS) -g -o $(TARGET) $(SRC_FILES) 
