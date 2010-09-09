/*
 * firewall.c
 *
 *  Created on: Jul 19, 2010
 *      Author: Igor Kramer Pinotti (ipinotti@pd3.com.br)
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
#include <librouter/ip.h>

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
	int ret = 0, in=1, out=0, i=0;
	char interface[32];
	dev_family *fam;

	/* Clean buffers */
	memset(interface, 0, sizeof(interface));

	char *policy_type;
	char *ethernet0_in, *ethernet0_out;
	char *ethernet1_in, *ethernet1_out;
	char *m3g0_in, *m3g0_out;
	char *m3g1_in, *m3g1_out;
	char *m3g2_in, *m3g2_out;

	policy_type = _get_parameter(req, "policy-type");

	ethernet0_in  = _get_parameter(req, "ethernet0-input");
	ethernet0_out = _get_parameter(req, "ethernet0-output");
	ethernet1_in  = _get_parameter(req, "ethernet1-input");
	ethernet1_out = _get_parameter(req, "ethernet1-output");

	m3g0_in  = _get_parameter(req, "m3g0-input");
	m3g0_out = _get_parameter(req, "m3g0-output");
	m3g1_in  = _get_parameter(req, "m3g1-input");
	m3g1_out = _get_parameter(req, "m3g1-output");
	m3g2_in  = _get_parameter(req, "m3g2-input");
	m3g2_out = _get_parameter(req, "m3g2-output");

	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(eth);
		sprintf(interface, "%s%d", fam->linux_string, i);

		ret = librouter_acl_clean_iface_acls(interface);
		if (ret < 0)
			goto end;

		switch(i){
			case 0:
				if (strcmp(ethernet0_in,"--1")  != 0)
					ret = librouter_acl_apply_exist_chain_in_intf(interface,ethernet0_in,in);
				if (strcmp(ethernet0_out,"--1") != 0)
					ret = librouter_acl_apply_exist_chain_in_intf(interface,ethernet0_out,out);
				break;
			case 1:
				if (strcmp(ethernet1_in,"--1")  != 0)
					ret = librouter_acl_apply_exist_chain_in_intf(interface,ethernet1_in,in);
				if (strcmp(ethernet1_out,"--1") != 0)
					ret = librouter_acl_apply_exist_chain_in_intf(interface,ethernet1_out,out);
				break;
			default:
				break;
		}
	}


	ret = librouter_acl_apply_access_policy(policy_type);

end:
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
	char *name, *ip_src, *ip_dst, *policy, *portrange_start, *portrange_end, *protocol;
	char *mask_temp, *ip_temp;
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

	policy = _get_parameter(req, "policy");
	protocol = _get_parameter(req, "protocol");

	ip_src = _get_parameter(req, "src1");
	ip_dst = _get_parameter(req, "dest1");

	portrange_start = _get_parameter(req, "portnstart");
	portrange_end = _get_parameter(req, "portnend");


	acl.name = name;

	/* Create new ACL if one does not exist */
	if (!librouter_acl_exists(acl.name))
		librouter_acl_create_new(acl.name);

	/* Check for policy */
	if (!strcmp(policy, "accept"))
		acl.action = acl_accept;
	else
		if (!strcmp(policy, "drop"))
			acl.action = acl_drop;
		else
			if (!strcmp(policy, "reject"))
				acl.action = acl_reject;

	/* Check for protocol and port range*/
	if (!strcmp(protocol, "ip"))
		acl.protocol = ip;
	else {

		if (!strcmp(protocol, "tcp"))
			acl.protocol = tcp;
		else
			if (!strcmp(protocol, "udp"))
				acl.protocol = udp;

		if ( portrange_start != NULL && portrange_end != NULL )
			sprintf(acl.dst_portrange, "%s:%s ", portrange_start, portrange_end);
	}

	/* Check for addrs SRC and DEST */
	if (!strcmp(ip_src, "*"))
		strcpy(acl.src_address, "0.0.0.0/0 ");
	else {
		if (strchr(ip_src,'/') == NULL){
			snprintf(acl.src_address,strlen(ip_src)+4, "%s/32 ", ip_src);
		}
		else {
			ip_temp = strtok(ip_src,"/");
			mask_temp = strtok(NULL,"");

			acl.src_cidr = librouter_ip_netmask2cidr(mask_temp);
			if (acl.src_cidr < 0) {
				log_err("Invalid netmask in ip_src");
				return -1;
			}
			sprintf(acl.src_address, "%s/%d ", ip_temp, acl.src_cidr);

			ip_temp=NULL;
			mask_temp=NULL;
		}
	}
	if (!strcmp(ip_dst, "*"))
			strcpy(acl.dst_address, "0.0.0.0/0 ");
	else {
		if (strchr(ip_dst,'/') == NULL){
			snprintf(acl.dst_address,strlen(ip_dst)+4, "%s/32 ", ip_dst);
		}
		else {
			ip_temp = strtok(ip_dst,"/");
			mask_temp = strtok(NULL,"");

			acl.dst_cidr = librouter_ip_netmask2cidr(mask_temp);
			if (acl.dst_cidr < 0) {
				log_err("Invalid netmask in ip_src");
				return -1;
			}

			sprintf(acl.dst_address, "%s/%d ", ip_temp, acl.dst_cidr);

			ip_temp=NULL;
			mask_temp=NULL;
		}
	}


	librouter_acl_apply(&acl);

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return ret;
}

int handle_fw_del_rule (struct request *req, struct response *resp)
{
	int ret = 0;
	char * rule;
	char * warning_string = NULL;
	char cmd[256];

	memset(cmd, 0, sizeof(cmd));

	rule = _get_parameter(req, "del");

	if (!librouter_acl_exists(rule)) {
		ret = -1;
		goto end;
	}

	if (librouter_acl_get_refcount(rule)) {
		ret = -1;
		warning_string = malloc(115+strlen(rule));
		snprintf(warning_string,115+strlen(rule),"<br/> Rule (<b>%s</b>) in use, can't be deleted!<br/> <br/> First is necessary to disable rule on the interface!",rule);
		goto end;
	}

	sprintf(cmd, "/bin/iptables -F %s", rule); /* flush */
	if (system(cmd) != 0)
		ret = -1;

	sprintf(cmd, "/bin/iptables -X %s", rule); /* delete */
	if (system(cmd) != 0)
		ret = -1;

end:
	if (warning_string){
		cgi_response_add_parameter(resp, "err_msg", (void *) warning_string, CGI_STRING);
		free(warning_string);
		warning_string = NULL;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return ret;
}


int handle_config_firewall(struct request *req, struct response *resp)
{
	char *table;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	/* Check if returning just table (AJAX) */
	table = _get_parameter(req, "table");

	_get_default_policy(resp);

	_get_rules_table(resp);

	_print_page_javascript(resp);

	/* Return just table in case of AJAX request */
	if (table) {
		cgi_response_set_html(resp, "/wn/cgi/templates/config_firewall_table.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "menu_config", (void *) 3, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_firewall.html");

	return 0;
}

int handle_fw_view_rule(struct request *req, struct response *resp)
{
	char file[]="/var/rule_acl.conf";
	int ret = 0;
	FILE * acl_file;
	char * rule, * buffer;
	long lSize;
	size_t result;

	rule = _get_parameter(req, "view");

	acl_file = fopen (file, "w" );
	librouter_acl_dump(rule,acl_file,0);
	fclose(acl_file);

	acl_file = fopen(file, "r");

	// obtain file size:
	fseek (acl_file , 0 , SEEK_END);
	lSize = ftell (acl_file);
	rewind (acl_file);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char)*lSize);
	if (buffer == NULL)
		ret = -1;

	// copy the file into the buffer:
	result = fread (buffer,1,lSize,acl_file);
	if (result != lSize)
		ret = -1;

	fclose(acl_file);

	cgi_response_add_parameter(resp, "view_rule_buf", (void *) buffer, CGI_STRING);
	cgi_response_set_html(resp, "/wn/cgi/templates/fw_rules.html");

	free (buffer);
	return ret;

}


