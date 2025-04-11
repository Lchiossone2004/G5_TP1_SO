# G5_TP1_SO

TP 1 de Sistems Operativos 1Q 2025

Grupo 5:
alanari@itba.edu.ar - 64679
lchiossone@itba.edu.ar - 64359
lpercich@itba.edu.ar - 64316

# ChompChamps

This project includes several C components that interact via shared memory, designed to work with the **ChompChamps** game. It includes executables like `bot`, `view` and `master`, along with rules to compile, run, and clean the project.

## Project Structure

- `src/`: Contains the C source files.
- `include/`: Contains header files.
- `binaries/`: Output directory for compiled binaries.
- `sharedMem.c`: Common module used by all executables.

## Compilation

To compile all binaries (`bot`, `view`, `master`):

make build

## Run

In order to run the given ChompChamps:

```bash
make run
```

By default:
BOTS=1 (from binaries/bot)
WIDTH=10
HEIGHT=10
DELAY=200
TIMEOUT=10
SEED=time(NULL)
VIEW_ON=no

To customize this:

```bash
make run BOTS=b VIEW_ON=v WIDTH=w HEIGHT=h DELAY=d TIMEOUT=t SEED=s
```

Where:
b is the number of bots, which must be greater than 0 and less than 10.
v must be either yes or no.
w and h cannot be less than 10.
d is the number of milliseconds the master waits each time the game state is printed.
t is the timeout in seconds for receiving valid movement requests.
s is the seed used to generate the board.

In order to run master:

```bash
make run_nat
```

To customize:

```bash
make run_nat BOTS=b VIEW_ON=v WIDTH=w HEIGHT=h DELAY=d TIMEOUT=t SEED=s
```

Where:
b is the number of bots, which must be greater than 0 and less than 10.
v must be either yes or no.
w and h cannot be less than 10.
d is the number of milliseconds the master waits each time the game state is printed.
t is the timeout in seconds for receiving valid movement requests.
s is the seed used to generate the board.

## Cleaning

In order to remove all compiled binaries:

```bash
make clean
```

## Testing

Additionally, you can use a script to test both the provided master and the master developed by the group with different arguments.

```bash
./test/test_run.sh
```

The test results will be transcribed into a test_report.txt inside the test folder
