#flags 
CC = gcc
CFLAGS = -Wall

# Target binaries
BOT = bot
VIEW = view

# Source files for each program
BOT_SRC = bot.c
VIEW_SRC = view.c

# Object files for each program
BOT_OBJ = $(BOT_SRC:.c=.o)
VIEW_OBJ = $(VIEW_SRC:.c=.o)

# Default number of bots to 1
BOTS ?= 1

# Default view true 
VIEW_ON ?= yes
# Default target to build bot and view
all: $(BOT) $(VIEW)

# Rule to build the bot binary
$(BOT): $(BOT_OBJ)
	$(CC) $(BOT_OBJ) -o $(BOT)

# Rule to build the view binary
$(VIEW): $(VIEW_OBJ)
	$(CC) $(VIEW_OBJ) -o $(VIEW)

# Rule to compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the precompiled ChompChamps program with the specified number of bots
run: $(BOT) $(VIEW)
	./ChompChamps -p $(foreach n, $(shell seq 1 $(BOTS)),  $(BOT)) $(if $(filter yes,$(VIEW_ON)),-v $(VIEW))

# Clean up object and binary files
clean:
	rm -f $(BOT_OBJ) $(VIEW_OBJ) $(BOT) $(VIEW)
#Builds runs and cleans the project
build_run_clean: all run clean