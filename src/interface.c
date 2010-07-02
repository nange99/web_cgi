/*
 * interface.c
 *
 *  Created on: May 28, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include "web_config.h"
#include <net/if_arp.h>
#include "interface.h"

static int _ppp_to_3g(char *orig, char *dest)
{
	sprintf(dest, "m3G%c", orig[3]);
	return 0;
}

static int _3g_to_ppp(char *orig, char *dest)
{
	sprintf(dest, "ppp%c", orig[3]);
	return 0;
}

#define CGI_DHCP_VAR(x,y)	sprintf(x, "%s-dhcpc", y)

int handle_apply_intf_settings(struct request *req, struct response *resp)
{
	char *interface;
	int ret = 0;


	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	interface = cgi_request_get_parameter(req, "name");
	if (interface == NULL) {
		log_err("NULL interface received by handler\n");
		return -1;
	}

	/* Test for interface type */
	if (strstr(interface, "eth")) {
		char *dhcpc, *ipaddr, *ipmask, *speed, *up;
		char daemon_dhcpc[32];

		sprintf (daemon_dhcpc, DHCPC_DAEMON, interface);

		/* Handle Ethernet */
		dhcpc = cgi_request_get_parameter(req, "dhcpc");
		ipaddr = cgi_request_get_parameter(req, "ipaddr");
		ipmask = cgi_request_get_parameter(req, "ipmask");
		speed = cgi_request_get_parameter(req, "speed");
		up = cgi_request_get_parameter(req, "up");

		if (dhcpc)
			libconfig_exec_daemon (daemon_dhcpc);
		else {
			/* Kill DHCP client daemon if it is running */
			if (libconfig_exec_check_daemon(daemon_dhcpc))
				libconfig_kill_daemon(daemon_dhcpc);
		}

		ret = -1;

	} else if (strstr(interface, "lo")) {
		web_dbg();
		/* Handle Loopback */
	} else if (strstr(interface, "3G")) {
		/* Handle 3G */
		web_dbg();
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return 0;
}

#define ETHERNET_IFACE_NUM	2
#define LOOPBACK_IFACE_NUM	1
#define M3G_IFACE_NUM		3

static cgi_table * _fetch_eth_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;

	/* Create ethernet table */
	t = cgi_table_create(6, "name", "speed", "ipaddr", "ipmask", "dhcpc", "up");

	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		sprintf(iface, "ethernet%d", i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "link", (void *) conf.running, CGI_INTEGER);
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "dhcpc", (void *) conf.dhcpc, CGI_INTEGER);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

static cgi_table * _fetch_lo_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;

	/* Create loopback table */
	t = cgi_table_create(4, "name", "ipaddr", "ipmask", "up");

	for (i = 0; i < LOOPBACK_IFACE_NUM; i++) {
		sprintf(iface, "loopback%d", i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

static cgi_table * _fetch_3g_info(void)
{
	cgi_table *t;
	char iface[8], webname[8];
	int i;
	struct interface_conf conf;
	ppp_config ppp_cfg;

	/* Create loopback table */
	t = cgi_table_create(5, "name", "apn", "user", "pass", "up");

	for (i = 0; i < M3G_IFACE_NUM; i++) {
		sprintf(iface, "ppp%d", i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		if (libconfig_ppp_get_config(i, &ppp_cfg) < 0)
			return NULL;

		cgi_table_add_row(t);

		_ppp_to_3g(iface, webname);
		cgi_table_add_value(t, "name", (char *) webname, CGI_STRING);
		cgi_table_add_value(t, "apn", (char *) ppp_cfg.apn, CGI_STRING);
		cgi_table_add_value(t, "user", (char *) ppp_cfg.auth_user, CGI_STRING);
		cgi_table_add_value(t, "pass", (char *) ppp_cfg.auth_pass, CGI_STRING);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

int handle_config_interface(struct request *req, struct response *resp)
{
	cgi_table *eth_table, *lo_table, *m3g_table;


	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	web_dbg();

	eth_table = cgi_table_create(6, "name", "speed", "ipaddr", "ipmask", "dhcpc", "up");
	lo_table = cgi_table_create(4, "name", "ipaddr", "ipmask", "up");
	m3g_table = cgi_table_create(5, "name", "apn", "username", "password", "up");

	web_dbg();

	eth_table = _fetch_eth_info();
	if (eth_table == NULL) {
		log_err("Failed to fetch ethernet information\n");
		return -1;
	}

	lo_table = _fetch_lo_info();
	if (lo_table == NULL) {
		log_err("Failed to fetch loopback information\n");
		return -1;
	}

	m3g_table = _fetch_3g_info();
	if (m3g_table == NULL) {
		log_err("Failed to fetch 3G information\n");
		return -1;
	}

	web_dbg("Filling tables...\n");
	cgi_response_add_parameter(resp, "menu_config", (void *) 1, CGI_INTEGER);
	cgi_response_add_parameter(resp, "eth_table", (void *) eth_table, CGI_TABLE);
	cgi_response_add_parameter(resp, "lo_table", (void *) lo_table, CGI_TABLE);
	cgi_response_add_parameter(resp, "m3g_table", (void *) m3g_table, CGI_TABLE);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_interfaces.html");

	return 0;
}

