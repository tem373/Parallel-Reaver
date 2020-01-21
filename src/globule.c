/*
 * Reaver - Global variable access functions
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
#include "globule.h"

// Create an array of globals, indexed by p_id
// one global per process
struct globals **globules;


int globule_init()
{
	int ret = 0;

	globules = malloc(sizeof(struct globals*) * NUM_THREADS);
	if(globules)
	{
        int i;
        for (i = 0; i < NUM_THREADS; i++) {
            struct globals *global_i = malloc(sizeof(struct globals));
            globules[i] = global_i;
            memset(globules[i], 0, sizeof(struct globals));
            ret += 1;
            globules[i]->resend_timeout_usec = 200000;
            globules[i]->output_fd = -1;
        }

	}
	return ret;
}
void globule_deinit()
{
	int i = 0;

	if(globules)
	{
        int t_id;
        for(t_id = 0; t_id < NUM_THREADS; t_id++) {
            for(i=0; i<P1_SIZE; i++) {
                if(globules[t_id]->p1[i]) {
                    free(globules[t_id]->p1[i]);
                }
            }

            for(i=0; i<P2_SIZE; i++) {
                    if(globules[t_id]->p2[i]) {
                        free(globules[t_id]->p2[i]);
                    }
            }

            if(globules[t_id]->wps) wps_deinit(globules[t_id]->wps);
            if(globules[t_id]->handle) pcap_close(globules[t_id]->handle);
            if(globules[t_id]->pin) free(globules[t_id]->pin);
            if(globules[t_id]->iface) free(globules[t_id]->iface);
            if(globules[t_id]->ssid) free(globules[t_id]->ssid);
            if(globules[t_id]->session) free(globules[t_id]->session);
            if(globules[t_id]->static_p1) free(globules[t_id]->static_p1);
            if(globules[t_id]->static_p2) free(globules[t_id]->static_p2);
            if(globules[t_id]->fp) fclose(globules[t_id]->fp);
            if(globules[t_id]->exec_string) free(globules[t_id]->exec_string);

            if(globules[t_id]->output_fd != -1) close(globules[t_id]->output_fd);

            free(globules[t_id]);
        }

        free(globules);
	}
}

void set_log_file(FILE *fp)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->fp = fp;
}
FILE *get_log_file(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->fp;
}

void set_last_wps_state(int state)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->last_wps_state = state;
}
int get_last_wps_state()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->last_wps_state;
}

void set_session(char *value)  
{
	int t_id = omp_get_thread_num(); 
	if(globules[t_id]->session) free(globules[t_id]->session);
	globules[t_id]->session = (value) ? strdup(value) : NULL;
}
char *get_session()    
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->session;  
} 

void set_p1_index(int index)
{
	if(index < P1_SIZE)
	{
        int t_id = omp_get_thread_num();
		globules[t_id]->p1_index = index;
	}
}
int get_p1_index()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->p1_index;
}

void set_p2_index(int index)
{
	if(index < P2_SIZE)
	{
        int t_id = omp_get_thread_num();
		globules[t_id]->p2_index = index;
	}
}
int get_p2_index()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->p2_index;
}

void set_p1(int index, char *value)
{

    int t_id = omp_get_thread_num();
	if(index < P1_SIZE)
	{
		if(globules[t_id]->p1[index]) {
			free(globules[t_id]->p1[index]);
		}
		globules[t_id]->p1[index] = (value) ? strdup(value) : NULL;
	}
}
char *get_p1(int index)
{
	if(index < P1_SIZE)
	{
        int t_id = omp_get_thread_num();
		return globules[t_id]->p1[index];
	}
	return NULL;
}

void set_p2(int index, char *value)
{
	if(index < P2_SIZE) {
        int t_id = omp_get_thread_num();
		if(globules[t_id]->p2[index]) 
            free(globules[t_id]->p2[index]);

		globules[t_id]->p2[index] = (value) ? strdup(value) : NULL;
	}
}
char *get_p2(int index)
{
	if(index < P2_SIZE)
	{
        int t_id = omp_get_thread_num();
		return globules[t_id]->p2[index];
	}
	return NULL;
}

void set_key_status(enum key_state status)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->key_status = status;
}
enum key_state get_key_status()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->key_status;
}

void set_delay(int delay)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->delay = delay;
}
int get_delay()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->delay;
}

void set_fail_delay(int delay)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->fail_delay = delay;
}
int get_fail_delay()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->fail_delay;
}

void set_validate_fcs(int validate)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->validate_fcs = validate;
}
int get_validate_fcs(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->validate_fcs;
}

void set_recurring_delay(int delay)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->recurring_delay = delay;
}
int get_recurring_delay()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->recurring_delay;
}

void set_recurring_delay_count(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->recurring_delay_count = value;
}
int get_recurring_delay_count()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->recurring_delay_count;
}

void set_lock_delay(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->lock_delay = value;
}
int get_lock_delay()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->lock_delay;
}

void set_ignore_locks(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->ignore_locks = value;
}
int get_ignore_locks()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->ignore_locks;
}

void set_eap_terminate(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->eap_terminate = value;
}
int get_eap_terminate()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->eap_terminate;
}

void set_max_pin_attempts(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->max_pin_attempts = value;
}
int get_max_pin_attempts()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->max_pin_attempts;
}

void set_max_num_probes(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->max_num_probes = value;
}
int get_max_num_probes()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->max_num_probes;
}

void set_rx_timeout(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->rx_timeout = value;
}
int get_rx_timeout()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->rx_timeout;
}

void set_timeout_is_nack(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->timeout_is_nack = value;
}
int get_timeout_is_nack()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->timeout_is_nack;
}

void set_m57_timeout(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->m57_timeout = value;
}
int get_m57_timeout()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->m57_timeout;
}

void set_out_of_time(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->out_of_time = value;
}
int get_out_of_time()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->out_of_time;
}

void set_debug(enum debug_level value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->debug = value;
	if(value == DEBUG) wpa_debug_level = MSG_DEBUG;
}
enum debug_level get_debug()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->debug;
}

void set_eapol_start_count(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->eapol_start_count = value;
}
int get_eapol_start_count()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->eapol_start_count;
}

void set_fixed_channel(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->fixed_channel = value;
}
int get_fixed_channel()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->fixed_channel;
}

void set_auto_channel_select(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->auto_channel_select = value;
}
int get_auto_channel_select()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->auto_channel_select;
}

void set_wifi_band(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->wifi_band = value;
}
int get_wifi_band()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->wifi_band;
}

void set_opcode(enum wsc_op_code value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->opcode = value;
}
enum wsc_op_code get_opcode()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->opcode;
}

void set_eap_id(uint8_t value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->eap_id = value;
}
uint8_t get_eap_id()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->eap_id;
}

void set_ap_capability(uint16_t value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->ap_capability = value;
}
uint16_t get_ap_capability()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->ap_capability;
}

void set_channel(int channel)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->channel = channel;
}
int get_channel(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->channel;
}

void set_bssid(unsigned char *value)
{
	int t_id = omp_get_thread_num();
	memcpy(globules[t_id]->bssid, value, MAC_ADDR_LEN);
}
unsigned char *get_bssid()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->bssid;
}

void set_mac(unsigned char *value)
{
	int t_id = omp_get_thread_num();
	memcpy(globules[t_id]->mac, value, MAC_ADDR_LEN);
}
unsigned char *get_mac()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->mac;
}

void set_ssid(char *value)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->ssid)
	{
	int t_id = omp_get_thread_num();
		free(globules[t_id]->ssid);
		globules[t_id]->ssid = NULL;
	}

	if(value)
	{
	int t_id = omp_get_thread_num();
		if(strlen(value) > 0)
		{
	int t_id = omp_get_thread_num();
			globules[t_id]->ssid = strdup(value);
		}
	}
}
char *get_ssid()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->ssid;
}

void set_iface(char *value)
{
	int t_id = omp_get_thread_num();
	if(value)
	{
	int t_id = omp_get_thread_num();
		if(globules[t_id]->iface)
		{
	int t_id = omp_get_thread_num();
			free(globules[t_id]->iface);
		}

		globules[t_id]->iface = strdup(value);
	}
	else if(globules[t_id]->iface)
	{
	int t_id = omp_get_thread_num();
		free(globules[t_id]->iface);
		globules[t_id]->iface = NULL;
	}
}
char *get_iface()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->iface;
}

void set_pin(char *value)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->pin) free(globules[t_id]->pin);
	globules[t_id]->pin = (value) ? strdup(value) : NULL;
}
char *get_pin()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->pin;
}

void set_static_p1(char *value)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->static_p1) free(globules[t_id]->static_p1);
	globules[t_id]->static_p1 = (value) ? strdup(value) : NULL;
}

char *get_static_p1(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->static_p1;
}

void set_static_p2(char *value)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->static_p2) free(globules[t_id]->static_p2);
	globules[t_id]->static_p2 = (value) ? strdup(value) : NULL;
}

char *get_static_p2(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->static_p2;
}

void set_pin_string_mode(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->use_pin_string = value;
}

int get_pin_string_mode(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->use_pin_string;
}

void set_win7_compat(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->win7_compat = value;
}

int get_win7_compat(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->win7_compat;
}

void set_dh_small(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->dh_small = value;
}
int get_dh_small(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->dh_small;
}

void set_external_association(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->external_association = value;
}
int get_external_association(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->external_association;
}

void set_nack_reason(enum nack_code value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->nack_reason = value;
}
enum nack_code get_nack_reason()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->nack_reason;
}

void set_handle(pcap_t *value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->handle = value;
}
pcap_t *get_handle()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->handle;
}

void set_wps(struct wps_data *value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->wps = value;
}
struct wps_data *get_wps()
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->wps;
}

void set_ap_htcaps(unsigned char *value, int len)
{
	int t_id = omp_get_thread_num();
	free(globules[t_id]->htcaps);
	globules[t_id]->htcaps = malloc(len);
	globules[t_id]->htcaps_len = len;
	memcpy(globules[t_id]->htcaps, value, len);
}

unsigned char *get_ap_htcaps(int *len)
{
	int t_id = omp_get_thread_num();
	*len = globules[t_id]->htcaps_len;
	return globules[t_id]->htcaps;
}

void set_ap_rates(unsigned char *value, int len)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->ap_rates)
	{
	int t_id = omp_get_thread_num();
		free(globules[t_id]->ap_rates);
		globules[t_id]->ap_rates_len = 0;
	}

	globules[t_id]->ap_rates = malloc(len);
	if(globules[t_id]->ap_rates)
	{
	int t_id = omp_get_thread_num();
		memcpy(globules[t_id]->ap_rates, value, len);
		globules[t_id]->ap_rates_len = len;
	}
}

unsigned char *get_ap_rates(int *len)
{
	int t_id = omp_get_thread_num();
	*len = globules[t_id]->ap_rates_len;
	return globules[t_id]->ap_rates;
}

void set_ap_ext_rates(unsigned char *value, int len)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->ap_ext_rates)
	{
	int t_id = omp_get_thread_num();
		free(globules[t_id]->ap_ext_rates);
		globules[t_id]->ap_ext_rates_len = 0;
	}

	globules[t_id]->ap_ext_rates = malloc(len);
	if(globules[t_id]->ap_ext_rates)
	{
	int t_id = omp_get_thread_num();
		memcpy(globules[t_id]->ap_ext_rates, value, len);
		globules[t_id]->ap_ext_rates_len = len;
	}
}

unsigned char *get_ap_ext_rates(int *len)
{
	int t_id = omp_get_thread_num();
	*len = globules[t_id]->ap_ext_rates_len;
	return globules[t_id]->ap_ext_rates;
}

void set_exec_string(char *string)
{
	int t_id = omp_get_thread_num();
	if(globules[t_id]->exec_string)
	{
	int t_id = omp_get_thread_num();
		free(globules[t_id]->exec_string);
		globules[t_id]->exec_string = NULL;
	}

	if(string)
	{
	int t_id = omp_get_thread_num();
		globules[t_id]->exec_string = strdup(string);
	}
}
char *get_exec_string(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->exec_string;
}

void set_oo_send_nack(int value)
{
	int t_id = omp_get_thread_num();
	globules[t_id]->oo_send_nack = value;
}
int get_oo_send_nack(void)
{
	int t_id = omp_get_thread_num();
	return globules[t_id]->oo_send_nack;
}

void set_vendor(int is_set, const unsigned char* v) {
	int t_id = omp_get_thread_num();
	globules[t_id]->vendor_oui[0] = is_set;
	if(is_set) memcpy(globules[t_id]->vendor_oui+1, v, 3);
}
unsigned char *get_vendor(void) {
	int t_id = omp_get_thread_num();
	if(!globules[t_id]->vendor_oui[0]) return 0;
	return globules[t_id]->vendor_oui+1;
}

void set_repeat_m6(int val) {
	int t_id = omp_get_thread_num();
	globules[t_id]->repeat_m6 = val;
}

int get_repeat_m6(void) {
	int t_id = omp_get_thread_num();
	return globules[t_id]->repeat_m6;
}

int get_output_fd(void) {
	int t_id = omp_get_thread_num(); 
    return globules[t_id]->output_fd; 
}

#include "pcapfile.h"
void set_output_fd(int fd) {
	int t_id = omp_get_thread_num();
	globules[t_id]->output_fd = fd;
	if (fd != -1) pcapfile_write_header(fd);
}
