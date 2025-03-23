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
int main() {

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
    int colors[9] = {41,42,43,44,45,46,47,100,101};         //Colors for the players
    int aux;                                                //To store what is i the cell
    int found = 0;                                          //If teh player to be printed is found 
    while(!state_map->game_ended){
        clearScreen();
        sem_wait(&sync_map->A);                             //Tells master its going to print
        for(int row = 0; row < state_map->board_height; row++){             //Rows
            for(int col = 0; col <state_map->board_with; col++){            //Collums
                aux = state_map->board_origin[col+row*state_map->board_with];
                if(aux <= 0){
                    for(int i = 0; i < state_map->num_of_players && !found; i++){
                        if(aux == i*(-1)){
                            printf("\033[%dm[%2d]\033[0m",colors[i],aux);       //Prints the player in color 
                            found = 1;
                        }
                    }
                }
                else{
                printf("[%2d]",aux);
                }
                found = 0;
        }
        printf("\n");
    }
    for(int i = 0; i < state_map->num_of_players; i++){
        printf("\033[%dm  \033[0m ",colors[i]);                                 //Prints the color of the plyer 
        printf("Player: %s | ", state_map->players_list[i].player_name);        //Prints player name
        printf("Score: %d\n", state_map->players_list[i].score);                //Prints player score
    }
        sem_post(&sync_map->B);                             //Tells master it finished printing 
    }


    // clearScreen();
    // srand(time(NULL));
    // int rowas = 10;
    // int columnas = 10;
    // int random1;
    // int random2;
    // int timeOfTimer = 2;
    // time_t start, now;
    // time(&start);
    // char *table[rowas][columnas];
    // int coloredMatrix[rowas][columnas];
    // for(int i = 0; i <rowas; i++){
    //     for(int j = 0; j <columnas; j++){
    //         random1 = rand() % 10 + 1;
    //         table[i][j] = malloc(20);
    //         coloredMatrix[i][j] = 0;
    //         snprintf(table[i][j], 20, "[%2d]",random1);
    //     }
    // }
    // while (timeOfTimer) {
    //     random1 = rand() %10;
    //     random2 = rand() %10;
    //     time(&now);
    //     printf("Timer: %d\n", timeOfTimer);
    //     if(difftime(now,start) >=1){
    //         start = now;
    //         timeOfTimer--;
    //     }
    //     if(coloredMatrix[random1][random2] == 0){
    //         coloredMatrix[random1][random2] = 1;
    //         char aux[20];
    //         strcpy(aux, table[random1][random2]);
    //         snprintf(table[random1][random2],30, "\033[41m%s\033[0m",aux);
    //         drawBoard(rowas,columnas,table);
    //         usleep(500000);
    //     }
    //     clearScreen();
    // }
    // printf("Game ended\n");
    // usleep(1000000);
    // clearScreen();
    // for(int i = 0; i<rowas; i++){
    //     for(int j = 0; j < columnas; j++){
    //         free(table[i][j]);
    //     }
    // }
    return 0;
}

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}
