/*
 * firmware.c
 *
 *  Created on: Jun 17, 2010
 *      Author: Pedro Kiefer (pkiefer@pd3.com.br)
 */

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>

#include "firmware.h"
#include "web_config.h"

int handle_firmware_version(struct request *req, struct response *resp)
{
	char *version, *boot;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	version = get_system_version();
	boot = get_boot_version();

	cgi_response_add_parameter(resp, "version", version, CGI_STRING);
	cgi_response_add_parameter(resp, "boot", boot, CGI_STRING);

	cgi_response_set_html(resp, "/wn/cgi/templates/firmware_info.html");

	return 0;
}

int handle_firmware_upgrade(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/firmware_upgrade.html");

	return 0;
}
int handle_firmware_receive_file(struct request *req, struct response *resp)
{
	return 0;
}
