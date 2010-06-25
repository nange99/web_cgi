/*
 * interface.c
 *
 *  Created on: May 28, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include "web_config.h"

int show_interface_stats (cgi_table *t, char *name)
{
	struct interface_conf conf;

	if (libconfig_ip_iface_get_config(name, &conf))
		return -1;

	cgi_table_add_row(t);

	cgi_table_add_value(t, "name", (char *) name, CGI_STRING);
	cgi_table_add_value(t, "link", (void *) conf.running, CGI_INTEGER);
	cgi_table_add_value (t, "ipaddr", conf.main_ip.ipaddr, CGI_STRING);
	cgi_table_add_value (t, "ipmask", conf.main_ip.ipmask, CGI_STRING);

	return 0;
}

int handle_config_interface(struct request *req, struct response *resp)
{
	//char intfs[4][32] = { "ethernet0", "ethernet1", "loopback0", NULL };
	char intf_list[32][16] = { "ethernet0", "ethernet1", "loopback0", "ppp0", "ppp1", "ppp2", "\0" };
	cgi_table *t;
	int i;

	 if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	t = cgi_table_create(4, "name", "link", "ipaddr", "ipmask");

	for (i=0; intf_list[i][0] != '\0'; i++) {
		show_interface_stats(t, intf_list[i]);
	}

	cgi_response_add_parameter(resp, "table", (void *) t, CGI_TABLE);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_interfaces.html");

	return 1;
}


