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
	_get_auth_string(FILE_PAM_LOGIC, buf);
	cgi_response_add_parameter(resp, "cli_auth_type", (char *) buf, CGI_STRING);

	memset(buf, 0, sizeof(buf));
	sprintf(buf,"web-");
	_get_auth_string(FILE_PAM_WEB, buf);
	cgi_response_add_parameter(resp, "web_auth_type", (char *) buf, CGI_STRING);

	return 0;
}

static int _handle_auth_servers(struct response *resp)
{
	int i;
	char server[32], key[32], timeout[32];
	struct auth_server tacacs_servers[AUTH_MAX_SERVERS];
	struct auth_server radius_servers[AUTH_MAX_SERVERS];

	memset(tacacs_servers, 0, sizeof(tacacs_servers));
	memset(radius_servers, 0, sizeof(radius_servers));

	librouter_pam_get_tacacs_servers(tacacs_servers);
	librouter_pam_get_radius_servers(radius_servers);

	for (i = 0; i < AUTH_MAX_SERVERS; i++) {
		sprintf(server, "tacacs_server_%d", i+1);
		sprintf(key, "tacacs_key_%d", i+1);
		sprintf(timeout, "tacacs_timeout_%d", i+1);

		if (tacacs_servers[i].ipaddr)
			cgi_response_add_parameter(resp, server, tacacs_servers[i].ipaddr,
			                CGI_STRING);

		if (tacacs_servers[i].key)
			cgi_response_add_parameter(resp, key, tacacs_servers[i].key, CGI_STRING);

		if (tacacs_servers[i].timeout)
			cgi_response_add_parameter(resp, timeout,
			                (void *) tacacs_servers[i].timeout, CGI_INTEGER);
	}

	for (i = 0; i < AUTH_MAX_SERVERS; i++) {
		sprintf(server, "radius_server_%d", i+1);
		sprintf(key, "radius_key_%d", i+1);
		sprintf(timeout, "radius_timeout_%d", i+1);

		if (radius_servers[i].ipaddr)
			cgi_response_add_parameter(resp, server, radius_servers[i].ipaddr,
			                CGI_STRING);

		if (radius_servers[i].key)
			cgi_response_add_parameter(resp, key, radius_servers[i].key, CGI_STRING);

		if (radius_servers[i].timeout)
			cgi_response_add_parameter(resp, timeout,
			                (void *) radius_servers[i].timeout, CGI_INTEGER);
	}

	librouter_pam_free_servers(AUTH_MAX_SERVERS, tacacs_servers);
	librouter_pam_free_servers(AUTH_MAX_SERVERS, radius_servers);

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

	/* Password match ? */
	if (strcmp(passwd, confirm))
		return -1;

	/*Add user sequence*/
	if (librouter_pam_cmds_del_user_from_group(username) < 0 )
		return -1;

	if (librouter_pam_add_user(username, passwd) < 0)
		return -1;

	/* Adicionado privilege 15 hardcoded for now */
	if (librouter_pam_cmds_add_user_to_group(username, "15") < 0)
		return -1;

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
		librouter_pam_config_mode(mode, FILE_PAM_LOGIC);

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
	struct auth_server radius_server[AUTH_MAX_SERVERS];
	int i;

	memset(radius_server, 0, sizeof(radius_server));

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	librouter_pam_del_radius_server(NULL); /* Delete all configured servers */

	for (i=0; i< AUTH_MAX_SERVERS;i++) {
		char server[32], key[32], timeout[32];
		sprintf(server, "radius_server_%d", i+1);
		sprintf(key, "radius_key_%d", i+1);
		sprintf(timeout, "radius_timeout_%d", i+1);

		radius_server[i].ipaddr = _get_parameter(req, server);
		radius_server[i].key = _get_parameter(req, key);
		if (_get_parameter(req, timeout) != NULL)
			radius_server[i].timeout = atoi(_get_parameter(req, timeout));

		/* Apply settings if ip is defined */
		if (radius_server[i].ipaddr != NULL)
			librouter_pam_add_radius_server(&radius_server[i]);
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
	struct auth_server tacacs_server[AUTH_MAX_SERVERS];
	int i;

	memset(tacacs_server, 0, sizeof(tacacs_server));

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	librouter_pam_del_tacacs_server(NULL); /* Delete all configured servers */

	for (i=0; i< AUTH_MAX_SERVERS;i++) {
		char server[32], key[32], timeout[32];
		sprintf(server, "tacacs_server_%d", i+1);
		sprintf(key, "tacacs_key_%d", i+1);
		sprintf(timeout, "tacacs_timeout_%d", i+1);

		tacacs_server[i].ipaddr = _get_parameter(req, server);
		tacacs_server[i].key = _get_parameter(req, key);
		if (_get_parameter(req, timeout) != NULL)
			tacacs_server[i].timeout = atoi(_get_parameter(req, timeout));

		/* Apply settings if ip is defined */
		if (tacacs_server[i].ipaddr != NULL)
			librouter_pam_add_tacacs_server(&tacacs_server[i]);
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
	if (del != NULL){
		/* Delete user sequence*/
		librouter_pam_cmds_del_user_from_group(del);
		librouter_pam_del_user(del);
	}
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

	/* Get servers */
	_handle_auth_servers(resp);

	cgi_response_add_parameter(resp, "menu_config", (void *) 8, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_auth.html");

	return 0;
}
