/*
 * Reaver - Main and reaver_usage functions
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

#include "wpscrack.h"
#include "iface.h"
#include "simulate.h"

extern const char* get_version(void);
static int reaver_usage(char *prog_name);

int reaver_main(int argc, char **argv)
{
	int ret_val = EXIT_FAILURE, r = 0;
	struct wps_data *wps = NULL;

	globule_init();
	init_default_settings();

	fprintf(stderr, "\nReaver v%s WiFi Protected Setup Attack Tool\n", get_version());
	fprintf(stderr, "Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>\n\n");

/*    fprintf(stderr, " >> PREAVER: %d\n", NUM_THREADS);*/

	if(argc < 2)
	{
		ret_val = reaver_usage(argv[0]);
		goto end;
	}

	/* Process the command line arguments */
	if(process_arguments(argc, argv) == EXIT_FAILURE)
	{
		ret_val = reaver_usage(argv[0]);
		goto end;
	}

	/* Sanity checking on the message timeout value */	
	if(get_m57_timeout() > M57_MAX_TIMEOUT) 
	{
		set_m57_timeout(M57_MAX_TIMEOUT);
	}
	else if(get_m57_timeout() <= 0)
	{
		set_m57_timeout(M57_DEFAULT_TIMEOUT);
	}

	/* Sanity checking on the receive timeout value */
	if(get_rx_timeout() <= 0)
	{
		set_rx_timeout(DEFAULT_TIMEOUT);
	}

	/* Initialize signal handlers */
	sigint_init();
	sigalrm_init();

	/* Mark the start time */
    double start_time = omp_get_wtime();

	/* Do it. */
	int result = crack();
	
	/* Mark the end time */
    double end_time = omp_get_wtime();

	/* Check our key status */
    int i;
    #pragma omp parallel for num_threads(NUM_THREADS)
    for(i = 0; i < NUM_THREADS; i++) {
        int t_id = omp_get_thread_num();
        if (t_id == result) {
            if(get_key_status() == KEY_DONE) {
                wps = get_wps();

                char *pin = get_pin();
                fprintf(stderr, "[+] WPS PIN: %s\n", pin);
                if(wps->key)      cprintf(CRITICAL, "[+] WPA PSK: '%s'\n", wps->key);
                if(wps->essid)    cprintf(CRITICAL, "[+] AP SSID: '%s'\n", wps->essid);

                /* Run user-supplied command */
                if(get_exec_string())
                {
                    r = system(get_exec_string());
                }

                ret_val = EXIT_SUCCESS;
            }
            else {
                cprintf(CRITICAL, "[-] Failed to recover WPA key\n");
            }
        }
    }
	
end:
	globule_deinit();
	return ret_val;
}

static int reaver_usage(char *prog_name)
{
        float fail_timeout = 0;

        fail_timeout = ((float) M57_DEFAULT_TIMEOUT / (float) SEC_TO_US);

        fprintf(stderr, "Required Arguments:\n");
        fprintf(stderr, "\t-i, --interface=<wlan>          Name of the monitor-mode interface to use\n");
        fprintf(stderr, "\t-b, --bssid=<mac>               BSSID of the target AP\n");

        fprintf(stderr, "\nOptional Arguments:\n");
        fprintf(stderr, "\t-m, --mac=<mac>                 MAC of the host system\n");
        fprintf(stderr, "\t-e, --essid=<ssid>              ESSID of the target AP\n");
        fprintf(stderr, "\t-c, --channel=<channel>         Set the 802.11 channel for the interface (implies -f)\n");
	fprintf(stderr, "\t-s, --session=<file>            Restore a previous session file\n");
	fprintf(stderr, "\t-C, --exec=<command>            Execute the supplied command upon successful pin recovery\n");
        fprintf(stderr, "\t-f, --fixed                     Disable channel hopping\n");
        fprintf(stderr, "\t-5, --5ghz                      Use 5GHz 802.11 channels\n");
        fprintf(stderr, "\t-v, --verbose                   Display non-critical warnings (-vv or -vvv for more)\n");
        fprintf(stderr, "\t-q, --quiet                     Only display critical messages\n");
        fprintf(stderr, "\t-h, --help                      Show help\n");
        
        fprintf(stderr, "\nAdvanced Options:\n");
	fprintf(stderr, "\t-p, --pin=<wps pin>             Use the specified pin (may be arbitrary string or 4/8 digit WPS pin)\n");
	fprintf(stderr, "\t-d, --delay=<seconds>           Set the delay between pin attempts [%d]\n", DEFAULT_DELAY);
        fprintf(stderr, "\t-l, --lock-delay=<seconds>      Set the time to wait if the AP locks WPS pin attempts [%d]\n", DEFAULT_LOCK_DELAY);
        fprintf(stderr, "\t-g, --max-attempts=<num>        Quit after num pin attempts\n");
        fprintf(stderr, "\t-x, --fail-wait=<seconds>       Set the time to sleep after %d unexpected failures [0]\n", WARN_FAILURE_COUNT);
        fprintf(stderr, "\t-r, --recurring-delay=<x:y>     Sleep for y seconds every x pin attempts\n");
        fprintf(stderr, "\t-t, --timeout=<seconds>         Set the receive timeout period [%d]\n", DEFAULT_TIMEOUT);
        fprintf(stderr, "\t-T, --m57-timeout=<seconds>     Set the M5/M7 timeout period [%.2f]\n", fail_timeout);
	fprintf(stderr, "\t-A, --no-associate              Do not associate with the AP (association must be done by another application)\n");
	fprintf(stderr, "\t-N, --no-nacks                  Do not send NACK messages when out of order packets are received\n");
	fprintf(stderr, "\t-S, --dh-small                  Use small DH keys to improve crack speed\n");
        fprintf(stderr, "\t-L, --ignore-locks              Ignore locked state reported by the target AP\n");
        fprintf(stderr, "\t-E, --eap-terminate             Terminate each WPS session with an EAP FAIL packet\n");
        fprintf(stderr, "\t-J, --timeout-is-nack           Treat timeout as NACK (DIR-300/320)\n");
        fprintf(stderr, "\t-F, --ignore-fcs                Ignore frame checksum errors\n");
	fprintf(stderr, "\t-w, --win7                      Mimic a Windows 7 registrar [False]\n");
	fprintf(stderr, "\t-K, --pixie-dust                Run pixiedust attack\n");
	fprintf(stderr, "\t-Z                              Run pixiedust attack\n");
        fprintf(stderr, "\t-O, --output-file=<filename>    Write packets of interest into pcap file\n");

        fprintf(stderr, "\nExample:\n\t%s -i wlan0mon -b 00:90:4C:C1:AC:21 -vv\n\n", prog_name);

        return EXIT_FAILURE;
}

