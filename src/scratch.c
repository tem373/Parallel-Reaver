int KEY_COUNT = 11000; // 10^4 + 10^3

int t_loop_count[NUM_THREADS];
int t_sleep_count[NUM_THREADS];
int t_key_done[NUM_THREADS];

for(int i = 0; i < KEY_COUNT; i++) {
    int t_id = omp_get_thread_num();

    pcap_sleep(get_delay());




    t_loop_count[t_id]++;
    t_sleep_count[t_id]++;
    t_key_done[t_id] = get_key_status();
}
