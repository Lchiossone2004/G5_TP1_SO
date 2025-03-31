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



void createPlayers(GameState *state_map,int players_added,int width, int height, char **players, int (*pipes)[2]);
void fillBoard(int width, int height, GameState *state_map);
void semaphoreStary(GameSync *sync_map);
 
int main(int argc, char * argv[]) {
    int width=10;
    int height=10;
    int delay= 200;
    int timeout=10;
    int seed = time(NULL);
    bool just_argued=false;
    char* view = NULL;
    char* players[9];
    char* setting_args[6]={"-w", "-h", "-d", "-t", "-s", "-v"};
    char is_bot=1;
    int players_added=0;
    int i=1;

    while(i<argc){
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
            height = atoi(argv[i]);
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
    if(players_added==0){
        perror("Error: At least one player must be specified using -p.");
        exit(EXIT_FAILURE);
    }
    if(players_added>9){
        perror("Error: At most 9 players can be specified using -p.");
        exit(EXIT_FAILURE);
    }

    //Creation of shared memory

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    createMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,height);

    //Setting up the game
    
        //Start the Semaphores
    semaphoreStary(sync_map);

        //Creating the board
    srand(seed);                                //Set the inputed seed, if it's not inputed it will be time(NULL) 
    fillBoard(width, height,state_map);
    state_map->board_height = height;
    state_map->board_width = width;
    state_map->game_ended = false;

    //Creation of child procesess

        //Creation of the view

    if(view != NULL){
        printf("hay view\n");
        char w[10];
        char h[10];
        sprintf(w,"%d",width);
        sprintf(h,"%d",height);
        char * args_list[] = {view,w,h, NULL};
        pid_t pid = fork();
        if(pid < 0){
            perror("Error connecting the view\n");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            execv(view,args_list);
            perror("Execv fail.\n");
            exit(EXIT_FAILURE);
        }
    }

    sem_post(&sync_map->A);
    sem_wait(&sync_map->B);
    sem_wait(&sync_map->A);

        //Creation of the pipes for players (one for each)

    int pipes[players_added][2];
    int max_fd = 0;
    for(int i = 0; i < players_added; i++){
        if(pipe(pipes[i]) == -1){
            perror("Failure creating pipe.\n");
            return 1;
        }
        if(pipes[i][0] > max_fd){
            max_fd = pipes[i][0];
        }
    }

        //Creation of the players
    createPlayers(state_map,players_added,width,height,players,pipes);

    //Pipe management
    struct timeval time_out;
    time_out.tv_sec = timeout;
    time_out.tv_usec = 0;
    fd_set read_fds;
    for(int i = 0; i<players_added; i++){
        FD_ZERO(&read_fds);                                     //Setting up the pipe list for the select 
        for(int j = 0; j <players_added; j++){                  //Fills up the pipe list
            FD_SET(pipes[i][0],&read_fds);                      
        }
    int act = select(max_fd+1,&read_fds,NULL,NULL,&time_out);   //Select for each player (checking each player pipe)
    if(act == -1){
        perror("Error makeing the select");
        return 1;
    }
    else if(act == 0){
        printf("Timeout\n");
    }
    else{
        for(int i = 0; i<players_added; i++){
            if(FD_ISSET(pipes[i][0],&read_fds)){                //Checks if the i-player has written something on his pipe
                char buffer[512] = {0};
                int readed = read(pipes[i][0],buffer,sizeof(buffer));
                if(readed < 0){
                    perror("Error reading the buffer\n");
                }
                else{
                printf("Readed:%s, From child N: %d\n",buffer,i+1);
                }
            }
        }
    }
}

    //Cleaning

    for(int i = 0; i < players_added; i++){                     //Waits for the players and view to finish
        wait(NULL);
    }

    clearMemory(state_map,sync_map,state_fd,sync_fd,width,height);  //Clears and closes the shared memory
    return 0;
}


void semaphoreStary(GameSync *sync_map){
    sem_init(&sync_map->A,0,1);
    sem_init(&sync_map->B,0,1);
    sem_init(&sync_map->C,1,1);
    sem_init(&sync_map->D,1,1);
    sem_init(&sync_map->E,1,1);
    sync_map->F = 0;
}



void fillBoard(int width, int height, GameState *state_map) {
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
           state_map->board_origin[i*width + j] = (rand() % 9) + 1; //para que quede entre 0 y 9
        }
    }
}

int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

void createPlayers(GameState *state_map,int players_added,int width, int height, char **players, int (*pipes)[2]){                                       //Creo los players, Casteo medio feo pero funciona
    
    int start_pos[9][2] = {{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{8,8},{9,9}};
    char w[10];
    char h[10];
    sprintf(w,"%d",width);
    sprintf(h,"%d",height);
    
    for(int i = 0; i<players_added;i++){
        pid_t pid = fork();
        if(pid < 0){
            perror("Error creating child process\n");
            exit(0);
        }
        if(pid == 0){
            //Set player parameters
            char * args_list[] = {players[i],w,h, NULL};
            state_map->players_list[i].is_blocked = false;
            strcpy(state_map->players_list[i].player_name, players[i]);
            state_map->players_list[i].pos_x = start_pos[i][0];
            state_map->players_list[i].pos_y = start_pos[i][1];
            close(pipes[i][0]);         //The child ony writes on the pipe 
            dup2(pipes[i][1],STDOUT_FILENO);    //Replace de stdout (fd: 1) wiith the created pipe 
            execv(players[i],args_list);
            perror("Player execv fail.\n");
            exit(0);
        }
        close(pipes[i][1]);             //Close the witting endo of the pipe for the pearent
        state_map->players_list[i].player_pid = pid;
    }
}

