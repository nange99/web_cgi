/*
 * firmware.c
 *
 *  Created on: Jun 17, 2010
 *      Author: Pedro Kiefer (pkiefer@pd3.com.br)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_upload.h>

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

	web_dbg("Firmware version : %s\n", version);
	web_dbg("Bootloader version : %s\n", boot);

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

int handle_firmware_upgrade_frame(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/firmware_form.html");


	return 0;
}

int handle_firmware_receive_file(struct request *req, struct response *resp)
{
	cgi_file *f;
	int rtn;
	pid_t sid;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	if (!cgi_request_is_post(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/firmware_upgrade.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/firmware_save.html");

	f = cgi_upload_get_file(req, "firmware");

	if (f == NULL) {
		cgi_response_add_parameter(resp, "err", (void *) 1, CGI_INTEGER);
		return 0;
	} else if (f->error > 0) {
		cgi_response_add_parameter(resp, "err", (void *) 2, CGI_INTEGER);
		cgi_response_add_parameter(resp, "err_msg", cgi_upload_get_error_str(f->error), CGI_STRING);
		return 0;
	}

	rtn = libconfig_flash_check_image(f->tmp_filename);

	web_dbg("libconfig check image status: %d\n", rtn);

	if (rtn > 0) {
		/* image error */
		cgi_response_add_parameter(resp, "err", (void *) 3, CGI_INTEGER);
		return 0;
	}

	rtn = libconfig_flash_write_image(f->tmp_filename);

	if (rtn > 0) {
		/* write error */
		cgi_response_add_parameter(resp, "err", (void *) 4, CGI_INTEGER);
	}

	/* display message and reboot! */
	cgi_response_set_html(resp, "/wn/cgi/templates/firmware_save.html");

	web_dbg("rebooting ...\n");

	switch (fork()) {
	case 0: /* child */
		umask(0);
		sid = setsid();
		if (sid < 0) {
			exit(EXIT_FAILURE);
		}
		chdir("/");
		/* wait 5 seconds and reboot */
		popen("/sbin/reboot -d 5 -f", "r");

		exit(0);
		break;
	case -1: /* error */
		break;
	default: /* parent - nothing to be done */
		break;
	}

	return 0;
}
