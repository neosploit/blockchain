MKDIR_P = mkdir -p
CC = gcc
CFLAGS = -std=c99 -Wall
LDFLAGS = -lulfius -ljansson

SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
BIN_DIR = bin

ARGPARSE = $(LIB_DIR)/argparse/argparse.c

all: dns_server client

dns_server: $(wildcard $(SRC_DIR)/dns_server/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

client:	$(wildcard $(SRC_DIR)/client/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

clean:
	$(RM) $(BIN_DIR)/*