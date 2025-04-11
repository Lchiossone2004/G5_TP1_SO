#include <string.h> // For strcpy
#include "sharedMem.h"
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void clearScreen(); // Cleans the screen

int main(int argc, char *argv[])
{
    int width = atoi(argv[1]);  // Playing board width
    int height = atoi(argv[2]); // Playing board height

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    openMemory(&state_fd, &sync_fd, &state_map, &sync_map, width, height);

    const char *colors[] = {
        "\033[48;2;48;93;114m",
        "\033[48;2;104;165;184m",
        "\033[48;2;142;209;178m",
        "\033[48;2;237;187;153m",
        "\033[48;2;244;223;125m",
        "\033[48;2;222;142;174m",
        "\033[48;2;186;133;206m",
        "\033[48;2;255;166;158m",
        "\033[48;2;112;176;190m"};
    const char *head_colors[] = {
        "\033[48;2;38;73;91m",
        "\033[48;2;83;132;147m",
        "\033[48;2;113;167;142m",
        "\033[48;2;189;150;122m",
        "\033[48;2;195;178;100m",
        "\033[48;2;178;114;139m",
        "\033[48;2;149;106;165m",
        "\033[48;2;204;133;126m",
        "\033[48;2;89;141;152m"};
    const char *board_colors[] = {
        "\033[48;2;220;224;193m",
        "\033[48;2;178;190;181m"};

    int position_value = 0; // To store what is in the cell
    int player_found = 0;   // If the player to be printed is found

    while (!state_map->game_ended)
    {
        clearScreen();
        sem_wait(&sync_map->to_print); // Tells master it is going to print
        printf("\t\t\t| CHOMP CHAMPS |\n");
        printf("\t\t\t-----------------\n");

        for (int row = -1; row < height; row++)
        { // Rows
            if (row != -1)
            {
                printf("R:%-2d ", row + 1);
            }
            else
            {
                printf("    ");
            }
            for (int col = 0; col < width; col++)
            { // Columns
                if (row == -1)
                {
                    printf(" C:%-2d ", col + 1);
                }
                else
                {
                    position_value = state_map->board_origin[col + row * width];
                    if (position_value <= 0)
                    {
                        for (int i = 0; i < state_map->num_of_players && !player_found; i++)
                        {
                            if (position_value == i * (-1))
                            {
                                if (col == state_map->players_list[i].pos_x && row == state_map->players_list[i].pos_y)
                                {
                                    printf("%s* %2d *\033[0m", head_colors[i], position_value); // Prints the player's head with color
                                }
                                else
                                {
                                    printf("%s  %2d  \033[0m", colors[i], position_value); // Prints the player with color
                                }
                                player_found = 1;
                            }
                        }
                    }
                    else
                    {
                        printf("%s  %2d  \033[0m", board_colors[(row + col) % 2], position_value);
                    }
                    player_found = 0;
                }
            }
            printf("\n");
        }
        for (int i = 0; i < state_map->num_of_players; i++)
        {
            printf("%s  \x1b[0m ", colors[i]);                                  // Prints the color of the player
            printf("Player: %-16s | ", state_map->players_list[i].player_name); // Prints player's name
            printf("Score: %3d | ", state_map->players_list[i].score);          // Prints player's score
            printf("Coordinates(x,y): (%-2d;%2d) | ", state_map->players_list[i].pos_x, state_map->players_list[i].pos_y);
            printf("%s\n", state_map->players_list[i].is_blocked ? "is blocked" : "");
        }
        sem_post(&sync_map->end_print); // Tells master it finished printing
    }

    closeMemory(state_map, sync_map, state_fd, sync_fd, width, height); // Cleaning

    return 0;
}

void clearScreen()
{
    printf("\033[H\033[J");
}
