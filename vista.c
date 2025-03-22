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
void drawBoard(int filas, int columnas, char *table[filas][columnas]);
void delay(int milliseconds);

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

    int aux;
    while(!state_map->game_ended){
        clearScreen();
        sem_wait(&sync_map->A);
        for(int fil = 0; fil < state_map->board_height; fil++){
            for(int col = 0; col <state_map->board_with; col++){
                aux = state_map->board_origin[col+fil*state_map->board_with];
                if(aux == 0){
                    printf("\033[41m[%2d]\033[0m",aux);
                }
                else{
                printf("[%2d]",aux);
                }
         }
         printf("\n");
     }
        printf("Player: %s ", state_map->players_list[0].player_name);
        printf("Score: %d\n", state_map->players_list[0].score);
        sem_post(&sync_map->B);
    }


    // clearScreen();
    // srand(time(NULL));
    // int filas = 10;
    // int columnas = 10;
    // int random1;
    // int random2;
    // int timeOfTimer = 2;
    // time_t start, now;
    // time(&start);
    // char *table[filas][columnas];
    // int coloredMatrix[filas][columnas];
    // for(int i = 0; i <filas; i++){
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
    //         drawBoard(filas,columnas,table);
    //         usleep(500000);
    //     }
    //     clearScreen();
    // }
    // printf("Game ended\n");
    // usleep(1000000);
    // clearScreen();
    // for(int i = 0; i<filas; i++){
    //     for(int j = 0; j < columnas; j++){
    //         free(table[i][j]);
    //     }
    // }
    return 0;
}

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}
