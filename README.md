# PREAVER - Parallelized Reaver
# Thomas Mason & Kabir Singh
# April 2019

# To install: download and install preaver folder
# Navigate to preaver/src
# run ./configure
# run make

# To run: ./reaver -i wlan0mon -c 3 -b 00:00:00:00:00:00 -vv
# This will run a simulated (and perfectly legal!) attack
# Output will be the final correct pin, the time taken in the main 
# cracking loop using omp wtime() and the thread that found the pin

# Files of particular interest:
# cracker.c wpscrack.c pins.c globule.c simulation.c
