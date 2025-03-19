#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>  // Para usleep()
void clearScreen();
void drawBoard(int filas, int columnas, char *table[filas][columnas]);

int main() {
    srand(time(NULL));
    int filas = 10;
    int columnas = 10;
    int random1;
    int random2;
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
    while (1) {
        for(int i = 0; i < 10; i++){
            random1 = rand() %10;
            random2 = rand() %10;
            if(coloredMatrix[random1][random2] == 0){
                coloredMatrix[random1][random2] = 1;
                char aux[20];
                strcpy(aux, table[random1][random2]);
                snprintf(table[random1][random2],30, "\033[41m%s\033[0m",aux);
                clearScreen();
                drawBoard(filas,columnas,table);
                usleep(500000);
            }
        }

    }
    return 0;
}

void clearScreen() {
    printf("\033[H\033[J"); // Limpia la pantalla
}

void drawBoard(int filas, int columnas, char *table[filas][columnas]) {
    clearScreen();
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            printf("%s",table[i][j]);
        }
        printf("\033[0m\n"); // Restaura el color por defecto
    }
}

