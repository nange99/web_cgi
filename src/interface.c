/*
 * interface.c
 *
 *  Created on: May 28, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include "web_config.h"
#include "interface.h"

#include <net/if_arp.h>

/**
 * _convert_string_web_to_linux		Convert web to linux string
 *
 * @param type
 * @param web_interface
 * @return Pointer to linux string, needs to be free'd after use
 */
static char *_convert_string_web_to_linux(device_type type, char *web_interface)
{
	char *linux_interface;
	dev_family *fam = libconfig_device_get_family_by_type(type);

	if (fam == NULL)
		return NULL;

	linux_interface = malloc(32);

	/* Position the string at the index */
	web_interface += strlen(fam->web_string);
	sprintf(linux_interface, "%s%s", fam->linux_string, web_interface);

	return linux_interface;
}

/**
 * _apply_eth_settings	Apply settings to an ethernet interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_eth_settings(struct request *req, struct response *resp)
{
	char *interface, *dhcpc, *ipaddr, *ipmask, *speed, *up;
	char daemon_dhcpc[32];
	char *linux_interface;

	/* Handle Ethernet */
	interface = cgi_request_get_parameter(req, "name");
	dhcpc = cgi_request_get_parameter(req, "dhcpc");
	ipaddr = cgi_request_get_parameter(req, "ipaddr");
	ipmask = cgi_request_get_parameter(req, "ipmask");
	speed = cgi_request_get_parameter(req, "speed");
	up = cgi_request_get_parameter(req, "up");

	web_dbg("interface is %s\n", interface);
	linux_interface = _convert_string_web_to_linux(eth, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}

	sprintf(daemon_dhcpc, DHCPC_DAEMON, linux_interface);

	if (dhcpc)
		libconfig_exec_daemon (daemon_dhcpc);
	else {
		/* Kill DHCP client daemon if it is running */
		if (libconfig_exec_check_daemon(daemon_dhcpc))
			libconfig_kill_daemon(daemon_dhcpc);

		libconfig_ip_ethernet_set_addr(linux_interface, ipaddr, ipmask);
	}

	if (up)
		libconfig_dev_noshutdown(linux_interface);
	else
		libconfig_dev_shutdown(linux_interface);

	free(linux_interface);
	return 0;
}

/**
 * _apply_lo_settings	Apply settings to a loopback interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_lo_settings(struct request *req, struct response *resp)
{
	char *interface, *ipaddr, *ipmask, *up;
	char *linux_interface;

	interface = cgi_request_get_parameter(req, "name");
	ipaddr = cgi_request_get_parameter(req, "ipaddr");
	ipmask = cgi_request_get_parameter(req, "ipmask");
	up = cgi_request_get_parameter(req, "up");

	web_dbg("interface is %s\n", interface);
	linux_interface = _convert_string_web_to_linux(eth, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}

	libconfig_ip_interface_set_addr(linux_interface, ipaddr, ipmask);

	if (up)
		libconfig_dev_noshutdown(linux_interface);
	else
		libconfig_dev_shutdown(linux_interface);

	free(linux_interface);
	return 0;
}

/**
 * _apply_3g_settings	Apply settings to a 3G interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_3g_settings(struct request *req, struct response *resp)
{
	char *interface, *apn, *user, *pass, *sim, *up;
	char *linux_interface;
	int major;

	interface = cgi_request_get_parameter(req, "name");
	apn = cgi_request_get_parameter(req, "apn");
	user = cgi_request_get_parameter(req, "user");
	pass = cgi_request_get_parameter(req, "pass");
	sim = cgi_request_get_parameter(req, "sim");
	up = cgi_request_get_parameter(req, "up");

	web_dbg("interface is %s\n", interface);
	linux_interface = _convert_string_web_to_linux(eth, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}

	major = libconfig_device_get_major(linux_interface, str_linux);

	web_dbg("Major is %d\n", major);

	libconfig_modem3g_set_apn(apn, major);
	libconfig_modem3g_set_username(user, major);
	libconfig_modem3g_set_password(pass, major);

	libconfig_ppp_reload_backupd(); /* Tell backupd that configuration has changed */

	free(linux_interface);
	return 0;
}

/**
 * handle_apply_intf_settings	Apply settings to a network interface
 *
 * This function will be called via CGI by the web browser, to apply
 * the form with interface information. Depending on the interface's type,
 * the appropriate handler will be called.
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
int handle_apply_intf_settings(struct request *req, struct response *resp)
{
	char *interface = NULL;
	int ret = -1;
	dev_family *fam;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	interface = cgi_request_get_parameter(req, "name");
	if (interface == NULL) {
		log_err("NULL interface received by handler\n");
		return ret;
	}

	fam = libconfig_device_get_family_by_name(interface, str_web);

	/* Test for interface type */
	switch (fam->type) {
	case eth:
		ret = _apply_eth_settings(req, resp);
		break;
	case lo:
		ret = _apply_lo_settings(req, resp);
		break;
	case ppp:
		ret = _apply_3g_settings(req, resp);
		break;
	default:
		break;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return ret;
}

/**
 * _fetch_eth_info	Fetch ethernet information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_eth_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;
	dev_family *fam;

	fam = libconfig_device_get_family_by_type(eth);

	if (fam == NULL)
		return NULL;

	/* Create ethernet table */
	t = cgi_table_create(6, "name", "speed", "ipaddr", "ipmask", "dhcpc", "up");

	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		sprintf(iface, "%s%d", fam->linux_string, i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		sprintf(iface, "%s%d", fam->web_string, i);

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

/**
 * _fetch_lo_info	Fetch loopback information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_lo_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;
	dev_family *fam;

	fam = libconfig_device_get_family_by_type(lo);

	if (fam == NULL)
		return NULL;

	/* Create loopback table */
	t = cgi_table_create(4, "name", "ipaddr", "ipmask", "up");

	for (i = 0; i < LOOPBACK_IFACE_NUM; i++) {
		sprintf(iface, "%s%d", fam->linux_string, i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		sprintf(iface, "%s%d", fam->web_string, i);

		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

/**
 * _fetch_3g_info	Fetch 3G information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_3g_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;
	ppp_config ppp_cfg;
	dev_family *fam;

	fam = libconfig_device_get_family_by_type(ppp);

	if (fam == NULL)
		return NULL;

	/* Create loopback table */
	t = cgi_table_create(5, "name", "apn", "user", "pass", "up");

	for (i = 0; i < M3G_IFACE_NUM; i++) {
		sprintf(iface, "%s%d", fam->linux_string, i);
		if (libconfig_ip_iface_get_config(iface, &conf) < 0)
			return NULL;

		if (libconfig_ppp_get_config(i, &ppp_cfg) < 0)
			return NULL;

		sprintf(iface, "%s%d", fam->web_string, i);
		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "apn", (char *) ppp_cfg.apn, CGI_STRING);
		cgi_table_add_value(t, "user", (char *) ppp_cfg.auth_user, CGI_STRING);
		cgi_table_add_value(t, "pass", (char *) ppp_cfg.auth_pass, CGI_STRING);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

/**
 * handle_config_interface	Main handler for interface configuration
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
int handle_config_interface(struct request *req, struct response *resp)
{
	cgi_table *eth_table, *lo_table, *m3g_table;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	eth_table = cgi_table_create(6, "name", "speed", "ipaddr", "ipmask", "dhcpc", "up");
	lo_table = cgi_table_create(4, "name", "ipaddr", "ipmask", "up");
	m3g_table = cgi_table_create(5, "name", "apn", "username", "password", "up");


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

