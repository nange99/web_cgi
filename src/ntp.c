/*
 * ntp.c
 *
 *  Created on: Jul 19, 2010
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
#include <libcgiservlet/cgi_table.h>

#include <librouter/options.h>
#include <librouter/ntp.h>
#include <librouter/args.h>
#include <librouter/libtime.h>

#include "web_config.h"

int handle_apply_date_settings(struct request *req, struct response *resp)
{
	int ret = 0;
	char *enable, *server1, *server2, *server3, *date, *time;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	enable = _get_parameter(req, "ntp_sync");
	server1 = _get_parameter(req, "ntp_server1");
	server2 = _get_parameter(req, "ntp_server2");
	server3 = _get_parameter(req, "ntp_server3");
	date = _get_parameter(req, "date");
	time = _get_parameter(req, "time");

	if (enable != NULL)
		librouter_ntp_set_daemon(1);
	else
		librouter_ntp_set_daemon(0);

	/* Delete all servers */
	librouter_ntp_exclude_server(NULL);

	if (server1 != NULL)
		librouter_ntp_server(server1, NULL);
	if (server2 != NULL)
		librouter_ntp_server(server2, NULL);
	if (server3 != NULL)
		librouter_ntp_server(server3, NULL);

	web_dbg("date is: %s\n", date);
	web_dbg("time is: %s\n", time);

	if (date && time) {
		int day, mon, year;
		int hour, min, sec;

		day = atoi(date);
		mon = atoi(date+3);
		year = atoi(date+6);
		hour = atoi(time);
		min = atoi(time+3);
		sec = atoi(time+6);

		if (librouter_time_set_date(day, mon, year, hour, min, sec) < 0)
			log_err("Could not set date");
	}


	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return ret;
}

int handle_apply_timezone_settings(struct request *req, struct response *resp)
{
	int ret = 0;
	char *name, *timezone;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	name = _get_parameter(req, "tzname");
	timezone = _get_parameter(req, "tzvalue");
	if (timezone)
		timezone+= 3; /* skip gmt */

	web_dbg("name: %s timezone: %s\n", name, timezone);
	if (name && timezone)
		librouter_time_set_timezone(name, atoi(timezone), 0);


	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return ret;
}

static int _get_timezone(struct response *resp)
{
	int hours, mins;
	char name[16];

	if (librouter_time_get_timezone(name, &hours, &mins) < 0)
		return -1;

	cgi_response_add_parameter(resp, "tzname", (char *)name, CGI_STRING);
	cgi_response_add_parameter(resp, "tzvalue", (void *)hours, CGI_INTEGER);

	return 0;
}

static int _get_servers(struct response *resp)
{
	char servers[256];
	char cgi_param[32];
	arglist *args;
	int i;

	memset(servers, 0, sizeof(servers));

	if (librouter_ntp_get_servers(servers) < 0) {
		log_err("Could not get NTP server information\n");
		return -1;
	}

	if (servers[0]) {
		web_dbg(" Configured NTP servers are: %s", servers);
		/* Server are separated by spaces, make individual strings */
		args = librouter_make_args(servers);
		for (i=0; i < args->argc; i++) {
			sprintf(cgi_param, "ntp_server%d", i+1);
			cgi_response_add_parameter(resp, cgi_param, (char *)args->argv[i], CGI_STRING);
		}
		librouter_destroy_args(args);
	}
	return 0;
}

int handle_config_ntp(struct request *req, struct response *resp)
{

	int enable_ntp;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	enable_ntp = librouter_ntp_get_daemon(); /* Check if ntp is enabled */
	cgi_response_add_parameter(resp, "ntp_sync", (void *)enable_ntp, CGI_INTEGER);

	if (_get_servers(resp) < 0)
		log_err("Could not fetch NTP servers information\n");

	if (_get_timezone(resp) < 0)
		log_err("Could not fetch timezone information\n");

	cgi_response_add_parameter(resp, "menu_config", (void *) 9, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_ntp.html");

	return 0;
}
