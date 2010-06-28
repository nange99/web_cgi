/*
 * route.c
 *
 *  Created on: Jun 14, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
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
	route->network = cgi_request_get_parameter(req, "dest");
	route->mask = cgi_request_get_parameter(req, "mask");
	route->gateway = cgi_request_get_parameter(req, "gateway");
	route->interface = cgi_request_get_parameter(req, "interface");
	route->metric = atoi(cgi_request_get_parameter(req, "metric"));

	web_dbg("network = %s\n", route->network);
	web_dbg("mask = %s\n", route->mask);
	web_dbg("gateway = %s\n", route->gateway);


	libconfig_quagga_add_route(route);

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

	del = cgi_request_get_parameter(req, "del");

	/* Check if we are deleting one entry */
	if (del)
		libconfig_quagga_del_route(del);

	/* Check if returning just table (AJAX) */
	table = cgi_request_get_parameter(req, "table");

	t = cgi_table_create(6, "dest", "mask", "gateway", "interface", "metric", "hash");

	next = route = libconfig_quagga_get_routes();

	while (next) {
		web_dbg("dest = %s\n", next->network);
		web_dbg("mask = %s\n", next->mask);
		cgi_table_add_row(t);
		web_dbg();
		cgi_table_add_value(t, "dest", (char *) next->network, CGI_STRING);
		web_dbg();
		cgi_table_add_value(t, "mask", (char *) next->mask, CGI_STRING);
		web_dbg();
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
	if (table || del)
		cgi_response_set_html(resp, "/wn/cgi/templates/list_route_table.html");
	else
		cgi_response_set_html(resp, "/wn/cgi/templates/list_route.html");

	libconfig_quagga_free_routes(route);

	return 0;
}
