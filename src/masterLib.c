#include "masterLib.h"

void processArguments(int argc, char *argv[], int *width, int *height, int *delay, int *timeout, int *seed, char **view, char **players, int *players_added)
{
    int opt;
    *width = 10;
    *height = 10;
    *delay = 200;
    *timeout = 10;
    *seed = time(NULL);
    *view = NULL;
    *players_added = 0;

    while ((opt = getopt(argc, argv, "p:w:h:d:t:s:v:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            if (!optarg || optarg[0] == '-')
            {
                fprintf(stderr, "Error: Must add player after -p.\n");
                exit(EXIT_FAILURE);
            }
            players[*players_added] = optarg;
            printf("Player added: %s\n", optarg);
            (*players_added)++;
            while (optind < argc && argv[optind][0] != '-' && *players_added < 9)
            {
                players[*players_added] = argv[optind++];
                printf("Player added: %s\n", players[*players_added]);
                (*players_added)++;
            }
            break;

        case 'w':
            *width = atoi(optarg);
            if (*width < 10)
            {
                fprintf(stderr, "Invalid width value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;

        case 'h':
            *height = atoi(optarg);
            if (*height < 10)
            {
                fprintf(stderr, "Invalid height value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;

        case 'd':
            *delay = atoi(optarg);
            if (*delay < 0)
            {
                fprintf(stderr, "Invalid delay value, must be positive.\n");
                exit(EXIT_FAILURE);
            }
            break;

        case 't':
            *timeout = atoi(optarg);
            if (*timeout < 10)
            {
                fprintf(stderr, "Invalid timeout value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;

        case 's':
            *seed = atoi(optarg);
            break;

        case 'v':
            *view = optarg;
            break;

        case '?':
            if (optopt == 'p' || optopt == 'w' || optopt == 'h' || optopt == 'd' || optopt == 't' || optopt == 's' || optopt == 'v')
            {
                fprintf(stderr, "Error: Option -%c requires a value.\n", optopt);
            }
            else
            {
                fprintf(stderr, "Error: Invalid argument.\n");
            }
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unexpected error while parsing arguments.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (*players_added == 0)
    {
        fprintf(stderr, "Error: At least one player must be specified using -p.\n");
        exit(EXIT_FAILURE);
    }

    if (*players_added > 9)
    {
        fprintf(stderr, "Error: At most 9 players can be specified using -p.\n");
        exit(EXIT_FAILURE);
    }
}

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
{ // Checks pipes and looks for requests
    Request request = {.direction = -1, .player_num = -1};
    fd_set read_fds;
    FD_ZERO(&read_fds); // Setting up the pipe list for the select
    for (int i = 0; i < players_added; i++)
    { // Add only if file descriptor is open
        if (pipes[i][0] != -1)
        {
            FD_SET(pipes[i][0], &read_fds); // Agregar descriptor de lectura al conjunto
            if (pipes[i][0] > max_fd)
            {
                max_fd = pipes[i][0]; // Actualizar el valor de max_fd si encontramos un descriptor más alto
            }
        }
    }
    int act = select(max_fd + 1, &read_fds, NULL, NULL, &time_out); // Select for each player (checking each player pipe)

    if (act == -1)
    {
        perror("Error making the select");
        exit(EXIT_FAILURE);
    }
    else if (act == 0)
    {
        request.direction = -2;
        request.player_num = -2;
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

void mapPositions(int x_orig, int y_orig, int r_orig, int c_orig, int r_new, int c_new, int *x_new, int *y_new)
{
    *x_new = (x_orig * c_new) / c_orig;
    *y_new = (y_orig * r_new) / r_orig;
}
void createPlayers(GameState *state_map, int players_added, int width, int height, char **players, int (*pipes)[2], int error_report[2])
{
    int start_pos[9][2] = {
        {8, 5}, {8, 7}, {6, 8}, {3, 8}, {2, 6}, {2, 4}, {3, 2}, {6, 2}, {8, 3}};

    int r_orig = 10, c_orig = 10;
    int r_new = height, c_new = width;

    int x_new, y_new;

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
            for (int j = 0; j < players_added; j++)
            {
                if (j != i)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            close(error_report[0]);
            close(error_report[1]);

            mapPositions(start_pos[i][0], start_pos[i][1], r_orig, c_orig, r_new, c_new, &x_new, &y_new);
            // Player's parameters
            char *args_list[] = {players[i], w, h, NULL};
            state_map->players_list[i].is_blocked = false;
            strcpy(state_map->players_list[i].player_name, players[i]);
            state_map->players_list[i].pos_x = x_new;
            state_map->players_list[i].pos_y = y_new;
            state_map->board_origin[state_map->board_width * y_new + x_new] = i * (-1);
            state_map->players_list[i].score = 0;
            close(pipes[i][0]);
            dup2(pipes[i][1], STDOUT_FILENO); // Redirect standard output to pipe
            close(pipes[i][1]);
            execv(players[i], args_list); // Call child's file
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

void printWinner(GameState *state_map, int players_added)
{
    int winner = 0;
    for (int i = 0; i < players_added; i++)
    {
        if (state_map->players_list[i].score > state_map->players_list[winner].score)
        {
            winner = i;
        }
        else if (state_map->players_list[i].score == state_map->players_list[winner].score)
        {
            if (state_map->players_list[i].valid_moves < state_map->players_list[winner].valid_moves)
            {
                winner = i;
            }
        }
    }
    printf("THE WINNER IS PLAYER (%d): %s\n", winner, state_map->players_list[winner].player_name);
}

void cleanSemaphores(GameSync *sync_map)
{
    sem_destroy(&sync_map->end_print);
    sem_destroy(&sync_map->master_mutex);
    sem_destroy(&sync_map->reader_mutex);
    sem_destroy(&sync_map->state_mutex);
    sem_destroy(&sync_map->to_print);
}