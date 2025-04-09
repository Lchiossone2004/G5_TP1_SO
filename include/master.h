#ifndef MASTER_H
#define MASTER_H
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "structs.h"
#include <time.h>
#include <sys/wait.h>   //To wait
#include <sys/types.h>
#include <sys/select.h>
#include "sharedMem.h"

typedef struct{
    int direction;
    int player_num;
}Request;

void processArguments(int argc, char *argv[], int *width, int *height, int *delay, int *timeout, int *seed, char **view, char **players, int *players_added);
void createPlayers(GameState *state_map,int players_added,int width, int height, char **players, int (*pipes)[2]);
void fillBoard(int width, int height, GameState *state_map);
void semaphoreStary(GameSync *sync_map);
Request checkRequest(struct timeval time_out, int players_added,int (*pipes)[2], int max_fd, int * current_player);
int processRequest(Request request, GameState *state_map);
int isValid(int y, int x, GameState *state_map);
void isBlocked(GameState *state_map, int player_number);

#endif