#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "structs.h"
#include <time.h>
#include "sharedMem.h"

void clearScreen();
int main(int argc, char * argv[]) {

    int width = atoi(argv[1]);          //Playing board width
    int height = atoi(argv[2]);         //Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,height);

    int colors[] = {100, 102, 103, 104, 105, 106, 107, 100, 41};
    
    int aux;                                                //To store what is i the cell
    int found = 0;                                          //If teh player to be printed is found 

    while(!state_map->game_ended){
        clearScreen();
        sem_wait(&sync_map->A);                             //Tells master its going to print
        for(int row = 0; row < height; row++){             //Rows
            for(int col = 0; col < width; col++){            //Collums
                aux = state_map->board_origin[col+row*width];
                if(aux <= 0){
                    for(int i = 0; i < state_map->num_of_players && !found; i++){
                        if(aux == i*(-1)){
                            if(col == state_map->players_list[i].pos_x && row == state_map->players_list[i].pos_y){
                                printf("\033[%dm[*%2d*]\033[0m",colors[i],aux);       //Prints the player in color 
                            }
                            else{
                                printf("\033[%dm[ %2d ]\033[0m",colors[i],aux);       //Prints the player in color 
                            }
                            found = 1;
                        }
                    }
                }
                else{
                printf("[ %2d ]",aux);
                }
                found = 0;
        }
        printf("\n");
    }
    for(int i = 0; i < state_map->num_of_players; i++){
        printf("\033[%dm  \033[0m ",colors[i]);                                 //Prints the color of the plyer 
        printf("Player: %s | ", state_map->players_list[i].player_name);        //Prints player name
        printf("Score: %d | ", state_map->players_list[i].score);                //Prints player score
        printf("Coordinates(x,y): (%d,%d)\n",state_map->players_list[i].pos_x,state_map->players_list[i].pos_y);
    }
        sem_post(&sync_map->B);                             //Tells master it finished printing 
    }
        //Cleaning
    closeMemory(state_map,sync_map,state_fd,sync_fd,width,height);
    return 0;
}

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}
