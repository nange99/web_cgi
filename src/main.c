/*
 * main.c
 *
 *  Created on: May 26, 2010
 *      Author: tgrande
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <libcgiservlet/cgi_servlet.h>

int handle_config_interface(struct request *req, struct response *resp)
{
	cgi_response_set_html(resp, "/wn/cgi/templates/config_interfaces.html");

	cgi_response_add_parameter (resp, "teste", "O codigo CGI funciona", CGI_STRING);

	return 1;
}

int handle_config_firewall(struct request *req, struct response *resp)
{
	cgi_response_set_html(resp, "/wn/cgi/templates/config_firewall.html");

	return 1;
}

int handle_config_nat(struct request *req, struct response *resp)
{
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 1;
}

int main(int argc, char **argv)
{
	struct url_mapping map[] = {
		{.url = "/config_interfaces", .handler = handle_config_interface},
		{.url = "/config_nat", .handler = handle_config_nat},
		{.url = "/config_firewall", .handler = handle_config_firewall}
	};

	struct config conf = {
		NULL
		/*default_error_html = "error.html";*/
	};

	int size = sizeof(map) / sizeof(struct url_mapping);

	syslog(LOG_INFO, "web_digistar is running. Config size is %d\n", size);

	cgi_servlet_init (&conf, &map, size, NULL);

	return 0;
}
