#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"

unsigned char CheckSurroundings(GameState *state_map);      // Checks the surroundings of the bot to see the highest value tile to jump
int inBounds(int totalPosition, GameState *state_map);      // Checks the position the algorithm is using is valid
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

    for(int i = 0; i < 5; i++){
        direction = CheckSurroundings(state_map);     
        if(write(1, &direction, sizeof(direction)) == -1){  //Writes in the pipe o fd 1 (given by the master)
            perror("Failed to write on pipe 7\n");
        }
        sem_post(&sync_map->C); // Changes the semaphore to tell the master that it can read.

    }
    return 0;
}

unsigned char CheckSurroundings(GameState *state_map){
    unsigned char toRet = 0;
    int aux;
    unsigned int x,y;
    x = state_map->players_list[0].pos_x;
    y = state_map->players_list[0].pos_y;
    for(int i = -1 ; i <= 1 ; i++){
        for(int j = -1; j<= 1; j++){
            if( i != 0 && j != 0){
                aux = state_map->board_origin[(x+i) + state_map->board_with*(y+j)];
                if(inBounds(aux,state_map) && state_map->board_origin[aux] > 1 && state_map->board_origin[aux] > toRet){
                    toRet = selectDir(i,j);
                }
            }
        }
}
    return toRet;
}

int inBounds(int totalPosition, GameState *state_map){
    return totalPosition <= state_map->board_height*state_map->board_with && totalPosition >= 0;
}

int selectDir(int i, int j){
    if(i == -1){
        if(j == -1){
            return 7;
        }
        if( j == 0){
            return 6;
        }
        if( j == 1){
            return 5;
        }
    }
    if(i ==0){
        if(j == -1){
            return 0;
        }
        if( j == 1){
            return 4;
        }
    }
    if(i == 1){
        if(j == -1){
            return 1;
        }
        if( j == 0){
            return 2;
        }
        if( j == 1){
            return 3;
        }
    }
    return -1;

}