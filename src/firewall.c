/*
 * firewall.c
 *
 *  Created on: Jul 19, 2010
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

#include <librouter/acl.h>
#include <librouter/args.h>
#include <librouter/iptc.h>
#include <librouter/device.h>

#include "web_config.h"
#include "interface.h"

static void _get_default_policy(struct response *resp)
{
	FILE *f;
	char buf[128];

	memset(buf, 0, sizeof(buf));

	f = tmpfile();
	librouter_acl_dump_policy(f);

	rewind(f);
	fread(buf, sizeof(buf), 1, f);

	if (strstr(buf, "accept"))
		cgi_response_add_parameter(resp, "policy_type", "accept", CGI_STRING);
	else
		cgi_response_add_parameter(resp, "policy_type", "drop", CGI_STRING);

	fclose(f);
}

static void _get_rules_table(struct response *resp)
{
	cgi_table *t;
	arglist *args;
	char chains[512];
	int i;

	memset(chains, 0, sizeof(chains));

	librouter_iptc_query_filter(chains);

	t = cgi_table_create(1, "rulename");

	if (chains[0]) {
		args = librouter_make_args(chains);
		for (i = 0; i < args->argc; i++) {
			cgi_table_add_row(t);
			cgi_table_add_value(t, "rulename", args->argv[i], CGI_STRING);
		}
		librouter_destroy_args(args);
	}

	cgi_response_add_parameter(resp, "rules_table", (void *) t, CGI_TABLE);
}

/**
 * _print_page_javascript	Print relevant javascript in page
 *
 * For now, we fetch the interface's rules to
 * show the right ones that are selected.
 *
 * @param resp
 */
static void _print_page_javascript(struct response *resp)
{
	char line[128];
	char javascript[512];
	char *chain;
	char interface[32];
	int i;
	dev_family *fam;


	/* Clean buffers */
	memset(line, 0, sizeof(line));
	memset(javascript, 0, sizeof(javascript));
	memset(interface, 0, sizeof(interface));

	/* Ethernet */
	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(eth);
		sprintf(interface, "%s%d", fam->linux_string, i);

		chain = librouter_iptc_filter_get_chain_for_iface(0, interface);
		if (chain != NULL) {
			web_dbg("INPUT chain for %s is %s\n", interface, chain);
			sprintf(line, "$('#%s-input-%s').attr('selected','selected');\n\t", interface, chain);
			strcat(javascript, line);
		}

		chain = librouter_iptc_filter_get_chain_for_iface(1, interface);
		if (chain != NULL) {
			web_dbg("OUTPUT chain for %s is %s\n", interface, chain);
			sprintf(line, "$('#%s-output-%s').attr('selected','selected');\n\t", interface, chain);
			strcat(javascript, line);
		}

	}

	/* 3G */
	for (i = 0; i < M3G_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(ppp);
		sprintf(interface, "%s%d", fam->linux_string, i);

		chain = librouter_iptc_filter_get_chain_for_iface(0, interface);
		if (chain != NULL) {
			web_dbg("INPUT chain for %s is %s\n", interface, chain);
			sprintf(line, "$('#%s-input-%s').attr('selected','selected');\n\t", interface, chain);
			strcat(javascript, line);
		}

		chain = librouter_iptc_filter_get_chain_for_iface(1, interface);
		if (chain != NULL) {
			web_dbg("OUTPUT chain for %s is %s\n", interface, chain);
			sprintf(line, "$('#%s-output-%s').attr('selected','selected');\n\t", interface, chain);
			strcat(javascript, line);
		}

	}

	cgi_response_add_parameter(resp, "javascript", javascript, CGI_STRING);
}

int handle_apply_fw_general_settings(struct request *req, struct response *resp)
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

int handle_fw_add_rule(struct request *req, struct response *resp)
{
	int ret = 0;
	char *name, *ip_src, *ip_dst;
	struct acl_config acl;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	memset(&acl, 0, sizeof(acl));

	name = _get_parameter(req, "rulename");

	if (name == NULL) {
		log_err("rule named is not specified\n");
		return -1;
	}

	ip_src = _get_parameter(req, "src");
	ip_dst = _get_parameter(req, "dst");

	acl.name = name;

	/* Create new ACL if one does not exist */
	if (!librouter_acl_exists(acl.name))
		librouter_acl_create_new(acl.name);

	librouter_acl_apply(&acl);

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return ret;
}

int handle_config_firewall(struct request *req, struct response *resp)
{
	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	_get_default_policy(resp);

	_get_rules_table(resp);

	_print_page_javascript(resp);

	cgi_response_add_parameter(resp, "menu_config", (void *) 3, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_firewall.html");

	return 0;
}
