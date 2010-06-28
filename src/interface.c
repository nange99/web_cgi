/*
 * interface.c
 *
 *  Created on: May 28, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include "web_config.h"

static int _ppp_to_3g(char *orig, char *dest)
{
	sprintf(dest, "m3G%c", orig[3]);
	return 0;
}

static int show_interface_stats (cgi_table *t, char *name)
{
	struct interface_conf conf;
	char buf[8];

	if (libconfig_ip_iface_get_config(name, &conf))
		return -1;

	cgi_table_add_row(t);

	if (strstr(name,"ppp")) {
		_ppp_to_3g(name, buf);
		cgi_table_add_value(t, "name", (char *) buf, CGI_STRING);
	} else
		cgi_table_add_value(t, "name", (char *) name, CGI_STRING);

	cgi_table_add_value(t, "link", (void *) conf.running, CGI_INTEGER);
	cgi_table_add_value (t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
	cgi_table_add_value (t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
	cgi_table_add_value (t, "dhcpc", (void *) conf.dhcpc, CGI_INTEGER);
	cgi_table_add_value (t, "up", (void *) conf.up, CGI_INTEGER);

	web_dbg("interface %s has ipaddr %s\n", name, conf.main_ip.ipaddr);

	return 0;
}

int handle_apply_intf_settings(struct request *req, struct response *resp)
{
	char *interface;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	interface = cgi_request_get_parameter(req,"name");
	//if (interface == NULL) {
	//	log_err("NULL interface received by handler\n");
	//	return -1;
	//}

	/* Test for interface type */
	if (strstr(interface, "eth")) {
		/* Handle Ethernet */
	} else if (strstr(interface, "lo")) {
		/* Handle Loopback */
	} else if (strstr(interface, "3G")) {
		/* Handle 3G */
	}

	handle_config_interface(req,resp);

	return 0;
}

int handle_config_interface(struct request *req, struct response *resp)
{
	//char intfs[4][32] = { "ethernet0", "ethernet1", "loopback0", NULL };
	char intf_list[32][16] = { "ethernet0", "ethernet1", "loopback0", "ppp0", "ppp1", "ppp2", "\0" };
	//char intf_list[32][16] = { "ethernet0", "\0" };
	cgi_table *t;
	int i;

	 if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	t = cgi_table_create(6, "name", "link", "ipaddr", "ipmask", "dhcpc", "up");

	for (i=0; intf_list[i][0] != '\0'; i++) {
		show_interface_stats(t, intf_list[i]);
	}

	cgi_response_add_parameter(resp, "table", (void *) t, CGI_TABLE);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_interfaces.html");

	return 0;
}


