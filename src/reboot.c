/*
 * reboot.c
 *
 *  Created on: Jun 14, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>

#include <sys/reboot.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>

#include "web_config.h"

int handle_reboot(struct request *req, struct response *resp)
{
	 char *const parms[] = {"/sbin/reboot", "-d", "5", "-f", NULL };

	 if (!cgi_session_exists(req)) {
		 cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		 return 0;
	 }

	web_dbg("rebooting ...\n");

	switch (fork()) {
	case 0: /* child */
		/* Create a new SID for the child process */
		setsid();
		execv("/sbin/reboot", parms);
		exit(0);
	case -1: /* error */
		break;
	default: /* parent - nothing to be done */
		cgi_response_set_html(resp, "/wn/cgi/templates/do_reboot_message.html");
		break;
	}

	return 0; /* Not reached */
}
