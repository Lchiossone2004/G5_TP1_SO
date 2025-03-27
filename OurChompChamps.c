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
    bool just_argued=false;
    bool has_view=false;
    char* view;
    char* players[9];
    char* setting_args[6]={"-w", "-h", "-d", "-t", "-s", "-v"};
    char is_bot=1;
    int players_added=0;
    int i=1;
    while (i < argc) {
        if (just_argued) {
            for (int j = 0; j < 6; j++) {
                if (!strcmp(argv[i], setting_args[j])) {
                    perror("Error: Value expected after previous argument, not another argument.\n");
                    exit(EXIT_FAILURE);
                }
            }
            just_argued = false;
        }
        if (!strcmp(argv[i], "-p")) {
            just_argued = true;
            i++;
            while (is_bot && players_added < 9 && i < argc) {
                for (int j = 0; j < 6 && is_bot; j++) {
                    if (!strcmp(argv[i], setting_args[j])) {
                        is_bot = 0;
                    }
                }
                if (is_bot) {
                    players[players_added++] = argv[i++];
                    printf("%s \n", argv[i - 1]);
                }
            }
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[0])) {  // -w
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -w requires a value.\n");
                exit(EXIT_FAILURE);
            }
            if (atoi(argv[i]) < 10) {
                perror("Invalid width value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            width = atoi(argv[i]);
            i++;
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[1])) {  // -h
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -h requires a value.\n");
                exit(EXIT_FAILURE);
            }
            if (atoi(argv[i]) < 10) {
                perror("Invalid height value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            heigth = atoi(argv[i]);
            i++;
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[2])) {  // -d
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -d requires a value.\n");
                exit(EXIT_FAILURE);
            }
            if (atoi(argv[i]) < 0) {
                perror("Invalid delay value, must be positive.\n");
                exit(EXIT_FAILURE);
            }
            delay = atoi(argv[i]);
            i++;
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[3])) {  // -t
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -t requires a value.\n");
                exit(EXIT_FAILURE);
            }
            if (atoi(argv[i]) < 10) {
                perror("Invalid timeout value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            timeout = atoi(argv[i]);
            i++;
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[4])) {  // -s
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -s requires a value.\n");
                exit(EXIT_FAILURE);
            }
            seed = atoi(argv[i]);
            i++;
            just_argued = false; 
        }
        else if (!strcmp(argv[i], setting_args[5])) {  // -v
            just_argued = true;
            if (++i >= argc) {
                perror("Error: -v requires a value.\n");
                exit(EXIT_FAILURE);
            }
            has_view = true;
            view = argv[i];
            i++;
            just_argued = false; 
        }
        else if (!just_argued) {
            perror("Invalid argument\n");
            exit(EXIT_FAILURE);
        }
    }
    if (just_argued) {
        perror("Error: Last argument requires a value.\n");
        exit(EXIT_FAILURE);
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