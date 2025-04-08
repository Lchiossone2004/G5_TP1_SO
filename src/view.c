#include <string.h> // For strcpy
#include "sharedMem.h"

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
        "\033[48;2;48;93;114m",   // Ocean blue
        "\033[48;2;104;165;184m", // Sky blue
        "\033[48;2;142;209;178m", // Mint green
        "\033[48;2;237;187;153m", // Soft peach
        "\033[48;2;244;223;125m", // Pastel yellow
        "\033[48;2;222;142;174m", // Dusty pink
        "\033[48;2;186;133;206m", // Lavender
        "\033[48;2;255;166;158m", // Light coral
        "\033[48;2;112;176;190m"  // Muted teal
    };
    const char *head_colors[] = {
        "\033[48;2;38;73;91m",    // Dark ocean blue
        "\033[48;2;83;132;147m",  // Dark sky blue
        "\033[48;2;113;167;142m", // Dark mint green
        "\033[48;2;189;150;122m", // Dark soft peach
        "\033[48;2;195;178;100m", // Dark pastel yellow
        "\033[48;2;178;114;139m", // Dark dusty pink
        "\033[48;2;149;106;165m", // Dark lavender
        "\033[48;2;204;133;126m", // Darker light coral
        "\033[48;2;89;141;152m"};
    const char *board_colors[] = {
        "\033[48;2;220;224;193m", // Pale sage (for odd cells)
        "\033[48;2;178;190;181m"  // Soft stone green (for even cells)
    };

    int position_value = 0; // To store what is in the cell
    int player_found = 0;   // If the player to be printed is found

    while (!state_map->game_ended)
    {
        clearScreen();
        sem_wait(&sync_map->to_print); // Tells master it is going to print
        for (int row = 0; row < height; row++)
        { // Rows
            for (int col = 0; col < width; col++)
            { // Columns
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
            printf("\n");
        }
        for (int i = 0; i < state_map->num_of_players; i++)
        {
            printf("%s  \x1b[0m ", colors[i]);                               // Prints the color of the player
            printf("Player: %s | ", state_map->players_list[i].player_name); // Prints player's name
            printf("Score: %3d | ", state_map->players_list[i].score);       // Prints player's score
            printf("Coordinates(x,y): (%d,%d) | ", state_map->players_list[i].pos_x, state_map->players_list[i].pos_y);
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
