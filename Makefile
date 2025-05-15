# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude 

#Directories 

SRC_DIR = src
BIN_DIR = binaries

# Target binaries
BOT = $(BIN_DIR)/bot
VIEW = $(BIN_DIR)/view
SHM = sharedMem
MAST = $(BIN_DIR)/master

# Source files for each program
BOT_SRC = $(SRC_DIR)/bot.c $(SRC_DIR)/sharedMem.c
VIEW_SRC = $(SRC_DIR)/view.c $(SRC_DIR)/sharedMem.c
MASTER_SRC = $(SRC_DIR)/master.c $(SRC_DIR)/sharedMem.c $(SRC_DIR)/masterLib.c

# Default values
BOTS ?= 1
WIDTH ?= 10
HEIGHT ?= 10
DELAY ?= 200
TIMEOUT ?= 10
SEED?=
VIEW_ON ?= no

# Default target to build bot and view
build: $(BOT) $(VIEW) $(MAST)

# Rule to build the bot binary
$(BOT): $(BOT_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build the view binary
$(VIEW): $(VIEW_SRC)
	$(CC) $(CFLAGS) $^ -o $@
	
# Rule to build the master binary
$(MAST): $(MASTER_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Run the precompiled ChompChamps program with the specified number of bots
run:
	./$(BIN_DIR)/ChompChamps -w $(WIDTH) -h $(HEIGHT) -d $(DELAY) -t $(TIMEOUT) -p $(foreach n, $(shell seq 1 $(BOTS)), $(BOT)) $(if $(filter yes,$(VIEW_ON)),-v $(VIEW)) \
		$(if $(SEED),-s $(SEED))

# Run the precompiled ChompChamps program with the specified number of bots
run_nat:
	./$(MAST) -w $(WIDTH) -h $(HEIGHT) -d $(DELAY) -t $(TIMEOUT) -p $(foreach n, $(shell seq 1 $(BOTS)), $(BOT)) $(if $(filter yes,$(VIEW_ON)),-v $(VIEW)) \
		$(if $(SEED),-s $(SEED))

# Clean up binary files
clean:
	rm -f $(BOT) $(VIEW) $(MAST) ./test/test_report.txt 
	rm -rf ./test/test_logs/*

# Builds, runs, and cleans the project with the original master
all: build run clean
# Builds, runs, and cleans the project with our master
all_nat: build run_nat clean