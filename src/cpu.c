/*
 * cpu.c
 *
 *  Created on: Jul 20, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include <librouter/options.h>

#include "web_config.h"

int handle_status_cpu(struct request *req, struct response *resp)
{

	FILE *f;
	char buf[2048];

	memset(buf, 0, sizeof(buf));

	f = fopen("/proc/cpuinfo", "r");
	if (f == NULL)
		return 0;

	fread(buf, sizeof(buf), 1, f);
	fclose(f);

	cgi_response_add_parameter(resp, "cpuinfo", (char *) buf, CGI_STRING);
	cgi_response_add_parameter(resp, "menu_status", (void *) 9, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_cpu.html");

	return 0;
}

int handle_status_memory(struct request *req, struct response *resp)
{

	FILE *f;
	char buf[2048];

	memset(buf, 0, sizeof(buf));

	f = fopen("/proc/meminfo", "r");
	if (f == NULL)
		return 0;

	fread(buf, sizeof(buf), 1, f);
	fclose(f);

	cgi_response_add_parameter(resp, "meminfo", (char *) buf, CGI_STRING);
	cgi_response_add_parameter(resp, "menu_status", (void *) 10, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_memory.html");

	return 0;
}
