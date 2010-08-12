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
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/reboot.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include <librouter/options.h>

#include "web_config.h"

int handle_reboot(struct request *req, struct response *resp)
{
	pid_t sid;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	web_dbg("rebooting ...\n");

	cgi_response_set_html(resp, "/wn/cgi/templates/do_reboot_message.html");

	switch (fork()) {
	case 0: /* child */
		umask(0);
		sid = setsid();
		if (sid < 0) {
			exit(EXIT_FAILURE);
		}
		chdir("/");

		/* wait 3 seconds and reboot */
		popen("/sbin/reboot -d 3 -f ", "r");

		exit(0); /* Not reached */
		break;
	case -1: /* error */
		break;
	default: /* parent - nothing to be done */
		break;
	}

	return 0;
}
