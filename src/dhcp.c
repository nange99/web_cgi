/*
 * dhcp.c
 *
 *  Created on: Jul 8, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>

#include <librouter/options.h>
#include <librouter/dhcp.h>

#include "web_config.h"

int handle_apply_dhcpd_settings(struct request *req, struct response *resp)
{
	struct dhcp_server_conf_t conf;
	int ret;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	memset(&conf, 0, sizeof(struct dhcp_server_conf_t));

	if (_get_parameter(req, "enable_server") != NULL)
		conf.enable = 1;

	conf.pool_start = _get_parameter(req, "poolstart");
	conf.pool_end = _get_parameter(req, "poolend");
	conf.default_router = _get_parameter(req, "router");
	conf.dnsserver = _get_parameter(req, "dnsserver");
	conf.domain = _get_parameter(req, "domain");

	if (_get_parameter(req, "leasetime") != NULL)
		conf.default_lease_time = atoi(_get_parameter(req, "leasetime"));

	if (_get_parameter(req, "maxleasetime") != NULL)
		conf.max_lease_time = atoi(_get_parameter(req, "maxleasetime"));

	ret = librouter_dhcp_server_set_config(&conf);

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return ret;
}

int handle_config_dhcpd(struct request *req, struct response *resp)
{
	struct dhcp_server_conf_t conf;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	if (librouter_dhcp_server_get_config(&conf) < 0) {
		log_err("Could not get DHCP server configuration\n");
		return -1;
	}

	cgi_response_add_parameter(resp, "enable_server", (void *) conf.enable, CGI_INTEGER);

	if (conf.pool_start != NULL)
		cgi_response_add_parameter(resp, "poolstart", (char *) conf.pool_start, CGI_STRING);

	if (conf.pool_end != NULL)
		cgi_response_add_parameter(resp, "poolend", (char *) conf.pool_end, CGI_STRING);

	if (conf.default_router != NULL)
		cgi_response_add_parameter(resp, "router", (char *) conf.default_router, CGI_STRING);

	if (conf.dnsserver != NULL)
		cgi_response_add_parameter(resp, "dnsserver", (char *) conf.dnsserver, CGI_STRING);

	if (conf.domain != NULL)
		cgi_response_add_parameter(resp, "domain", (char *) conf.domain, CGI_STRING);

	if (conf.default_lease_time != 0)
		cgi_response_add_parameter(resp, "default_lease_time", (void *) conf.default_lease_time, CGI_INTEGER);

	if (conf.max_lease_time != 0)
		cgi_response_add_parameter(resp, "max_lease_time", (void *) conf.max_lease_time, CGI_INTEGER);

	cgi_response_add_parameter(resp, "menu_config", (void *) 10, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_dhcpd.html");

	librouter_dhcp_server_free_config(&conf);

	return 1;
}
