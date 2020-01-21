#ifndef WPS_SIM
#define WPS_SIM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

char* gen_random_pin();
char *substring(char *str, int start, int len);

extern void set_random_pin();
extern int sim_do_wps_exchange(char *pin, int key_part);

char *real_pin;

#endif
