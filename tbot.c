#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"
#include <time.h>
#include <sys/wait.h>   //To wait
#include <sys/types.h>
#include <sys/select.h>
#include "./sharedMem.h"

int main(int argc, char * argv[]){

    int width = atoi(argv[1]);          //Playing board width
    int height = atoi(argv[2]);         //Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,height);
    for(int i; i <height; i++){
        for(int j = 0; j <width; j++){
            printf("[%d]",state_map->board_origin[width*i+j]);
        }
        printf("\n");
    }
    pid_t pid = getpid();
    int i = 0;
    while(pid != state_map->players_list[i].player_pid) i++;
    printf("Name: %s",state_map->players_list[i].player_name);
    printf("Pos x: %d Posy: %d\n",state_map->players_list[i].pos_x,state_map->players_list[i].pos_y);
    return 0;
}