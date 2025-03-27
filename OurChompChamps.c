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

void createPlayers(int players_added,int width, int height, char **players, int (*pipes)[2]);

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

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    createMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,heigth);

    //Creacion de procesos

    //Creo los pipes para los hijos (uno por hijo)

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

    //Creacion de los players
    createPlayers(players_added,width,heigth,players,pipes);

    //Manejo de los pipes
    struct timeval time_out;
    time_out.tv_sec = timeout;
    time_out.tv_usec = 0;
    fd_set read_fds;
    for(int i = 0; i<players_added; i++){
        FD_ZERO(&read_fds);                                     //Inicializo el conjunto de los files descriptors para leer el pipe
        for(int j = 0; j <players_added; j++){                  //Reincinio el read_fs cada iteracion
            FD_SET(pipes[i][0],&read_fds);                      //Agrega cada file descriptor a la ""lista"" de read fd
        }
    //Hago el select para esperar a ver si el jugador escribe algo
    int act = select(max_fd+1,&read_fds,NULL,NULL,&time_out);
    if(act == -1){
        perror("Error makeing the select");
        return 1;
    }
    else if(act == 0){
        printf("Timeout\n");
    }
    else{
        for(int i = 0; i<players_added; i++){
            if(FD_ISSET(pipes[i][0],&read_fds)){                //Mira si el i-esimo pipe tiene cosas para leer
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
    //Creacion de la view


    //Cleaning

    for(int i = 0; i<players_added; i++){
        wait(NULL);
    }

    clearMemory(state_map,sync_map,state_fd,sync_fd,width,heigth);
    return 0;
}



void createPlayers(int players_added,int width, int height, char **players, int (*pipes)[2]){                                       //Creo los players, Casteo medio feo pero funciona
    char w[10];
    char h[10];
    char player_name[16] = "nombre";
    sprintf(w,"%d",width);
    sprintf(h,"%d",height);
    char * args_list[] = {player_name,w,h, NULL};
    
    for(int i = 0; i<players_added;i++){
        pid_t pid = fork();
        if(pid < 0){
            perror("Error creating child process\n");
            exit(0);
        }
        if(pid == 0){
            strcpy(player_name,players[i]);
            close(pipes[i][0]);         //The child ony writes on the pipe 
            dup2(pipes[i][1],STDOUT_FILENO);    //Replace de stdout (fd: 1) wiith the created pipe 
            execv(players[i],args_list);
            perror("Execv fail.\n");
            exit(0);
        }
    }
}

