/*
 * route.c
 *
 *  Created on: Jun 14, 2010
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
#include <librouter/quagga.h>

#include "web_config.h"
#include "route.h"

int handle_add_route(struct request *req, struct response *resp)
{
	struct routes_t *route;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	route = malloc(sizeof(struct routes_t));
	memset(route, 0, sizeof(struct routes_t));

	/* Get form info */
	route->network = _get_parameter(req, "dest");
	route->mask = _get_parameter(req, "mask");
	route->gateway = _get_parameter(req, "gateway");
	route->interface = _get_parameter(req, "interface");
	if (_get_parameter(req, "metric") != NULL)
		route->metric = atoi(_get_parameter(req, "metric"));

	web_dbg("network = %s\n", route->network);
	web_dbg("mask = %s\n", route->mask);
	web_dbg("gateway = %s\n", route->gateway);


	librouter_quagga_add_route(route);

	free(route); /* libcgi frees the parameters */

	return 0;
}

int handle_static_routes(struct request *req, struct response *resp)
{
	struct routes_t *route, *next;
	cgi_table *t;
	char *table, *del;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	del = _get_parameter(req, "del");

	/* Check if we are deleting one entry */
	if (del)
		librouter_quagga_del_route(del);

	/* Check if returning just table (AJAX) */
	table = _get_parameter(req, "table");

	t = cgi_table_create(6, "dest", "mask", "gateway", "interface", "metric", "hash");

	next = route = librouter_quagga_get_routes();

	while (next) {
		web_dbg("dest = %s\n", next->network);
		web_dbg("mask = %s\n", next->mask);

		cgi_table_add_row(t);
		cgi_table_add_value(t, "dest", (char *) next->network, CGI_STRING);
		cgi_table_add_value(t, "mask", (char *) next->mask, CGI_STRING);

		if (next->gateway)
			cgi_table_add_value(t, "gateway", (char *) next->gateway, CGI_STRING);
		else
			cgi_table_add_value(t, "gateway", (char *) "-", CGI_STRING);

		if (next->interface)
			cgi_table_add_value(t, "interface", (char *) next->interface, CGI_STRING);
		else
			cgi_table_add_value(t, "interface", (char *) "-", CGI_STRING);

		if (next->metric)
			cgi_table_add_value(t, "metric", (void *) next->metric, CGI_INTEGER);
		else
			cgi_table_add_value(t, "metric", (void *) 1, CGI_INTEGER);

		cgi_table_add_value(t, "hash", (char *) next->hash, CGI_STRING);

		next = next->next;
	}

	cgi_response_add_parameter(resp, "table", (void *) t, CGI_TABLE);
	if (table || del) {
		cgi_response_set_html(resp, "/wn/cgi/templates/list_route_table.html");
	} else {
		cgi_response_add_parameter(resp, "menu_config", (void *) 2, CGI_INTEGER);
		cgi_response_set_html(resp, "/wn/cgi/templates/list_route.html");
	}

	librouter_quagga_free_routes(route);

	return 0;
}
