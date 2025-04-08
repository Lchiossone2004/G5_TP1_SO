#include "masterLib.h"

void isBlocked(GameState *state_map, int player_number)
{
    state_map->players_list[player_number].is_blocked = true;
    for (int fil = -1; fil < 2; fil++)
    {
        for (int col = -1; col < 2; col++)
        {
            int x = state_map->players_list[player_number].pos_x;
            int y = state_map->players_list[player_number].pos_y;
            if ((fil != 0 || col != 0) && isValid(fil + y, col + x, state_map))
            {
                state_map->players_list[player_number].is_blocked = false;
            }
        }
    }
}

int processRequest(Request request, GameState *state_map)
{ // Updates board and player's position
    const int delta_x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int delta_y[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int new_x = state_map->players_list[request.player_num].pos_x + delta_x[request.direction];
    int new_y = state_map->players_list[request.player_num].pos_y + delta_y[request.direction];
    if (isValid(new_y, new_x, state_map))
    {
        state_map->players_list[request.player_num].pos_x = new_x;
        state_map->players_list[request.player_num].pos_y = new_y;
        state_map->players_list[request.player_num].score += state_map->board_origin[new_y * state_map->board_width + new_x];
        state_map->board_origin[new_y * state_map->board_width + new_x] = (-1) * request.player_num;
        return 0;
    }
    else
    {
        return 0;
    }
}

Request checkRequest(struct timeval time_out, int players_added, int (*pipes)[2], int max_fd, int *current_player)
{ // Checks pipes and looks for requests (aca es donde falta el tema de un orden justo)
    Request request = {.direction = -1, .player_num = -1};
    fd_set read_fds;
    FD_ZERO(&read_fds); // Setting up the pipe list for the select
    for (int i = 0; i < players_added; i++)
    { // Add only if file descriptor is open
        FD_SET(pipes[i][0], &read_fds);
    }
    int act = select(max_fd + 1, &read_fds, NULL, NULL, &time_out); // Select for each player (checking each player pipe)
    if (act == -1)
    {
        perror("Error makeing the select");
        exit(EXIT_FAILURE);
    }
    else if (act == 0)
    {
        request.direction = -2;
        request.player_num = -2;
        printf("Timeout\n");
        return request;
    }
    else
    {
        if (pipes[*current_player][0] != -1 && FD_ISSET(pipes[*current_player][0], &read_fds))
        { // Checks if the current player has written something on his pipe
            unsigned char direc;
            memset(&direc, 0, sizeof(direc));
            int readed = read(pipes[*current_player][0], &direc, sizeof(direc));
            if (readed == 0)
            {
            }
            else if (readed < 0)
            {
                perror("Error reading the buffer\n");
            }
            else
            {
                request.direction = direc;
                request.player_num = *current_player;
            }
        }
    }
    *current_player = (*current_player + 1) % players_added; // Iteration of player (if last, goes back to first)
    return request;
}

void semaphoreStary(GameSync *sync_map)
{ // Initialize semaphores
    sem_init(&sync_map->to_print, 1, 0);
    sem_init(&sync_map->end_print, 1, 0);
    sem_init(&sync_map->master_mutex, 1, 1);
    sem_init(&sync_map->state_mutex, 1, 1);
    sem_init(&sync_map->reader_mutex, 1, 1);
    sync_map->readers_counter = 0;
}

void fillBoard(int width, int height, GameState *state_map)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            state_map->board_origin[i * width + j] = (rand() % 9) + 1; // So it's between 0 and 9
        }
    }
}

void createPlayers(GameState *state_map, int players_added, int width, int height, char **players, int (*pipes)[2], int error_report[2])
{
    int start_pos[9][2] = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}}; // Initial positions
    char w[10];
    char h[10];

    snprintf(w, sizeof(w), "%d", width); // Snprintf to avoid overflow
    snprintf(h, sizeof(h), "%d", height);

    for (int i = 0; i < players_added; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Error creating child process\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        { // Child process
            close(error_report[0]);
            // Player's parameters
            char *args_list[] = {players[i], w, h, NULL};
            state_map->players_list[i].is_blocked = false;
            strcpy(state_map->players_list[i].player_name, players[i]);
            int x = state_map->players_list[i].pos_x = start_pos[i][0];
            int y = state_map->players_list[i].pos_y = start_pos[i][1];
            state_map->board_origin[state_map->board_width * y + x] = i * (-1);
            state_map->players_list[i].score = 0;
            close(pipes[i][0]);
            dup2(pipes[i][1], STDOUT_FILENO); // Redirect standard output to pipe
            execv(players[i], args_list);     // Call child's file
            perror("Player execv fail");
            int error = -1;
            write(error_report[1], &error, sizeof(error));
            exit(EXIT_FAILURE);
        }
        close(pipes[i][1]);
        state_map->players_list[i].player_pid = pid;
    }
}

int isValid(int y, int x, GameState *state_map)
{
    return x >= 0 && x < state_map->board_width &&
           y >= 0 && y < state_map->board_height && state_map->board_origin[y * state_map->board_width + x] > 0;
}