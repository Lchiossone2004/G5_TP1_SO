#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"
#include <time.h>

void clearScreen();
int main(int argc, char * argv[]) {

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


    int colors[] = {100, 102, 103, 104, 105, 106, 107, 100, 41};
    
    int aux;                                                //To store what is i the cell
    int found = 0;                                          //If teh player to be printed is found 

    while(!state_map->game_ended){
        clearScreen();
        sem_wait(&sync_map->A);                             //Tells master its going to print
        for(int row = 0; row < 10; row++){             //Rows
            for(int col = 0; col < 10; col++){            //Collums
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

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}
