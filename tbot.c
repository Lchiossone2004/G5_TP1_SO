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

int main(int argc, char * argv[]){
    printf("hola soy el bot, mi width es: %s y mi height es; %s y mi nombre es: %s\n", argv[1],argv[2],argv[0]);
    return 0;
}