#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"
#include <time.h>
#include <sys/wait.h>   //To wait
#include <sys/types.h>
#include <sys/select.h>

#define GAME_MEM "/game_state"
#define SYNC_MEM "/game_sync"

void createMemory(int *state_fd, int *sync_fd, GameState **state_map, GameSync **sync_map, int width, int height);
void clearMemory(GameState *state_map, GameSync *sync_map, int state_fd, int sync_fd, int width, int height);
void openMemory(int *state_fd, int *sync_fd, GameState **state_map, GameSync **sync_map, int width, int height);
void closeMemory(GameState *state_map, GameSync *sync_map, int state_fd, int sync_fd, int width, int height);

#endif