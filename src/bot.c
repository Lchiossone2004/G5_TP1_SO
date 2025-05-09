#include <string.h> // For strcpy
#include "sharedMem.h"

unsigned char CheckSurroundings(GameState *state_map, int width, int height, int playerNumber); // Checks the surroundings of the bot to see the highest value tile to jump
int inBounds(int x, int y, GameState *state_map);                                               // Checks the position the algorithm is using is valid
int selectDir(int x, int y);                                                                    // Selects the direction to go

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: falta argumento.\n");
        return 1;
    }

    int width = atoi(argv[1]);  // Playing board width
    int height = atoi(argv[2]); // Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd, &sync_fd, &state_map, &sync_map, width, height);

    int playerNumber = 0;
    pid_t pid = getpid();
    while (playerNumber < 9 && state_map->players_list[playerNumber].player_pid != pid)
    {
        playerNumber++;
    }

    unsigned char direction;
    int moved = 0; 
    int checkMoved = 0;
    while (!state_map->players_list[playerNumber].is_blocked && !state_map->game_ended)
    {
        sem_wait(&sync_map->master_mutex); // Mutex of master
        sem_post(&sync_map->master_mutex); // Mutex of master

        sem_wait(&sync_map->reader_mutex); // Mutex of other bots
        sync_map->readers_counter++;       // Increments the amount of readers
        if (sync_map->readers_counter == 1)
        {                                     // Checks if he is the first reading
            sem_wait(&sync_map->state_mutex); // Mutex the game stat
        }
        sem_post(&sync_map->reader_mutex); // Frees other bots

        direction = CheckSurroundings(state_map, width, height, playerNumber);
        checkMoved = state_map->players_list[playerNumber].invalid_moves + state_map->players_list[playerNumber].valid_moves;
        sem_wait(&sync_map->reader_mutex); // Mutex of other bots
        sync_map->readers_counter--;       // Substracs form the reader list
        if (sync_map->readers_counter == 0)
        {
            sem_post(&sync_map->state_mutex); // If he is the last one it frees the game state
        }
        sem_post(&sync_map->reader_mutex); // Frees other bots
        // sem_post(&sync_map->master_mutex); // Frees the master

        if(moved <= checkMoved){
            moved++;
        if (write(1, &direction, sizeof(direction)) == -1)
        { // Writes in the pipe o fd 1 (given by the master)
            perror("Failed to write on pipe 7\n");
        }
    }
    }
    closeMemory(state_map, sync_map, state_fd, sync_fd, width, height);
    close(1);
    return 0;
}

unsigned char CheckSurroundings(GameState *state_map, int width, int height, int playerNumber)
{
    int direction = 0;
    int position_value = 0;
    int actual_value = 0;
    unsigned int x, y;
    x = state_map->players_list[playerNumber].pos_x;
    y = state_map->players_list[playerNumber].pos_y;

    for (int fil = -1; fil <= 1; fil++)
    {
        for (int col = -1; col <= 1; col++)
        {
            if ((fil != 0 || col != 0) && inBounds(fil + y, col + x, state_map))
            {
                position_value = state_map->board_origin[(x + col) + state_map->board_width * (y + fil)];
                if (position_value > actual_value)
                {
                    direction = selectDir(fil, col);
                    actual_value = state_map->board_origin[(x + col) + state_map->board_width * (y + fil)];
                }
            }
        }
    }
    return direction;
}

int inBounds(int y, int x, GameState *state_map)
{ // Checks if the position is inside the boundaries of the board
    return x >= 0 && x < state_map->board_width &&
           y >= 0 && y < state_map->board_height;
}

int selectDir(int fil, int col)
{ // Returns the direction based on the adjacent x,y that has the highest value
    static const int directions[3][3] = {
        {7, 0, 1},
        {6, -1, 2},
        {5, 4, 3}};
    return directions[fil + 1][col + 1];
}