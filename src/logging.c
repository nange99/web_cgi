/*
 * logging.c
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
#include <sys/types.h>
#include <sys/stat.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include <librouter/options.h>

#include "web_config.h"

int handle_status_logging(struct request *req, struct response *resp)
{
	FILE *f;
	struct stat st;
	char *buf;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	f = fopen("/var/log/messages", "r");

	if (f == NULL)
		return 0; /* Log file does not exist */

	stat("/var/log/messages", &st);
	buf = malloc(st.st_size);

	web_dbg("File messages has size is %d\n", st.st_size);

	fread(buf, st.st_size, 1, f);
	fclose(f);

	cgi_response_add_parameter(resp, "log", (char *) buf, CGI_STRING);
	cgi_response_add_parameter(resp, "menu_status", (void *) 8, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_logging.html");

	free(buf);
	return 0;
}
