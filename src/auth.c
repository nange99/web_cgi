/*
 * auth.c
 *
 *  Created on: Jul 26, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <pwd.h>

#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>

#include <librouter/options.h>
#include <librouter/defines.h>
#include <librouter/args.h>
#include <librouter/pam.h>
#include <librouter/sha1.h>

#include "web_config.h"

static struct {
	enum aaa_modes mode;
	char *cgi_string;
} aaa_map[] = {
		{AAA_AUTH_NONE, "none"},
		{AAA_AUTH_LOCAL, "local"},
		{AAA_AUTH_RADIUS, "radius"},
		{AAA_AUTH_RADIUS_LOCAL, "radius-local"},
		{AAA_AUTH_TACACS, "tacacs"},
		{AAA_AUTH_TACACS_LOCAL, "tacacs-local"},
		{0, NULL}
};


/*
 * LOCAL FUNCTIONS
 */

/**
 * _get_user_hash	Calculate SHA-1 hash for username
 *
 *
 * @param user
 * @return pointer to hash, can be free'd with free()
 */
static char *_get_user_hash(char *user)
{
	sha_ctx hashctx;
	char sha1[20];
	char *hash;

	/* Unique user identifier */
	sha1_init(&hashctx);
	sha1_update(&hashctx, (unsigned char *) user, strlen(user));
	sha1_final((unsigned char *) sha1, &hashctx);
	hash = strdup(sha1_to_hex((unsigned char *) sha1));

	web_dbg("Hash for %s is %s\n", user, hash);

	return hash;
}



/**
 * _get_auth_type	Get type from string for current authentication
 *
 * @param string
 * @return 0 if success
 */
static int _get_auth_type(char *string)
{
	int i;

	for (i=0; aaa_map[i].mode != 0; i++) {
		if (!strcmp(aaa_map[i].cgi_string, string))
			return aaa_map[i].mode;
	}

	/* Not found ! */
	return -1;
}

/**
 * _get_auth_string	Get string from type for current authentication
 *
 * The HTML expects variables with the prefix "web-" or "cli-",
 * which are already present in 'type'.
 *
 * @param filename
 * @param type
 * @return 0 if success
 */
static int _get_auth_string(char *filename, char *type)
{
	int i;
	enum aaa_modes mode;

	mode = librouter_pam_get_current_mode(filename);

	for (i=0; aaa_map[i].mode != 0; i++) {
		if (aaa_map[i].mode == mode)
			strcat(type, aaa_map[i].cgi_string);
	}

	web_dbg("Auth type in %s is %s\n", filename, type);

	return 0;
}

/**
 * _handle_auth_type	Fill response with authentication types
 *
 * Read authentication types for CLI and WEB
 *
 * @param resp
 * @return 0 if success
 */
static int _handle_auth_type(struct response *resp)
{
	char buf[32];

	memset(buf, 0, sizeof(buf));
	sprintf(buf,"cli-");
	_get_auth_string(FILE_PAM_GENERIC, buf);
	cgi_response_add_parameter(resp, "cli_auth_type", (char *) buf, CGI_STRING);

	memset(buf, 0, sizeof(buf));
	sprintf(buf,"web-");
	_get_auth_string(FILE_PAM_WEB, buf);
	cgi_response_add_parameter(resp, "web_auth_type", (char *) buf, CGI_STRING);

	return 0;
}

/*
 *
 * EXPORTED HANDLERS
 *
 */

/**
 * handle_add_user	Add user to system
 *
 * @param req
 * @param resp
 * @return 0 if success
 */
int handle_add_user(struct request *req, struct response *resp)
{
	char *username, *passwd, *confirm;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	username = _get_parameter(req, "username");
	passwd = _get_parameter(req, "password");
	confirm = _get_parameter(req, "confirmpw");

	web_dbg("username : %s\n", username);
	web_dbg("pw : %s\n", passwd);
	web_dbg("confirm : %s\n", confirm);

	/* Password match ? */
	if (strcmp(passwd, confirm))
		return -1;

	if (librouter_pam_add_user(username, passwd) < 0)
		return -1;

	web_dbg();
	return 0;
}

/**
 * handle_apply_auth_type	Apply Authentication method
 *
 * @param req
 * @param resp
 * @return 0 if success
 */
int handle_apply_auth_type(struct request *req, struct response *resp)
{
	char *cli_auth_type, *web_auth_type;
	enum aaa_modes mode;
	int ret = 0;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cli_auth_type = _get_parameter(req, "cli_auth_type");
	web_auth_type = _get_parameter(req, "web_auth_type");

	mode = _get_auth_type(cli_auth_type);
	if (mode > 0)
		librouter_pam_config_mode(mode, FILE_PAM_GENERIC);

	mode = _get_auth_type(web_auth_type);
	if (mode > 0)
		librouter_pam_config_mode(mode, FILE_PAM_WEB);

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return 0;
}

/**
 * handle_apply_radius_settings		Apply RADIUS config
 *
 * @param req
 * @param resp
 * @return 0 if success
 */
int handle_apply_radius_settings(struct request *req, struct response *resp)
{
	int ret = 0;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return 0;
}

/**
 * handle_apply_tacacs_settings		Apply TACACS+ config
 *
 * @param req
 * @param resp
 * @return 0 if success
 */
int handle_apply_tacacs_settings(struct request *req, struct response *resp)
{
	int ret = 0;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return 0;
}

/**
 * handle_config_auth	Main handler for authentication
 *
 * @param req
 * @param resp
 * @return 0 if success
 */
int handle_config_auth(struct request *req, struct response *resp)
{
	cgi_table *t;
	char users[512];
	arglist *args;
	char *table, *del;
	int i;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	memset(users, 0, sizeof(users));

	/* Check if deleting entry */
	del = _get_parameter(req, "del");
	if (del != NULL)
		librouter_pam_del_user(del);

	/* Check if returning just table (AJAX) */
	table = _get_parameter(req, "table");

	/* Get users in system */
	librouter_pam_get_users(users);

	if (users[0]) {
		web_dbg("Users are : %s\n", users);
		args = librouter_make_args(users);
		t = cgi_table_create(1, "username");

		for (i=0; i < args->argc; i++) {
			cgi_table_add_row(t);
			cgi_table_add_value(t, "username", args->argv[i], CGI_STRING);
		}
		cgi_response_add_parameter(resp, "users_table", (void *)t, CGI_TABLE);
	}

	/* Return just table in case of AJAX request */
	if (table || del) {
		cgi_response_set_html(resp, "/wn/cgi/templates/config_auth_table.html");
		return 0;
	}

	/* Get auth type */
	_handle_auth_type(resp);

	cgi_response_add_parameter(resp, "menu_config", (void *) 8, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_auth.html");

	return 0;
}
