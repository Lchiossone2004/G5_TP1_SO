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

TBOT = $(BIN_DIR)/tbot

# Source files for each program
BOT_SRC = $(SRC_DIR)/bot.c $(SRC_DIR)/sharedMem.c
VIEW_SRC = $(SRC_DIR)/view.c $(SRC_DIR)/sharedMem.c
MASTER_SRC = $(SRC_DIR)/master.c $(SRC_DIR)/sharedMem.c

TBOT_SRC = $(SRC_DIR)/tbot.c $(SRC_DIR)/sharedMem.c

# Default number of bots to 1
BOTS ?= 1

# Default view true
VIEW_ON ?= yes

# Default target to build bot and view
build: $(BOT) $(VIEW) $(MAST) $(TBOT)

# Rule to build the bot binary
$(BOT): $(BOT_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build the view binary
$(VIEW): $(VIEW_SRC)
	$(CC) $(CFLAGS) $^ -o $@
	
# Rule to build the master binary
$(MAST): $(MASTER_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build the view binary
$(TBOT): $(TBOT_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Run the precompiled ChompChamps program with the specified number of bots
run: $(BOT) $(VIEW)
	./$(BIN_DIR)/ChompChamps -p $(foreach n, $(shell seq 1 $(BOTS)), $(BOT)) $(if $(filter yes,$(VIEW_ON)),-v $(VIEW))

# Run the precompiled ChompChamps program with the specified number of bots
run_nat: $(TBOT) $(VIEW)
	./$(MAST) -p $(foreach n, $(shell seq 1 $(BOTS)), $(TBOT)) $(if $(filter yes,$(VIEW_ON)),-v $(VIEW))

# Clean up binary files
clean:
	rm -f $(BOT) $(VIEW) $(MAST) $(TBOT)

# Builds, runs, and cleans the project
all: build run clean