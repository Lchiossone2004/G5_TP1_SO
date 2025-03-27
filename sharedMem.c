#include "./sharedMem.h"

void createMemory(int *state_fd, int *sync_fd, GameState **state_map, GameSync **sync_map, int width, int height) {                 //Creo la memoria compratida 
    *state_fd = shm_open(GAME_MEM, O_CREAT | O_RDWR, 0644);
    if (*state_fd == -1) {
        perror("Failure to create shared memory\n");
    }
    if (ftruncate(*state_fd, sizeof(GameState) + sizeof(int)*(width*height)) == -1) {
        perror("Failed to expand the shared memory\n");
    }
    *state_map = (GameState*)mmap(NULL, sizeof(GameState) + sizeof(int)*(width*height), PROT_READ | PROT_WRITE, MAP_SHARED, *state_fd, 0);
    if (*state_map == MAP_FAILED) {
        perror("Failed to map shared memory\n");
        exit(EXIT_FAILURE);
    }

    *sync_fd = shm_open(SYNC_MEM, O_CREAT | O_RDWR, 0666);
    if (*sync_fd == -1) {
        perror("Failure to create shared memory\n");
    }
    if (ftruncate(*sync_fd, sizeof(GameSync)) == -1) {
        perror("Failed to expand the shared memory\n");
    }
    *sync_map = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, *sync_fd, 0);
    if (*sync_map == MAP_FAILED) {
        perror("Failed to map shared memory\n");
    }
}

void clearMemory(GameState *state_map, GameSync *sync_map, int state_fd, int sync_fd, int width, int height){                       //Cierro la memoria compartida
    munmap(state_map,sizeof(GameState) + sizeof(int)*(width*height));
    close(state_fd);
    shm_unlink(GAME_MEM);
    munmap(sync_map,sizeof(GameSync));
    close(sync_fd);
    shm_unlink(SYNC_MEM);
}
void openMemory(int *state_fd, int *sync_fd, GameState **state_map, GameSync **sync_map, int width, int height){
    *state_fd = shm_open("/game_state", O_RDONLY,0644);  //Opens and maps the "game_state" shared memmory
        if(*state_fd == -1){
            perror("Game State shared memmor fail.\n");
            exit(EXIT_FAILURE);
        }
    *state_map = (GameState*)mmap(NULL, sizeof(GameState) + sizeof(int)*(width*height), PROT_READ, MAP_SHARED, *state_fd,0);

    *sync_fd = shm_open("/game_sync", O_RDWR,0666);      //Opens and maps the "game_state" shared memmory    
    if(*sync_fd == -1){
        perror("Sync State shared memmor fail.\n");
        exit(EXIT_FAILURE);
    }
    *sync_map = mmap(NULL, sizeof(GameSync), PROT_READ | PROT_WRITE, MAP_SHARED, *sync_fd,0);
}

void closeMemory(GameState *state_map, GameSync *sync_map, int state_fd, int sync_fd, int width, int height){
    if(munmap(state_map,sizeof(GameState) + sizeof(int)*(width*height)) == -1){
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
}