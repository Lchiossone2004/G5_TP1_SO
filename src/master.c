#include "master.h"

int main(int argc, char *argv[])
{
    int opt;
    int width = 10;
    int height = 10;
    int delay = 200;
    int timeout = 10;
    int seed = time(NULL);
    char *view = NULL;
    char *players[9] = {NULL};
    int current_player = 0;
    int players_added = 0;
    while ((opt = getopt(argc, argv, "p:w:h:d:t:s:v:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            if (!optarg || optarg[0] == '-')
            {
                perror("Must add player.");
                exit(EXIT_FAILURE);
            }
            players[players_added++] = optarg;
            printf("Player added: %s\n", optarg);
            while (optind < argc && argv[optind][0] != '-')
            {
                players[players_added++] = argv[optind++];
                printf("Player added: %s\n", players[players_added - 1]);
            }
            break;
        case 'w':
            int width = atoi(optarg);
            if (width < 10)
            {
                perror("Invalid width value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            int height = atoi(optarg);
            if (height < 10)
            {
                perror("Invalid height value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            int delay = atoi(optarg);
            if (delay < 0)
            {
                perror("Invalid delay value, must be positive.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            int timeout = atoi(optarg);
            if (timeout < 10)
            {
                perror("Invalid timeout value, must be higher than 9.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            seed = atoi(optarg);
            break;
        case 'v':
            view = optarg;
            break;
        case '?':
            if (optopt == 'p' || optopt == 'w' || optopt == 'h' || optopt == 'd' || optopt == 't' || optopt == 's' || optopt == 'v')
            {
                fprintf(stderr, "Error: Option -%c requires a value.\n", optopt);
            }
            else
            {
                perror("Error: Invalid argument");
            }
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Error inesperado.\n");
            exit(EXIT_FAILURE);
        }
    }
    if (players_added == 0)
    {
        perror("Error: At least one player must be specified using -p.\n");
        exit(EXIT_FAILURE);
    }
    if (players_added > 9)
    {
        perror("Error: At most 9 players can be specified using -p.\n");
        exit(EXIT_FAILURE);
    }

    // Creation of shared memory

    int state_fd;
    GameState *state_map;
    int sync_fd;
    GameSync *sync_map;
    createMemory(&state_fd, &sync_fd, &state_map, &sync_map, width, height); // Creo la memoria compartida

    // Setting up the game
    system("clear");
    printf("Bord--> Width: %d | Height: %d\n", width, height);
    printf("Game Settings--> Delay: %d | Seed: %d | View %s\n", delay, seed, view);
    printf("Number of players--> %d\n", players_added);
    for (int i = 0; i < players_added; i++)
    {
        printf("Player: %s\n", players[i]);
    }
    sleep(3);
    // Start the Semaphores
    semaphoreStary(sync_map);

    // Creating the board
    srand(seed); // Set the inputed seed, if it's not inputed it will be time(NULL)
    fillBoard(width, height, state_map);
    state_map->board_height = height;
    state_map->board_width = width;
    state_map->game_ended = false;

    // Creation of child procesess

    // Creation of the view

    if (view != NULL)
    { // Seteo de view
        printf("hay view\n");
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
        if (pid == 0)
        {
            execv(view, args_list);
            perror("Execv fail.\n");
            exit(EXIT_FAILURE);
        }
    }

    // Creation of the pipes for players (one for each)

    int pipes[players_added][2];
    int max_fd = 0;
    for (int i = 0; i < players_added; i++)
    { // Creacion de pipdes para los hijos
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

    // Creation of the players
    createPlayers(state_map, players_added, width, height, players, pipes); // Creo los jugadores
    state_map->num_of_players = players_added;

    // Pipe management
    struct timeval time_out; // Estructuras necesarias para el chequeo de pipdes y juego
    time_out.tv_sec = timeout;
    time_out.tv_usec = 0;
    int priority = 0;
    bool all_blocked = false;
    bool game_ended = false;
    // Pre game
    system("clear");
    // Game Cycle
    if (view != NULL)
    {
        sem_post(&sync_map->to_print); // imagein inicial
        sem_wait(&sync_map->end_print);
    }
    while (!game_ended)
    {
        Request request = checkRequest(time_out, players_added, pipes, max_fd, &current_player); // Busca una request
        sem_wait(&sync_map->master_mutex);                                                       // Esto es lo que dijo el profe para que no se desbarate todo
        sem_wait(&sync_map->state_mutex);
        sem_post(&sync_map->master_mutex);

        if (processRequest(request, state_map) != 0)
        { // Procesamos la request y vemos si es un movimiento valido o no
            state_map->players_list[request.player_num].invalid_moves++;
        }
        else
        {
            state_map->players_list[request.player_num].valid_moves++;
        }
        all_blocked = true;
        for (int player = 0; player < players_added; player++)
        { // Chequeamos si todos los jugadores estan bloqueados
            isBlocked(state_map, player);
            if (!state_map->players_list[player].is_blocked)
            {
                all_blocked = false;
            }
        }
        if (all_blocked == true)
        { // Si estan bloqueados termina el juego
            game_ended = true;
        }
        if (view != NULL)
        {
            sem_post(&sync_map->to_print); // Y estos dos semaforos son para el print y el ultimo libera el game state para los players
            sem_wait(&sync_map->end_print);
        }
        sem_post(&sync_map->state_mutex);
    }
    if (view != NULL)
    {
        sem_wait(&sync_map->state_mutex); // Esto ultimo es para que la view pueda exitear de manera corecta y no se quede trabada
        state_map->game_ended = true;
        sem_post(&sync_map->state_mutex);
        sem_post(&sync_map->to_print);
        sem_wait(&sync_map->end_print);
    }

    for (int i = 0; i < players_added; i++)
    { // Espera que todos los procesos hijos terminen
        wait(NULL);
    }
    if (view != NULL)
    { // Si la view esta prendida la tengo en cuenta para esperarla
        wait(NULL);
    }

    // Cleaning

    clearMemory(state_map, sync_map, state_fd, sync_fd, width, height); // Clears and closes the shared memory
    return 0;
}

void isBlocked(GameState *state_map, int player_number)
{ // Chequea si el jugador esta bloqueado
    bool blocked = true;
    for (int fil = -1; fil < 2; fil++)
    {
        for (int col = -1; col < 2; col++)
        {
            int x = state_map->players_list[player_number].pos_x;
            int y = state_map->players_list[player_number].pos_y;
            if ((fil != 0 || col != 0) && isValid(fil + y, col + x, state_map))
            {
                return;
            }
        }
    }
    state_map->players_list[player_number].is_blocked = blocked;
}

int processRequest(Request request, GameState *state_map)
{ // Agarra la request y actualiza la psoicion del jugador y el tablero
    const int delta_x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int delta_y[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int new_x = state_map->players_list[request.player_num].pos_x + delta_x[request.direction];
    int new_y = state_map->players_list[request.player_num].pos_y + delta_y[request.direction];
    if (isValid(new_y, new_x, state_map))
    {
        state_map->players_list[request.player_num].pos_x = new_x;
        state_map->players_list[request.player_num].pos_y = new_y;
        state_map->board_origin[new_y * state_map->board_width + new_x] = (-1) * request.player_num;
        return 0;
    }
    else
    {
        return 0;
    }
}

Request checkRequest(struct timeval time_out, int players_added, int (*pipes)[2], int max_fd, int *current_player)
{                                                          // Mira los pipes y busca requests (aca es donde falta el tema de un orden justo)
    Request request = {.direction = -1, .player_num = -1}; // Inicializa correctamente
    fd_set read_fds;
    FD_ZERO(&read_fds); // Setting up the pipe list for the select
    if (pipes[*current_player][0] != -1)
    { // Solo agregar si el descriptor sigue abierto
        FD_SET(pipes[*current_player][0], &read_fds);
    }

    int act = select(max_fd + 1, &read_fds, NULL, NULL, &time_out); // Select for each player (checking each player pipe)
    if (act == -1)
    {
        perror("Error makeing the select");
        exit(EXIT_FAILURE);
    }
    else if (act == 0)
    {
        request.direction = -1;
        request.player_num = -1;
        printf("Timeout\n");
    }
    else
    {

        if (FD_ISSET(pipes[*current_player][0], &read_fds))
        { // Checks if the current player has written something on his pipe
            int direc;
            memset(&direc, 0, sizeof(direc));
            int readed = read(pipes[*current_player][0], &direc, sizeof(direc));
            if (readed == 0)
            {
                printf("Pipe cerrado por el jugador %d\n", *current_player);
                close(pipes[*current_player][0]);
                FD_CLR(pipes[*current_player][0], &read_fds);
                pipes[*current_player][0] = -1;
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
    *current_player = (*current_player + 1) % players_added; // itero el jugador (si era el último, vuelvo al primero)
    return request;
}

void semaphoreStary(GameSync *sync_map)
{ // Inicializo los semaforos
    sem_init(&sync_map->to_print, 1, 0);
    sem_init(&sync_map->end_print, 1, 0);
    sem_init(&sync_map->master_mutex, 1, 1);
    sem_init(&sync_map->state_mutex, 1, 1);
    sem_init(&sync_map->reader_mutex, 1, 1);
    sync_map->readers_counter = 0;
}

void fillBoard(int width, int height, GameState *state_map)
{ // Llenar el tablero
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            state_map->board_origin[i * width + j] = (rand() % 9) + 1; // para que quede entre 0 y 9
        }
    }
}

void createPlayers(GameState *state_map, int players_added, int width, int height, char **players, int (*pipes)[2])
{                                                                                                   // Creacion de los jugadores
    int start_pos[9][2] = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}}; // Array de posiciones iniciales
    char w[10];
    char h[10];

    snprintf(w, sizeof(w), "%d", width); // Usar snprintf para evitar overflow
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
        { // Proceso hijo
            // Configurar parámetros del jugador
            char *args_list[] = {players[i], w, h, NULL};
            state_map->players_list[i].is_blocked = false;
            strcpy(state_map->players_list[i].player_name, players[i]); // cargo los parametros iniciles del jugador
            int x = state_map->players_list[i].pos_x = start_pos[i][0];
            int y = state_map->players_list[i].pos_y = start_pos[i][1];
            state_map->board_origin[state_map->board_width * y + x] = i * (-1);
            close(pipes[i][0]);               // Cierro la lectura de los pipes para los hijos
            dup2(pipes[i][1], STDOUT_FILENO); // Redirijo la salida standar al pipe osea fd = 1
            execv(players[i], args_list);     // Llamo al archivo del hijo
            perror("Player execv fail.\n");
            exit(EXIT_FAILURE);
        }
        close(pipes[i][1]);                          // Cierro el lado de la escritura del pipe para el padre
        state_map->players_list[i].player_pid = pid; // Guardo el pid del player en su lugar
    }
}

int isValid(int y, int x, GameState *state_map)
{ // Chequeo si la posicion esta en el tablero y si es un movimiento valido
    return x >= 0 && x < state_map->board_width &&
           y >= 0 && y < state_map->board_height && state_map->board_origin[y * state_map->board_width + x] > 0;
}