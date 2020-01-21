/*
 * Reaver - Main cracking functions
 * Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 */

#include <omp.h>
#include "cracker.h"
#include "utils/vendor.h"
#include "simulate.h"

/* Brute force all possible WPS pins for a given access point */
int crack()
{
	char *bssid = NULL;
	time_t start_time = 0;
	enum wps_result result = 0;
    int the_chosen_one = 0;

    set_random_pin();

	if(get_max_pin_attempts() == -1)
	{
		cprintf(CRITICAL, "[X] ERROR: This device has been blacklisted and is not supported.\n");
		return -1;
	}

	/* Initialize network interface */
		generate_pins();

		/* Convert BSSID to a string */
		bssid = "00:00:00:00:00:00";

		/* 
		 * We need to get some basic info from the AP, and also want to make sure the target AP
		 * actually exists, so wait for a beacon packet 
		 */
		char *vendor;
		if((vendor = get_vendor_string(get_vendor())))
			cprintf(INFO, "[+] Vendor: %s\n", vendor);
		if(get_max_pin_attempts() == -1)
		{
			cprintf(CRITICAL, "[X] ERROR: This device has been blacklisted and is not supported.\n");
			return -1;
		}

		/* Used to calculate pin attempt rates */
		start_time = time(NULL);

		/* If the key status hasn't been explicitly set by restore_session(), ensure that it is set to KEY1_WIP */
		if(get_key_status() <= KEY1_WIP)
		{
			set_key_status(KEY1_WIP);
		}
		/* 
		 * If we're starting a session at KEY_DONE, that means we've already cracked the pin and the AP is being re-attacked.
		 * Re-set the status to KEY2_WIP so that we properly enter the main cracking loop.
		 */
		else if(get_key_status() == KEY_DONE)
		{
			set_key_status(KEY2_WIP);
		}

		/* Main cracking loop */
        int KEY_COUNT = 10000; // 10^4 + 10^3

        char *t_pin[NUM_THREADS] = {0};
        int t_loop_count[NUM_THREADS] = {0};
        int t_sleep_count[NUM_THREADS]= {0};
        int t_assoc_fail_count[NUM_THREADS] = {0};
        int t_fail_count[NUM_THREADS] = {0};
        int t_pin_count[NUM_THREADS] = {0};
        enum wps_result t_result[NUM_THREADS] = {0};

        int t_key_done[NUM_THREADS] = {KEY1_WIP};

        int STOP = 0;

        double START_TIME = omp_get_wtime();

        fprintf(stderr, "starting parallel for, n=%d\n", NUM_THREADS);
        int i;
        #pragma omp parallel for num_threads(NUM_THREADS) 
        for(i = 0; i < KEY_COUNT; i++) {
            
            int t_id = omp_get_thread_num();

            // #pragma omp cancel didn't quite work here
            if((t_key_done[t_id] != 0 && t_key_done[t_id] != 1) || STOP) {
                continue;
            }

            /* took out rating limiting with a chainsaw */

			/* Initialize wps structure */
			set_wps(initialize_wps_data());
			if(!get_wps()) {
				cprintf(CRITICAL, "[-] Failed to initialize critical data structure\n");
                #pragma omp critical
                STOP = 1;
			}

			/* Try the next pin in the list */
			t_pin[t_id] = build_next_pin();

			if(!t_pin[t_id]) {
				cprintf(CRITICAL, "[-] Failed to generate the next payload\n");
                #pragma omp critical
                STOP = 1;
			} else{
                fprintf(stdout, "[+][%d] Trying pin \"%s\"\n", t_id, t_pin[t_id]);
			}

            /* simulate WPS transaction */
            if (get_key_status() == KEY1_WIP) {
                t_result[t_id] = sim_do_wps_exchange(t_pin[t_id], 1);
            } else {
                t_result[t_id] = sim_do_wps_exchange(t_pin[t_id], 2);
            }

			switch(t_result[t_id])
			{
				case KEY_REJECTED:
					t_fail_count[t_id] = 0;
					t_pin_count[t_id]++;
					advance_pin_count();
					break;
				case KEY_ACCEPTED:
                    // first thread to find p1, tells others to "stop"
                    if (get_key_status() == KEY1_WIP) {
                        set_key_status(KEY2_WIP);
                        #pragma omp critical
                        memset(t_key_done, 2, sizeof(t_key_done));
                        t_key_done[t_id] = KEY2_WIP;
                        #pragma omp critical
                        the_chosen_one = t_id;
                        KEY_COUNT = 1000;
                        i = 0;
                        fprintf(stdout, "[%d] GOT P1\n", t_id);
                    } else {
                        set_key_status(KEY_DONE);
                        t_key_done[t_id] = KEY_DONE;
                        fprintf(stdout, "[%d] GOT P2\n", t_id);
                        #pragma omp critical
                        STOP = 1;
                    }
					break;
				/* Unexpected timeout or EAP failure...try this pin again */
				default:
					cprintf(VERBOSE, "[!][%d] WPS transaction failed (code: 0x%.2X), re-trying last pin\n", t_id, t_result[t_id]);
					t_fail_count[t_id]++;
					break;
			}

			/* If we've had an excessive number of message failures in a row, print a warning */
			if(t_fail_count[t_id] == WARN_FAILURE_COUNT) {
				cprintf(WARNING, "[!][%d] WARNING: %d failed connections in a row\n", t_id, t_fail_count[t_id]);
				t_fail_count[t_id] = 0;
				pcap_sleep(get_fail_delay());
			}

			/* Display status and save current session state every DISPLAY_PIN_COUNT loops */
			if(t_loop_count[t_id] == DISPLAY_PIN_COUNT) {
                #pragma omp critical
				display_status(t_pin_count[t_id], start_time);
				t_loop_count[t_id] = 0;
			}

			/* 
			 * The WPA key and other settings are stored in the globule->wps structure. If we've 
			 * recovered the WPS pin and parsed these settings, don't free this structure. It 
			 * will be freed by wpscrack_free() at the end of main().
			 */
			if(get_key_status() != KEY_DONE) {
				wps_deinit(get_wps());
				set_wps(NULL);
			/* If we have cracked the pin, save a copy */
		    } else {
				set_pin(t_pin[t_id]);
			}

			free(t_pin[t_id]);
			t_pin[t_id] = NULL;

			/* If we've hit our max number of pin attempts, quit */
			if((get_max_pin_attempts() > 0) &&  (t_pin_count[t_id] == get_max_pin_attempts())) {
				cprintf(VERBOSE, "[+][%d] Quitting after %d crack attempts\n", t_id, get_max_pin_attempts());
                #pragma omp critical
                STOP = 1;
			}

            t_loop_count[t_id]++;
            t_sleep_count[t_id]++;
		}

		if(get_handle())
		{
			pcap_close(get_handle());
			set_handle(NULL);
		}

        double END_TIME = omp_get_wtime();
        fprintf(stderr, "[+] Time Taken: %f\n", END_TIME - START_TIME);

    return the_chosen_one;
}

/* 
 * Increment the index into the p1 or p2 array as appropriate.
 * If we're still trying to brute force the first half, increment p1.
 * If we're working on the second half, increment p2.
 */
void advance_pin_count()
{
	if(get_key_status() == KEY1_WIP)
	{
		set_p1_index(get_p1_index() + 1);
	} 
	else if(get_key_status() == KEY2_WIP)
	{
		set_p2_index(get_p2_index() + 1);
	}
}

/* Displays the status and rate of cracking */
void display_status(float pin_count, time_t start_time)
{
	float percentage = 0;
	int attempts = 0, average = 0;
	time_t now = 0, diff = 0;
	struct tm *tm_p = NULL;
        char time_s[256] = { 0 };

	if(get_key_status() == KEY1_WIP)
	{
		attempts = get_p1_index() + get_p2_index();
	}
	/* 
	 * If we've found the first half of the key, then the entire key1 keyspace
	 * has been exhausted/eliminated. Our output should reflect that.
	 */
	else if(get_key_status() == KEY2_WIP)
	{
		attempts = P1_SIZE + get_p2_index();
	}
	else if(get_key_status() == KEY_DONE)
	{
		attempts = P1_SIZE + P2_SIZE;
	}

	percentage = (float) (((float) attempts / (P1_SIZE + P2_SIZE)) * 100);
	
	now = time(NULL);
	diff = now - start_time;

        tm_p = localtime(&now);
	if(tm_p)
	{
        	strftime(time_s, sizeof(time_s), TIME_FORMAT, tm_p);
	}
	else
	{
		perror("localtime");
	}

	if(pin_count > 0)
	{
		average =  (int) (diff / pin_count);
	}
	else
	{
		average = 0;
	}

	cprintf(INFO, "[+] %.2f%% complete @ %s (%d seconds/pin)\n", percentage, time_s, average);

	return;
}
