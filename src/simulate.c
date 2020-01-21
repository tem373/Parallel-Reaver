#include "simulate.h"

unsigned int get_checksum(unsigned int pin) {
	unsigned int accum = 0;
	while (pin) {
		accum += 3 * (pin % 10);
		pin /= 10;
		accum += pin % 10;
		pin /= 10;
	}

	return (10 - accum % 10) % 10;
}

char* gen_random_pin() {
     
    unsigned int upper_max = 9999;
    unsigned int lower_max = 999;
    unsigned int upper_min = 1000;
    unsigned int lower_min = 100;

    srand(time(0));
    unsigned int upper_rand = (rand() % (upper_max - upper_min + 1)) + upper_min;
    unsigned int lower_rand = (rand() % (lower_max - lower_min + 1)) + lower_min;

    unsigned int total_rand = (upper_rand * 1000) + lower_rand;

    //total_rand = 1000100;
    unsigned int cs = get_checksum(total_rand);
    total_rand *= 10;
    total_rand += cs;

    char *new_pin = malloc(sizeof(char) * 8);
    sprintf(new_pin, "%d", total_rand);

    return new_pin;

}

void set_random_pin() {
    char *new_pin = gen_random_pin();
    real_pin = new_pin;
}

char *substring(char *str, int start, int len) {
    char *substr = malloc(sizeof(char) * len);
    sprintf(substr, "%.*s", len, str+start);
    return substr;
}

int sim_do_wps_exchange(char *att_pin, int key_part) {
    int result;
    int t_id = omp_get_thread_num();

    if (key_part == 1) {
        char *real_p1 = substring(real_pin, 0, 4);
        char *att_p1  = substring(att_pin, 0, 4);
        result = strcmp(real_p1, att_p1);
    } else {
        char *real_p2 = substring(real_pin, 4, 3);
        char *att_p2  = substring(att_pin, 4, 3);
        result = strcmp(real_p2, att_p2);
    }


    if (result == 0) {
        return result;
    } else {
        return 1;
    }

}
