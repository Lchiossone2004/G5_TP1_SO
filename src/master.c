#include "masterLib.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    bool should_cleanup = true;
    int current_player = 0;
    int width, height, delay, timeout, seed;
    char *view;
    char *players[9] = {NULL};
    int players_added = 0;

    processArguments(argc, argv, &width, &height, &delay, &timeout, &seed, &view, players, &players_added);

    // Creation of shared memory

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    createMemory(&state_fd, &sync_fd, &state_map, &sync_map, width, height);
    semaphoreStary(sync_map); // Semaphores initializer
    // Setting up the game
    system("clear");
    printf("Bord--> Width: %d | Height: %d\n", width, height);
    printf("Game Settings--> Delay: %d | Seed: %d | View %s\n", delay, seed, view);
    printf("Number of players--> %d\n", players_added);
    for (int i = 0; i < players_added; i++)
    {
        printf("Player: %s\n", players[i]);
    }

    // Creating the board
    srand(seed); // Set the inputed seed, if it's not inputed it will be time(NULL)
    fillBoard(width, height, state_map);
    state_map->board_height = height;
    state_map->board_width = width;
    state_map->game_ended = false;

    // Creation of child procesess

    // Creation of the pipes for players (one for each)

    int pipes[players_added][2];
    int max_fd = 0;
    for (int i = 0; i < players_added; i++)
    { // Creation of the pipes for children
        if (pipe(pipes[i]) == -1)
        {
            perror("Failure creating pipe.\n");
            return 1;
        }
        if (pipes[i][0] > max_fd)
        {
            max_fd = pipes[i][0];
        }
    }

    int error_report[2]; // To see if any file fails
    if (pipe(error_report) == -1)
    {
        perror("Error creating error_report pipe\n");
        exit(EXIT_FAILURE);
    }

    // Creation of the players
    createPlayers(state_map, players_added, width, height, players, pipes, error_report);
    state_map->num_of_players = players_added;
    bool invalid_input = false;

    // Creation of the view

    if (view != NULL)
    { // Setting view
        char w[10];
        char h[10];
        sprintf(w, "%d", width);
        sprintf(h, "%d", height);
        char *args_list[] = {view, w, h, NULL};
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error connecting the view\n");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            close(error_report[0]);
            execv(view, args_list);
            perror("View execv fail");
            int error = -1;
            ssize_t bytes_written = write(error_report[1], &error, sizeof(error));
            if (bytes_written == -1)
            {
                perror("Write to pipe failed");
            }
            exit(EXIT_FAILURE);
        }
    }
    sleep(2);
    // File path error management
    int flags = fcntl(error_report[0], F_GETFL, 0);
    fcntl(error_report[0], F_SETFL, flags | O_NONBLOCK); // Making it so that it does not block the process
    int error = 0;
    read(error_report[0], &error, sizeof(error));
    if (error == -1)
    {
        invalid_input = true;
        state_map->game_ended = true;
        sem_post(&sync_map->to_print);
    }
    close(error_report[1]); // Closing of the error pipe, no more child process will be created from this point foward
    // Pipe management
    struct timeval time_out; // Structs required for pipe checking and game logic
    time_out.tv_sec = timeout;
    time_out.tv_usec = 0;
    bool all_blocked = false;
    bool game_ended = false;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = delay * 1000000L; // 1 millisecond = 1,000,000 nanoseconds
    // Pre game
    if (!invalid_input)
    {
        system("clear");
    }
    // Game Cycle
    if (view != NULL && !invalid_input)
    {
        sem_post(&sync_map->to_print); // Initial image
        sem_wait(&sync_map->end_print);
    }

    while (!game_ended && !invalid_input)
    {
       
        nanosleep(&ts, NULL); // Short delay

        Request request = checkRequest(time_out, players_added, pipes, max_fd, &current_player);

        sem_wait(&sync_map->master_mutex);
        sem_wait(&sync_map->state_mutex);
        sem_post(&sync_map->master_mutex);
        if (request.player_num == -2)
        {
            // Timeout: no players answered
            game_ended = true;
            if (invalid_input)
            {
                perror("Invalid input.");
                should_cleanup = false;
                exit(EXIT_FAILURE);
            }
            return 0;
        }
        else if (request.player_num != -1)
        {
            if (processRequest(request, state_map) != 0)
            {
                state_map->players_list[request.player_num].invalid_moves++;
            }
            else
            {
                state_map->players_list[request.player_num].valid_moves++;
            }
        }
        

        // Checking if players are blocked

        if (view != NULL)
        {
            sem_post(&sync_map->to_print);
            sem_wait(&sync_map->end_print);
        }

        sem_post(&sync_map->state_mutex);
        all_blocked = true;
        for (int player = 0; player < players_added; player++)
        {
            isBlocked(state_map, player);
            if (!state_map->players_list[player].is_blocked)
            {
                all_blocked = false;
            }
        }
        if (all_blocked)
        {
            game_ended = true;
            state_map->game_ended = true;
        }
    }

    if (view != NULL && !invalid_input)
    {
        sem_wait(&sync_map->state_mutex); // So view can exit properly
        sem_post(&sync_map->state_mutex);
        sem_post(&sync_map->to_print);
        sem_wait(&sync_map->end_print);
    }
    int status;
    for (int i = 0; i < players_added; i++)
    { // Waits for the players to end
        wait(&status);
        printf("Player %d, exited with status [%d] wth a score of %3d. ", i, status, state_map->players_list[i].score);
        int valid = state_map->players_list[i].valid_moves;
        int invalid = state_map->players_list[i].invalid_moves;
        printf(" %2d moves, %2d valid, %2d invalid.\n", valid + invalid, valid, invalid);
    }
    if (view != NULL)
    { // Waits for the view to end
        wait(&status);
        printf("View exited with status [%d]\n", status);
    }
    if (!invalid_input)
    {
        printWinner(state_map, players_added);
    }
    // Cleaning
    if (should_cleanup) {
        clearMemory(state_map, sync_map, state_fd, sync_fd, width, height);
    }
    if (invalid_input)
    {
        perror("Invalid input.");
        exit(EXIT_FAILURE);
    }
    return 0;
}
