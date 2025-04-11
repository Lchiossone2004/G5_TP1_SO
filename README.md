# G5_TP1_SO
TP 1 de Sistems Operativos 1Q 2025 

Grupo 5:
alanari@itba.edu.ar
lchiossone@itba.edu.ar
lpercich@itba.edu.ar

# ChompChamps

This project includes several C components that interact via shared memory, designed to work with the **ChompChamps** game. It includes executables like `bot`, `view`, `master`, and `tbot`, along with rules to compile, run, and clean the project.

## Project Structure

- `src/`: Contains the C source files.
- `include/`: Contains header files.
- `binaries/`: Output directory for compiled binaries.
- `sharedMem.c`: Common module used by all executables.

## Compilation

To compile all binaries (`bot`, `view`, `master`, `tbot`):

make build

## Run

In order to run the given ChompChamps:
```bash
make run
```
By default there will be one bot from binaries/bot and visualization enabled.

To customize this:
```bash
make run BOTS=? VIEW_ON= (yes/no). Being ? the quantity of bots, must be lower than 10 (ten) and higher than 0 (cero).
```
In order to run master:
```bash
make run_nat
```
To customize:
```bash
make run_nat BOTS=? VIEW_ON= (yes/no). Being ? the quantity of bots, must be lower than 10 (ten) and higher than 0 (cero).
```
## Cleaning

In order to remove all compiled binaries:
```bash
make clean
```