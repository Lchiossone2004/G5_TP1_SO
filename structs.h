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
    bool can_move;                  // Indicates if the player has valid moves remaining
} PlayerState;

typedef struct{
    unsigned short board_with;      // Board with
    unsigned short board_height;    // Board height
    unsigned int num_of_players;    // Number of players
    PlayerState players_list[9];    // List of players
    bool game_ended;                // Indicates if the game is over
    int board_origin[];             // Pointer to the start of the board
}   GameState;

typedef struct{
    sem_t A;                        // Notify view there are changes to print
    sem_t B;                        // Notify master that view has finished printing
    sem_t C;                        // Mutex to prevent master blocking when accessing the state
    sem_t D;                        // Mutex for the game state
    sem_t E;                        // Mutex for the next variable
    unsigned int F;                 // Number of players reading the state
}   GameSync;