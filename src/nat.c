/*
 * nat.c
 *
 *  Created on: Jul 19, 2010
 *      Author: Igor Kramer Pinotti (ipinotti@pd3.com.br)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

int handle_apply_nat_general_settings(struct request *req, struct response *resp)
{
	int ret = 0;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return 0;
}

int handle_apply_nat_rules_settings(struct request *req, struct response *resp)
{
	int ret = 0;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return 0;
}

int handle_config_nat(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 0;
}

int handle_nat_add_rule(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 0;
}

int handle_nat_del_rule(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 0;
}

int handle_nat_view_rule(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 0;
}
