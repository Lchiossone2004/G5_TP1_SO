#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // For O_* constants
#include <sys/mman.h>   // For shm_open, mmap
#include <unistd.h>     // For ftruncate, close
#include <string.h>     // For strcpy
#include <stdbool.h>    // For bool
#include "./structs.h"
#include <time.h>
#define GAME_MEM "/game_state"
#define SYNC_MEM "/game_sync"

int main(int argc, char * argv[]) {
    int width=10;
    int heigth=10;
    int delay= 200;
    int timeout=10;
    int seed=time(NULL);

    bool has_view=false;
    char* view;
    char* players[9];
    char* setting_args[6]={"-w", "-h", "-d", "-t", "-s", "-v"};
    char is_bot=1;
    int players_added=0;
    int i=1;
    while(i<argc){
        if(!(strcmp(argv[i],"-p"))){
            i++;
            while (is_bot && players_added<9 && i<argc){
                for(int j=0 ; j<6 && is_bot; j++){
                    if(!(strcmp(argv[i], setting_args[j]))){
                        is_bot=0;
                    }
                   
                }
                if(is_bot){
                    players[players_added++]=argv[i++];
                    printf("%s \n",argv[i-1]);
                }
            }
        }
        else if(!(strcmp(argv[i],setting_args[0])) && atoi(argv[++i])>=10){
            width=atoi(argv[i]);
            i++;
        }
        else if(!(strcmp(argv[i],setting_args[1])) && atoi(argv[++i])>=10){
            heigth=atoi(argv[i]);
            i++;
        }
        else if(!(strcmp(argv[i],setting_args[2])) && atoi(argv[++i])>=0){
            delay=atoi(argv[i]);
            i++;
        }
        else if(!(strcmp(argv[i],setting_args[3])) && atoi(argv[++i])>=10){
            timeout=atoi(argv[i]);
            i++;
        }
        else if(!(strcmp(argv[i],setting_args[4])) && atoi(argv[++i])){
            seed=atoi(argv[i]);
            i++;
        }
        else if(!(strcmp(argv[i],setting_args[5]))){
            has_view=true;
            view=argv[++i];
            i++;
        }
        else{
            perror("Invalid argument \n");
            exit(EXIT_FAILURE);
        }
    }

    //Creacion de la memoria compartida 

    int state_fd = shm_open(GAME_MEM, O_CREAT | O_RDWR, 0666);
    if(state_fd == -1){
        perror("Failure to create shared memmory\n");
    }
    if(ftruncate(state_fd, sizeof(GameState) + sizeof(int)*(width*heigth)) == -1){
        perror("Failed to expand the shared memmory\n");
    }
    GameState *state_map = mmap(NULL, sizeof(GameState) + sizeof(int)*(width*heigth), PROT_READ, MAP_SHARED, state_fd,0);

    int sync_fd = shm_open(GAME_MEM, O_CREAT | O_RDWR, 0666);
    if(sync_fd == -1){
        perror("Failure to create shared memmory\n");
    }
    if(ftruncate(sync_fd, sizeof(GameSync)) == -1){
        perror("Failed to expand the shared memmory\n");
    }
    GameSync *sync_map = mmap(NULL, sizeof(GameSync), PROT_READ, MAP_SHARED, state_fd,0);




    //Cleaning
    munmap(state_map,sizeof(GameState) + sizeof(int)*(width*heigth));
    close(state_fd);
    shm_unlink(GAME_MEM);
    munmap(sync_map,sizeof(GameSync));
    close(sync_fd);
    shm_unlink(SYNC_MEM);
}