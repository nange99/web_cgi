/*
 * interface_status.c
 *
 *  Created on: Sep 8, 2010
 *      Author: Igor Kramer Pinotti (ipinotti@pd3.com.br)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <net/if_arp.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>
#include <libcgiservlet/cgi_upload.h>

#include <librouter/options.h>
#include <librouter/device.h>

#include "web_config.h"
#include "interface.h"

int handle_status_interfaces(struct request *req, struct response *resp)
{
	int ret = -1;

	ret = handle_config_interface_status(req,resp);

	return ret;
}
