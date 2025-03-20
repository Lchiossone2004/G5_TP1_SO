#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>  // Para usleep()
void clearScreen();
void drawBoard(int filas, int columnas, char *table[filas][columnas]);
void delay(int milliseconds);

int main() {
    clearScreen();
    srand(time(NULL));
    int filas = 10;
    int columnas = 10;
    int random1;
    int random2;
    int timeOfTimer = 2;
    time_t start, now;
    time(&start);
    char *table[filas][columnas];
    int coloredMatrix[filas][columnas];
    for(int i = 0; i <filas; i++){
        for(int j = 0; j <columnas; j++){
            random1 = rand() % 10 + 1;
            table[i][j] = malloc(20);
            coloredMatrix[i][j] = 0;
            snprintf(table[i][j], 20, "[%2d]",random1);
        }
    }
    while (timeOfTimer) {
        random1 = rand() %10;
        random2 = rand() %10;
        time(&now);
        printf("Timer: %d\n", timeOfTimer);
        if(difftime(now,start) >=1){
            start = now;
            timeOfTimer--;
        }
        if(coloredMatrix[random1][random2] == 0){
            coloredMatrix[random1][random2] = 1;
            char aux[20];
            strcpy(aux, table[random1][random2]);
            snprintf(table[random1][random2],30, "\033[41m%s\033[0m",aux);
            drawBoard(filas,columnas,table);
            usleep(500000);
        }
        clearScreen();
    }
    printf("Game ended\n");
    usleep(1000000);
    clearScreen();
    for(int i = 0; i<filas; i++){
        for(int j = 0; j < columnas; j++){
            free(table[i][j]);
        }
    }
    return 0;
}

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}

void drawBoard(int filas, int columnas, char *table[filas][columnas]) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            printf("%s",table[i][j]);
        }
        printf("\033[0m\n"); // Restaura el color por defecto
    }
}

void delay(int milliseconds){
    clock_t start_time = clock();
    while(clock() < start_time + milliseconds * (CLOCKS_PER_SEC /1000));
}