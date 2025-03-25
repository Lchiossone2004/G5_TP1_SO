#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"

unsigned char CheckSurroundings(GameState *state_map, int width, int height, int playerNumber);      // Checks the surroundings of the bot to see the highest value tile to jump
int inBounds(int x, int y, GameState *state_map);      // Checks the position the algorithm is using is valid
int selectDir(int x, int y);                                // Selects the direction to go  

int main(int argc, char *argv[]){

    int width = atoi(argv[1]);          //Playing board width
    int height = atoi(argv[2]);         //Playing board height

    int state_fd = shm_open("/game_state", O_RDONLY,0666);  //Opens and maps the "game_state" shared memmory
    if(state_fd == -1){
        perror("Game State shared memmor fail.\n");
        exit(EXIT_FAILURE);
    }
    GameState *state_map = mmap(NULL, sizeof(GameState), PROT_READ, MAP_SHARED, state_fd,0);

    int sync_fd = shm_open("/game_sync", O_RDWR,0666);      //Opens and maps the "game_state" shared memmory
    if(sync_fd == -1){
        perror("Sync State shared memmor fail.\n");
        exit(EXIT_FAILURE);
    }
    GameSync *sync_map = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, sync_fd,0);

    int playerNumber = 0;
    pid_t pid = getpid();
    while(playerNumber< 9 && state_map->players_list[playerNumber].player_pid != pid){
        playerNumber++;
    }

    unsigned char direction;
    while(!state_map->players_list[playerNumber].is_blocked){
        sem_wait(&sync_map->C);         //Mutex of master
        sem_wait(&sync_map->E);         //Mutex of other bots
        sync_map->F++;                  //Increments the amount of readers
        if(sync_map->F == 1){           //Checks if he is the first reading 
            sem_wait(&sync_map->D);     //Mutex the game stat
        }
        sem_post(&sync_map->E);         //Frees other bots
      
        direction = CheckSurroundings(state_map,width,height,playerNumber);     
        sem_wait(&sync_map->E);         //Mutex of other bots
        sync_map->F--;                  //Substracs form the reader list
        if(sync_map->F == 0){
            sem_post(&sync_map->D);     //If he is the last one it frees the game state
        }
        sem_post(&sync_map->E);         //Frees other bots
        sem_post(&sync_map->C);         //Frees the master

        if(write(1, &direction, sizeof(direction)) == -1){  //Writes in the pipe o fd 1 (given by the master)
            perror("Failed to write on pipe 7\n");
         }
        sleep(3);
    }
    //Cleaning
    if(munmap(state_map,sizeof(GameState)) == -1){
        perror("Error unmaping the memory\n");
    }
    if(munmap(sync_map,sizeof(GameSync)) == -1){
        perror("Error unmaping the memory\n");
    }
    if(close(state_fd) == -1){
        perror("Error unmaping the memory\n");
    }
    if(close(sync_fd) == -1){
        perror("Error unmaping the memory\n");
    }
    return 0;
}

unsigned char CheckSurroundings(GameState *state_map, int width, int height, int playerNumber){
    int toRet = 0;
    int aux;
    unsigned int x,y;
    int actual = 0;
    x = state_map->players_list[playerNumber].pos_x;
    y = state_map->players_list[playerNumber].pos_y;

    for(int fil = -1 ; fil <= 1 ; fil++){
        for(int col = -1; col<= 1; col++){
            if((fil != 0 || col != 0 ) && inBounds(fil + y,col + x,state_map)){
                aux = state_map->board_origin[(x+col) + state_map->board_width*(y+fil)];
                if(aux > actual ){
                    toRet = selectDir(fil,col);
                    actual = state_map->board_origin[(x+col) + state_map->board_width*(y+fil)];
                }
            }
        }
}
    return toRet;
}

int inBounds(int y, int x, GameState *state_map) {
    // Verifica si las coordenadas están dentro de los límites del tablero
    return x >= 0 && x < state_map->board_width &&
           y >= 0 && y < state_map->board_height;
}

int selectDir(int i, int j) {
    // Mapea los desplazamientos (i, j) a las direcciones: 0 = arriba, 1 = derecha-arriba, 2 = derecha, etc.
    static const int directions[3][3] = {
        {7, 0, 1},
        {6, -1, 2},
        {5, 4, 3}
    };
    return directions[i + 1][j + 1]; // El índice +1 es para manejar los índices negativos
}