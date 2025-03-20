#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"

int main(int argc, char *argv[]){

    int state_fd = shm_open("/game_state", O_RDONLY,0666);                                  //Opens and maps the "game_state" shared memmory
    if(state_fd == -1){
        perror("Game State shared memmor fail.");
        exit(EXIT_FAILURE);
    }
    void *state_map = mmap(NULL, sizeof(GameState), PROT_READ, MAP_SHARED, state_fd,0);

    int sync_fd = shm_open("/game_sync", O_RDONLY,0666);                                    //Opens and maps the "game_state" shared memmory
    if(sync_fd == -1){
        perror("Game State shared memmor fail.");
        exit(EXIT_FAILURE);
    }
    void *sync_map = mmap(NULL, sizeof(GameSync), PROT_READ, MAP_SHARED, sync_fd,0);

    return 0;
}