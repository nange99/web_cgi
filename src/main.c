/*
 * main.c
 *
 *  Created on: May 26, 2010
 *      Author: tgrande
 */
#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include "web_config.h"
#include "reboot.h"
#include "route.h"
#include "firmware.h"
#include "interface.h"

/**
 * Go to home page
 *
 * @param req
 * @param resp
 * @return
 */
int handle_home(struct request *req, struct response *resp)
{
	if (cgi_session_exists(req))
		cgi_response_set_html(resp, "/wn/cgi/templates/do_home.html");
	else
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");

	return 1;
}

/**
 * Logs a user in
 *
 * @param req
 * @param resp
 * @return
 */
int handle_login(struct request *req, struct response *resp)
{
	char *username;
	char *password;

	/* If session exists go directly to home page */
	if (cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_home.html");
		return 1;
	}

	/* Get User and Pass */
	username = cgi_request_get_parameter(req, "username");
	password = cgi_request_get_parameter(req, "password");

	/* This may be the first access, so the fields might be NULL */
	if ((username == NULL) && (password == NULL)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 1;
	}

	/* Do PAM authentication */
	if (pam_web_authenticate(username, password) == AUTH_OK) {
		cgi_session_init(req); /* Initiates a session */
		cgi_response_set_html(resp, "/wn/cgi/templates/do_home.html");
	} else {
		/* No good, try again */
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		/* Show authentication error */
		cgi_response_add_parameter(resp, "err", (void *)1, CGI_INTEGER);
	}

	return 1;
}

/**
 * Logs out a user, deleting existing session
 *
 * @param req
 * @param resp
 * @return
 */
int handle_logout(struct request *req, struct response *resp)
{
	if (cgi_session_exists(req))
		cgi_session_destroy(req);

	cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");

	return 1;
}

/**
 * Save current configuration on flash chip
 *
 * @param req
 * @param resp
 * @return
 */
int handle_saveconf(struct request *req, struct response *resp)
{
	cish_config *cish_cfg;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cish_cfg = lconfig_mmap_cfg();

	if (cish_cfg == NULL) {
		syslog(LOG_ERR, "Failed in mapping cish config\n");
		goto saveconf_finish;
	}

	/* Store configuration in f */
	if (lconfig_write_config(TMP_CFG_FILE, cish_cfg) < 0) {
		syslog(LOG_ERR, "Failed in writing to tmp file\n");
		goto saveconf_finish;
	}

	/* Save in flash */
	if (save_configuration(TMP_CFG_FILE) < 0) {
		syslog(LOG_ERR, "Failed in writing to flash\n");
		goto saveconf_finish;
	}

saveconf_finish:
	/* Remove temp file */
	unlink(TMP_CFG_FILE);

	lconfig_munmap_cfg(cish_cfg);

	cgi_response_set_html(resp, "/wn/cgi/templates/do_show_config_saved.html");

	web_dbg("Configuration saved!\n");

	return 0;
}

/*
 * Temporary Handlers
 *
 */
int handle_config_firewall(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_firewall.html");

	return 1;
}

int handle_config_nat(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 1;
}

int handle_config_qos(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_qos.html");

	return 1;
}

int handle_config_ipsec(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_ipsec.html");

	return 1;
}

int handle_config_snmp(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_snmp.html");

	return 1;
}

int handle_config_auth(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/config_auth.html");

	return 1;
}

int handle_status_interfaces(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_interfaces.html");

	return 1;
}

int handle_status_firewall(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_firewall.html");

	return 1;
}

int handle_status_nat(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_nat.html");

	return 1;
}

int handle_status_qos(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_qos.html");

	return 1;
}

int handle_status_ipsec(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_ipsec.html");

	return 1;
}

int handle_status_snmp(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_snmp.html");

	return 1;
}

int handle_status_auth(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_set_html(resp, "/wn/cgi/templates/status_auth.html");

	return 1;
}



int main(int argc, char **argv)
{
	struct url_mapping map[] = {
		/* Login handler */
		{.url = "/do_login", .handler = handle_login},
		/* Top menu handlers */
		{.url = "/do_logout", .handler = handle_logout},
		{.url = "/do_saveconf", .handler = handle_saveconf},
		{.url = "/do_reboot", .handler = handle_reboot},

		/* Home page */
		{.url = "/do_home", .handler = handle_home},

		/* Configuration pages */
		{.url = "/config_interfaces", .handler = handle_config_interface},
		{.url = "/config_static_routes", .handler = handle_static_routes},
		{.url = "/add_route", .handler = handle_add_route},

		{ .url = "/config_firewall", .handler = handle_config_firewall },
		{ .url = "/config_nat", .handler = handle_config_nat },
		{ .url = "/config_qos", .handler = handle_config_qos },
		{ .url = "/config_ipsec", .handler = handle_config_ipsec },
		{ .url = "/config_snmp", .handler = handle_config_snmp },
		{ .url = "/config_auth", .handler = handle_config_auth },
		{ .url = "/status_interfaces", .handler = handle_status_interfaces },
		{ .url = "/status_firewall", .handler = handle_status_firewall },
		{ .url = "/status_nat", .handler = handle_status_nat },
		{ .url = "/status_qos", .handler = handle_status_qos },
		{ .url = "/status_ipsec", .handler = handle_status_ipsec },
		{ .url = "/status_snmp", .handler = handle_status_snmp },
		{ .url = "/status_auth", .handler = handle_status_auth },


		/* Firmware */
		{.url ="/firmware_info", .handler = handle_firmware_version},
		{.url ="/firmware_upgrade", .handler = handle_firmware_upgrade},
		{.url ="/firmware_upgrade_frame", .handler = handle_firmware_upgrade_frame },
		{.url ="/firmware_upgrade_save", .handler = handle_firmware_receive_file}
	};

	struct config conf = {
		NULL
		/*default_error_html = "error.html";*/
	};

	int size = sizeof(map) / sizeof(struct url_mapping);

	web_dbg("web_digistar is running as %d UID. Config size is %d\n", geteuid(), size);

	cgi_servlet_init (&conf, (struct url_mapping *)&map, size, NULL);

	web_dbg("web_digistar done!\n");

	return 0;
}
