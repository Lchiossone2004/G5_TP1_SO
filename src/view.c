#include <string.h>     // For strcpy
#include "sharedMem.h"

void clearScreen();                     // Cleans the screen

int main(int argc, char * argv[]) {
    int width = atoi(argv[1]);          //Playing board width
    int height = atoi(argv[2]);         //Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd,&sync_fd,&state_map,&sync_map,width,height);

    const char *colors[] = {
        "\033[48;2;48;93;114m",   // Azul océano suave
        "\033[48;2;104;165;184m", // Azul celeste grisáceo
        "\033[48;2;142;209;178m", // Verde menta
        "\033[48;2;237;187;153m", // Durazno suave
        "\033[48;2;244;223;125m", // Amarillo pastel
        "\033[48;2;222;142;174m", // Rosa viejo
        "\033[48;2;186;133;206m", // Lavanda
        "\033[48;2;255;166;158m", // Coral claro
        "\033[48;2;112;176;190m"  // Verde azulado apagado
    };
    const char* head_colors[] = {
        "\033[48;2;38;73;91m",   // Azul océano suave oscuro
        "\033[48;2;83;132;147m", // Azul celeste grisáceo oscuro
        "\033[48;2;113;167;142m", // Verde menta oscuro
        "\033[48;2;189;150;122m", // Durazno suave oscuro
        "\033[48;2;195;178;100m", // Amarillo pastel oscuro
        "\033[48;2;178;114;139m", // Rosa viejo oscuro
        "\033[48;2;149;106;165m", // Lavanda oscura
        "\033[48;2;204;133;126m", // Coral claro oscuro
        "\033[48;2;89;141;152m"   // Verde azulado apagado oscuro
    };
    const char* board_colors[] = {
        "\033[48;2;220;224;193m",  // Verde grisáceo claro (casillas impares)
        "\033[48;2;178;190;181m"   // Verde piedra suave (casillas pares)
    };
    
    int position_value = 0;                                                 //To store what is i the cell
    int player_found = 0;                                                   //If the player to be printed is found 

    while(!state_map->game_ended){
        clearScreen();
        sem_wait(&sync_map->A);                                             //Tells master its going to print
        for(int row = 0; row < height; row++){                              //Rows
            for(int col = 0; col < width; col++){                           //Collums
                position_value = state_map->board_origin[col+row*width];
                if(position_value <= 0){
                    for(int i = 0; i < state_map->num_of_players && !player_found; i++){
                        if(position_value == i*(-1)){
                            if(col == state_map->players_list[i].pos_x && row == state_map->players_list[i].pos_y){
                                printf("%s* %2d *\033[0m",head_colors[i],position_value);                              //Prints the player head in color 
                            }
                            else{
                                printf("%s  %2d  \033[0m",colors[i],position_value);                              //Prints the player in color 
                            }
                            player_found = 1;
                        }
                    }
                }
                else{
                    printf("%s  %2d  \033[0m",board_colors[(row+col)%2],position_value);
                }
                player_found = 0;
        }
        printf("\n");
    }
    for(int i = 0; i < state_map->num_of_players; i++){
        printf("%s  \x1b[0m ",colors[i]);                                                                         //Prints the color of the plyer 
        printf("Player: %s | ", state_map->players_list[i].player_name);                                                //Prints player name
        printf("Score: %d | ", state_map->players_list[i].score);                                                       //Prints player score
        printf("Coordinates(x,y): (%d,%d)\n",state_map->players_list[i].pos_x,state_map->players_list[i].pos_y);
    }
        sem_post(&sync_map->B);                                                                                         //Tells master it finished printing 
    }
    
    closeMemory(state_map,sync_map,state_fd,sync_fd,width,height);                                                      //Cleaning 

    return 0;
}

void clearScreen() {
    printf("\033[H\033[J");
}
