#ifndef STRUCTS_H
#define STRUCTS_H


#include <stdio.h>
#include <fcntl.h>                  // For O_* constants
#include <sys/mman.h>               // For shm_open, mmap
#include <stdbool.h>                // For bool
#include <semaphore.h>

typedef struct{
    char player_name[16];           // Player name
    unsigned int score;             // Score
    unsigned int invalid_moves;     // Number of requests for invalid movements done
    unsigned int valid_moves;       // Number of requests for valid movements done
    unsigned short pos_x, pos_y;    // x,y coordinates on the bard
    pid_t player_pid;               // Process identifier
    bool is_blocked;                  // Indicates if the player is blocked
} PlayerState;

typedef struct{
    unsigned short board_width;      // Board with
    unsigned short board_height;    // Board height
    unsigned int num_of_players;    // Number of players
    PlayerState players_list[9];    // List of players
    bool game_ended;                // Indicates if the game is over
    int board_origin[];             // Pointer to the start of the board
}   GameState;

typedef struct{
    sem_t to_print;                        // Notify view there are changes to print
    sem_t end_print;                        // Notify master that view has finished printing
    sem_t master_mutex;                        // Mutex to prevent master blocking when accessing the state
    sem_t state_mutex;                        // Mutex for the game state
    sem_t reader_mutex;                        // Mutex for the next variable
    unsigned int readers_counter;                 // Number of players reading the state
}   GameSync;

#endif