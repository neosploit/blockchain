MKDIR_P = mkdir -p
CC = gcc
NVCC = nvcc
CFLAGS = -std=c99 -Wall
LDFLAGS = -lulfius -ljansson
LPTHREAD = -lpthread
FOPENMP = -fopenmp

CNFLAGS = -pedantic -O2
LDNFLAGS = -lX11 -lm -lGL -lGLU

SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
BIN_DIR = bin

ARGPARSE = $(LIB_DIR)/argparse/argparse.c

all: dns_server client wallet wallet_gui miner omp_miner cuda_miner

basic: dns_server client wallet miner

dns_server: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/dns_server/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

client:	$(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/client/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

wallet: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/wallet_console/*.c) $(wildcard $(SRC_DIR)/wallet/*.c) $(wildcard $(SRC_DIR)/hashing/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS) $(LPTHREAD)

wallet_gui: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/wallet_gui/*.c) $(wildcard $(SRC_DIR)/wallet/*.c) $(wildcard $(SRC_DIR)/hashing/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) $(CNFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS) $(LDNFLAGS) $(LPTHREAD)

miner: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/serial_miner/*.c) $(wildcard $(SRC_DIR)/miner/*.c) $(wildcard $(SRC_DIR)/hashing/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

omp_miner: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/omp_miner/*.c) $(wildcard $(SRC_DIR)/miner/*.c) $(wildcard $(SRC_DIR)/hashing/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS) $(FOPENMP)

cuda_miner: $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/cuda_miner/*.c*) $(wildcard $(SRC_DIR)/miner/*.c) $(wildcard $(SRC_DIR)/hashing/*.c)
	$(MKDIR_P) $(BIN_DIR)
	$(NVCC) -I $(INC_DIR)/$@ -o $(BIN_DIR)/$@ $^ $(ARGPARSE) $(LDFLAGS)

clean:
	$(RM) $(BIN_DIR)/*
