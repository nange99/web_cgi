/*
 * interface_stats.c
 *
 *  Created on: May 28, 2010
 *      Author: tgrande
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <linux/autoconf.h>

#include <libcgiservlet/cgi_servlet.h>

#include <libconfig/cish_defines.h>
#include <libconfig/pam.h>
#include <libconfig/ip.h>

int show_ethernet_stats (struct response *resp)
{
	struct interface_conf conf;
	char buf[64];

	if (libconfig_ip_iface_get_config("ethernet0", &conf))
		return -1;

	sprintf(buf, "Link is %s", conf.running ? "UP" : "DOWN");
	cgi_response_add_parameter (resp, "eth0link", buf, CGI_STRING);

	sprintf(buf, "IP address is %s", conf.main_ip.ipaddr);
	cgi_response_add_parameter (resp, "eth0ip", buf, CGI_STRING);

	return 0;
}



