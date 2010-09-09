/*
 * main.c
 *
 *  Created on: May 26, 2010
 *      Author: tgrande
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include <librouter/pam.h>
#include <librouter/config_mapper.h>
#include <librouter/config_fetcher.h>
#include <librouter/defines.h>
#include <librouter/nv.h>

#include "interface_status.h"
#include "web_config.h"
#include "reboot.h"
#include "route.h"
#include "firmware.h"
#include "interface.h"
#include "dhcp.h"
#include "logging.h"
#include "cpu.h"
#include "ntp.h"
#include "auth.h"
#include "firewall.h"
#include "nat.h"

/**
 * _get_parameter	Wrapper for cgi_request_get_parameter
 *
 * @param req
 * @param param
 * @return cgi string pointer
 */
char *_get_parameter(struct request *req, char *param)
{
	char *value;
	value = cgi_request_get_parameter(req, param);

	if (value == NULL)
		return NULL;

	if (strlen(value) == 0)
		return NULL;

	if (strlen(value) > CGI_MAX_PARAM_LEN)
		return NULL;

	return value;
}

/*****************/
/* Main handlers */
/*****************/

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
	username = _get_parameter(req, "username");
	password = _get_parameter(req, "password");

	/* This may be the first access, so the fields might be NULL */
	if ((username == NULL) && (password == NULL)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 1;
	}

	/* Do not accept an empty user */
	if (username[0] == '\0') {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		cgi_response_add_parameter(resp, "empty", (void *)1, CGI_INTEGER);
		return 1;
	}

	/* Do PAM authentication */
	if (librouter_pam_web_authenticate(username, password) == AUTH_OK) {
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
	struct router_config *router_cfg;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	router_cfg = librouter_config_mmap_cfg();

	if (router_cfg == NULL) {
		syslog(LOG_ERR, "Failed in mapping cish config\n");
		goto saveconf_finish;
	}

	/* Store configuration in f */
	if (librouter_config_write(TMP_CFG_FILE, router_cfg) < 0) {
		syslog(LOG_ERR, "Failed in writing to tmp file\n");
		goto saveconf_finish;
	}

	/* Save in flash */
	if (librouter_nv_save_configuration(TMP_CFG_FILE) < 0) {
		syslog(LOG_ERR, "Failed in writing to flash\n");
		goto saveconf_finish;
	}

saveconf_finish:
	/* Remove temp file */
	unlink(TMP_CFG_FILE);

	librouter_config_munmap_cfg(router_cfg);

	cgi_response_set_html(resp, "/wn/cgi/templates/do_show_config_saved.html");

	web_dbg("Configuration saved!\n");

	return 0;
}

/*
 * Temporary Handlers
 *
 */
int handle_status_firewall(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 2, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_firewall.html");

	return 1;
}

int handle_status_nat(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 3, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_nat.html");

	return 1;
}

int handle_status_qos(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_qos.html");

	return 1;
}

int handle_status_ipsec(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 5, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_ipsec.html");

	return 1;
}

int handle_status_snmp(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 6, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_snmp.html");

	return 1;
}

int handle_status_auth(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_status", (void *) 7, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_auth.html");

	return 1;
}

int handle_config_qos(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 5, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_qos.html");

	return 1;
}

int handle_config_ipsec(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 6, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_ipsec.html");

	return 1;
}

int handle_config_snmp(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 7, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_snmp.html");

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
		{.url = "/apply_intf_settings", .handler = handle_apply_intf_settings},

		/* Routes */
		{.url = "/config_static_routes", .handler = handle_static_routes},
		{.url = "/add_route", .handler = handle_add_route},

		/* Firewall */
		{ .url = "/config_firewall", .handler = handle_config_firewall },
		{ .url = "/apply_fw_general_settings", .handler = handle_apply_fw_general_settings },
		{ .url = "/add_fw_rule", .handler = handle_fw_add_rule },
		{ .url = "/del_fw_rule", .handler = handle_fw_del_rule },
		{ .url = "/view_fw_rule", .handler = handle_fw_view_rule },

		/* NAT */
		{ .url = "/config_nat", .handler = handle_config_nat },
		{ .url = "/apply_nat_general_settings", .handler = handle_apply_nat_general_settings },
		{ .url = "/app/apply_nat_rules_settings", .handler = handle_apply_nat_rules_settings },
		{ .url = "/add_nat_rule", .handler = handle_nat_add_rule },
		{ .url = "/del_nat_rule", .handler = handle_nat_del_rule },
		{ .url = "/view_nat_rule", .handler = handle_nat_view_rule },

		/* QoS */
		{ .url = "/config_qos", .handler = handle_config_qos },

		/* IPSec */
		{ .url = "/config_ipsec", .handler = handle_config_ipsec },

		/* SNMP */
		{ .url = "/config_snmp", .handler = handle_config_snmp },

		/* Authentication */
		{ .url = "/config_auth", .handler = handle_config_auth },
		{ .url = "/add_user", .handler = handle_add_user },
		{ .url = "/apply_auth_type", .handler = handle_apply_auth_type },
		{ .url = "/apply_radius_settings", .handler = handle_apply_radius_settings },
		{ .url = "/apply_tacacs_settings", .handler = handle_apply_tacacs_settings },

		/* NTP */
		{ .url = "/config_ntp", .handler = handle_config_ntp },
		{ .url = "/apply_date_settings", .handler = handle_apply_date_settings },
		{ .url = "/apply_timezone_settings", .handler = handle_apply_timezone_settings },

		/* DHCP Server */
		{ .url = "/config_dhcpd", .handler = handle_config_dhcpd },
		{ .url = "/apply_dhcp_settings", .handler = handle_apply_dhcpd_settings },

		/* Status Pages */
		{ .url = "/status_interfaces", .handler = handle_status_interfaces },

#ifdef OTHER_STATUS
		/* Funções desativadas no presente momento */
		{ .url = "/status_firewall", .handler = handle_status_firewall },
		{ .url = "/status_nat", .handler = handle_status_nat },
		{ .url = "/status_qos", .handler = handle_status_qos },
		{ .url = "/status_ipsec", .handler = handle_status_ipsec },
		{ .url = "/status_snmp", .handler = handle_status_snmp },
		{ .url = "/status_auth", .handler = handle_status_auth },
#endif
		{ .url = "/status_logging", .handler = handle_status_logging },
		{ .url = "/status_cpu", .handler = handle_status_cpu },
		{ .url = "/status_memory", .handler = handle_status_memory },

		/* Firmware */
		{.url ="/firmware_info", .handler = handle_firmware_version},
		{.url ="/firmware_upgrade", .handler = handle_firmware_upgrade},
		{.url ="/firmware_upgrade_frame", .handler = handle_firmware_upgrade_frame },
		{.url ="/firmware_upgrade_save", .handler = handle_firmware_receive_file}
	};

	struct url_mapping **map_p = (struct url_mapping **)((void *) &map);

	struct config conf = {
		NULL
		/*default_error_html = "error.html";*/
	};

	int size = sizeof(map) / sizeof(struct url_mapping);

	/* Run with effective UID */
	if (setuid(geteuid()) < 0) {
		log_err("Could not set UID with effective : %s\n", strerror(errno));
		return -1;
	}

	web_dbg("web_digistar is running as %d UID. Config size is %d\n", getuid(), size);

	cgi_servlet_init (&conf, map_p, size, NULL);

	web_dbg("web_digistar done!\n");

	return 0;
}
