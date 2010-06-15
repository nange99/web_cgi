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

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>

#include <libconfig/quagga.h>

int handle_static_routes(struct request *req, struct response *resp)
{
	char *route;
	char zebra_cmd[256];
	char buf_daemon[256];

	if (daemon_connect(ZEBRA_PATH) < 0)
		return -1;

	//new_cmdline = cish_to_linux_dev_cmdline((char*) cmdline);
	//new_cmdline = linux_to_zebra_network_cmdline((char*) new_cmdline);


	//sprintf(zebra_cmd, "ip route %s %s %s");

	daemon_client_execute("enable", stdout, buf_daemon, 0);
	daemon_client_execute("configure terminal", stdout, buf_daemon, 0);
	daemon_client_execute(zebra_cmd, stdout, buf_daemon, 0);
	daemon_client_execute("write file", stdout, buf_daemon, 0);

	fd_daemon_close();

	route = cgi_request_get_parameter(req, "route");

	return 0;
}
