#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"

unsigned char CheckSurroundings(GameState *state_map);      // Checks the surroundings of the bot to see the highest value tile to jump
int inBounds(int x, int y, GameState *state_map);      // Checks the position the algorithm is using is valid
int selectDir(int x, int y);                                // Selects the direction to go  

int main(int argc, char *argv[]){

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

    unsigned char direction;
    unsigned int x,y;
    for(int i = 0; i < 2; i++){
        sem_wait(&sync_map->C);
        sem_wait(&sync_map->E);
        sync_map->F++;
        if(sync_map->F == 1){
            sem_wait(&sync_map->D);
        }
        sem_post(&sync_map->E);
        sem_post(&sync_map->C);
        direction = CheckSurroundings(state_map);     
        if(write(1, &direction, sizeof(direction)) == -1){  //Writes in the pipe o fd 1 (given by the master)
           perror("Failed to write on pipe 7\n");
        }
        sem_wait(&sync_map->E);
        sync_map->F--;
        if(sync_map->F == 0){
            sem_post(&sync_map->D);
        }
        sem_post(&sync_map->E);
        sem_post(&sync_map->C);
    }
    return 0;
}

unsigned char CheckSurroundings(GameState *state_map){
    int toRet = -1;
    int aux;
    unsigned int x,y;
    int actual = 0;
    x = state_map->players_list[0].pos_x;
    y = state_map->players_list[0].pos_y;

    for(int fil = -1 ; fil <= 1 ; fil++){
        for(int col = -1; col<= 1; col++){
            if((fil != 0 || col != 0 ) && inBounds(fil + y,col + x,state_map)){
                aux = state_map->board_origin[(x+col) + state_map->board_with*(y+fil)];
                if(aux > actual ){
                    toRet = selectDir(fil,col);
                    actual = state_map->board_origin[(x+col) + state_map->board_with*(y+fil)];
                }
            }
        }
}
    return toRet;
}

int inBounds(int y, int x, GameState *state_map) {
    // Verifica si las coordenadas están dentro de los límites del tablero
    return x >= 0 && x < state_map->board_with &&
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