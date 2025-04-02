#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "structs.h"
#include <time.h>
#include <sys/wait.h>   //To wait
#include <sys/types.h>
#include <sys/select.h>
#include "sharedMem.h"

int main(int argc, char * argv[]){

    int width = atoi(argv[1]);          //Playing board width
    int height = atoi(argv[2]);         //Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,height);
    sem_wait(&sync_map->master_mutex);                                 //Mutex of master
    sem_post(&sync_map->master_mutex);                                 //Mutex of master

    sem_wait(&sync_map->reader_mutex);                                 //Mutex of other bots
    sync_map->readers_counter++;                                          //Increments the amount of readers
    if(sync_map->readers_counter == 1){                                   //Checks if he is the first reading 
        sem_wait(&sync_map->state_mutex);                             //Mutex the game stat
    }
    sem_post(&sync_map->reader_mutex);                                 //Frees other bots
  
    sem_wait(&sync_map->reader_mutex);                                 //Mutex of other bots
    sync_map->readers_counter--;                                          //Substracs form the reader list
    if(sync_map->readers_counter == 0){
        sem_post(&sync_map->state_mutex);                             //If he is the last one it frees the game state
    }
    sem_post(&sync_map->reader_mutex);                                 //Frees other bots
    int direc = 4;
    write(1, &direc, sizeof(direc));
    direc = 4;
    write(1, &direc, sizeof(direc));
    direc = 4;
    write(1, &direc, sizeof(direc));
    direc = 4;
    write(1, &direc, sizeof(direc));
    closeMemory(state_map,sync_map,state_fd,sync_fd, width, height);
    return 0;
}