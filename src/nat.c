/*
 * nat.c
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

#include <librouter/nat.h>
#include <librouter/args.h>
#include <librouter/device.h>
#include <librouter/ip.h>
#include <librouter/iptc.h>

#include "web_config.h"
#include "interface.h"

static void _get_rules_table(struct response *resp)
{
	cgi_table *t;
	arglist *args;
	char chains[512];
	int i;

	memset(chains, 0, sizeof(chains));

	librouter_iptc_query_nat(chains);

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
	char intf_web[32];
	int i;
	dev_family *fam;

	/* Clean buffers */
	memset(line, 0, sizeof(line));
	memset(javascript, 0, sizeof(javascript));
	memset(interface, 0, sizeof(interface));
	memset(intf_web, 0, sizeof(intf_web));


	web_dbg("Printing page - Javascript NAT");

	/* Ethernet */
	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(eth);
		sprintf(interface, "%s%d", fam->linux_string, i);

		chain = librouter_iptc_nat_get_chain_for_iface(0, interface);
		if (chain != NULL) {
			web_dbg("INPUT chain for %s is %s\n", interface, chain);
			sprintf(line, "$('#%s-input-%s').attr('selected','selected');\n\t", interface, chain);
			strcat(javascript, line);
		}

		chain = librouter_iptc_nat_get_chain_for_iface(1, interface);
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
		sprintf(intf_web,"%s%d", fam->web_string, i);

		chain = librouter_iptc_nat_get_chain_for_iface(0, interface);
		if (chain != NULL) {
			web_dbg("INPUT chain for %s == (%s) is %s\n", interface, intf_web, chain);
			sprintf(line, "$('#m%s-input-%s').attr('selected','selected');\n\t", intf_web, chain);
			strcat(javascript, line);
		}

		chain = librouter_iptc_nat_get_chain_for_iface(1, interface);
		if (chain != NULL) {
			web_dbg("OUTPUT chain for %s == (%s) is %s\n", interface, intf_web, chain);
			sprintf(line, "$('#m%s-output-%s').attr('selected','selected');\n\t", intf_web, chain);
			strcat(javascript, line);
		}

	}

	web_dbg("End of Print page - Javascript NAT");
	cgi_response_add_parameter(resp, "javascript", javascript, CGI_STRING);
}

int handle_config_nat(struct request *req, struct response *resp)
{
	char *table;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	/* Check if returning just table (AJAX) */
	table = _get_parameter(req, "table");

	_get_rules_table(resp);

	_print_page_javascript(resp);

	/* Return just table in case of AJAX request */
	if (table) {
		cgi_response_set_html(resp, "/wn/cgi/templates/config_nat_table.html");
		return 0;
	}

	cgi_response_add_parameter(resp, "ethernet_iface_num", (void *) ETHERNET_IFACE_NUM, CGI_INTEGER);
	cgi_response_add_parameter(resp, "m3g_iface_num", (void *) M3G_IFACE_NUM, CGI_INTEGER);
	cgi_response_add_parameter(resp, "menu_config", (void *) 4, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_nat.html");

	return 0;
}


int handle_apply_nat_general_settings(struct request *req, struct response *resp)
{
	int ret = 0, i=0;
	char interface[32];
	dev_family *fam;
	char * warning_string = NULL;


	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	/* Clean buffers */
	memset(interface, 0, sizeof(interface));

	char *ethernet0_in, *ethernet0_out;
	char *ethernet1_in, *ethernet1_out;
	char *m3g0_in, *m3g0_out;
	char *m3g1_in, *m3g1_out;
	char *m3g2_in, *m3g2_out;

	ethernet0_in  = _get_parameter(req, "eth0-input");
	ethernet0_out = _get_parameter(req, "eth0-output");
	ethernet1_in  = _get_parameter(req, "eth1-input");
	ethernet1_out = _get_parameter(req, "eth1-output");

	m3g0_in  = _get_parameter(req, "m3GModem0-input");
	m3g0_out = _get_parameter(req, "m3GModem0-output");
	m3g1_in  = _get_parameter(req, "m3GModem1-input");
	m3g1_out = _get_parameter(req, "m3GModem1-output");
	m3g2_in  = _get_parameter(req, "m3GModem2-input");
	m3g2_out = _get_parameter(req, "m3GModem2-output");

	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(eth);
		sprintf(interface, "%s%d", fam->linux_string, i);

		ret = librouter_nat_clean_iface_rules(interface);
		if (ret < 0)
			goto end;

		switch(i){
			case 0:
				if (strcmp(ethernet0_in,"--1")  != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,ethernet0_in,nat_chain_in);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ",ethernet0_in,interface);
						goto end;
					}
				}
				if (strcmp(ethernet0_out,"--1") != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,ethernet0_out,nat_chain_out);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ",ethernet0_out,interface);
						goto end;
					}
				}
				break;

			case 1:
				if (strcmp(ethernet1_in,"--1")  != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,ethernet1_in,nat_chain_in);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ",ethernet1_in,interface);
						goto end;
					}
				}
				if (strcmp(ethernet1_out,"--1") != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,ethernet1_out,nat_chain_out);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ",ethernet1_out,interface);
						goto end;
					}
				}
				break;

			default:
				break;
		}
	}


	for (i = 0; i < M3G_IFACE_NUM; i++) {
		fam = librouter_device_get_family_by_type(ppp);
		sprintf(interface, "%s%d", fam->linux_string, i);

		ret = librouter_nat_clean_iface_rules(interface);
		if (ret < 0)
			goto end;

		switch(i){
			case 0:
				if (strcmp(m3g0_in,"--1")  != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g0_in,nat_chain_in);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g0_in, interface);
						goto end;
					}
				}
				if (strcmp(m3g0_out,"--1") != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g0_out,nat_chain_out);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g0_out, interface);
						goto end;
					}
				}
				break;

			case 1:
				if (strcmp(m3g1_in,"--1")  != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g1_in,nat_chain_in);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g1_in, interface);
						goto end;
					}
				}
				if (strcmp(m3g1_out,"--1") != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g1_out,nat_chain_out);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g1_out, interface);
						goto end;
					}
				}
				break;

			case 2:
				if (strcmp(m3g2_in,"--1")  != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g2_in,nat_chain_in);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g2_in, interface);
						goto end;
					}
				}
				if (strcmp(m3g2_out,"--1") != 0){
					ret = librouter_nat_bind_interface_to_rule(interface,m3g2_out,nat_chain_out);
					if (ret < 0){
						warning_string = malloc(256);
						sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The selected rule (<b>%s</b>) can not be applied this way in <u>%s</u>! ", m3g2_out, interface);
						goto end;
					}
				}
				break;

			default:
				break;
		}
	}

end:
	web_dbg("Final Check for applied confs. NAT --> %s", ret < 0 ? "fail" : "ok");

	if (warning_string != NULL){
		cgi_response_add_parameter(resp, "err_msg", (void *) warning_string, CGI_STRING);
		free(warning_string);
		warning_string = NULL;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return ret;

}

int handle_nat_add_rule(struct request *req, struct response *resp)
{
	int ret = 0;
	char *name, *protocol, *translate, *intf_addr;
	char *ip_src, *ip_dst, *ip_nat_addr1, *ip_nat_addr2;
	char *portrange_start, *portrange_end, *portrange_nat_start, *portrange_nat_end;
	char *mask_temp, *ip_temp;
	struct nat_config nat;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	memset(&nat, 0, sizeof(nat));

	/* Get Parameters from webpage */
	name = _get_parameter(req, "rulename");
	if (name == NULL) {
		log_err("%% Rule named is not specified\n");
		ret = -1;
		goto end;
	}

	intf_addr = _get_parameter(req, "interface_address");
	protocol = _get_parameter(req, "protocol");
	translate = _get_parameter(req, "translate");

	ip_src = _get_parameter(req, "src1");
	ip_dst = _get_parameter(req, "dest1");

	ip_nat_addr1 = _get_parameter(req, "nat_addr1");
	ip_nat_addr2 = _get_parameter(req, "nat_addr2");

	portrange_start = _get_parameter(req, "portnstart");
	portrange_end = _get_parameter(req, "portnend");
	portrange_nat_start = _get_parameter(req, "portnat_start");
	portrange_nat_end = _get_parameter(req, "portnat_end");

	/* Rule Name */
	nat.name = name;

	/* Mode */
	nat.mode = add_nat;

	web_dbg("(1) - Check after get parameters");

	/* Check for protocol and port range*/
	if (!strcmp(protocol, "ip"))
		nat.protocol = proto_ip;
	else {

		if (!strcmp(protocol, "tcp"))
			nat.protocol = proto_tcp;
		else
			if (!strcmp(protocol, "udp"))
				nat.protocol = proto_udp;

		if ( portrange_start != NULL && portrange_end != NULL )
			sprintf(nat.dst_portrange, "%s:%s ", portrange_start, portrange_end);
	}

	web_dbg("(2) - Check after protocol and P_range");

	/* Source */
	if (!strcmp(ip_src, "*"))
		strcpy(nat.src_address, "0.0.0.0/0 ");
	else {
		if (strchr(ip_src,'/') == NULL){
			snprintf(nat.src_address,strlen(ip_src)+4, "%s/32 ", ip_src);
		}
		else {
			ip_temp = strtok(ip_src,"/");
			mask_temp = strtok(NULL,"");

			nat.src_cidr = librouter_ip_netmask2cidr(mask_temp);
			if (nat.src_cidr < 0) {
				log_err("%% Invalid netmask in ip_src");
				ret = -1;
				goto end;
			}

			sprintf(nat.src_address, "%s/%d ", ip_temp, nat.src_cidr);

			ip_temp=NULL;
			mask_temp=NULL;
		}
	}

	web_dbg("(3) - Check after source");

	/* Destination */
	if (!strcmp(ip_dst, "*"))
		strcpy(nat.dst_address, "0.0.0.0/0 ");
	else {
		if (strchr(ip_dst,'/') == NULL){
			snprintf(nat.dst_address,strlen(ip_dst)+4, "%s/32 ", ip_dst);
		}
		else {
			ip_temp = strtok(ip_dst,"/");
			mask_temp = strtok(NULL,"");

			nat.dst_cidr = librouter_ip_netmask2cidr(mask_temp);
			if (nat.dst_cidr < 0) {
				log_err("%% Invalid netmask in ip_dst");
				ret = -1;
				goto end;
			}

			sprintf(nat.dst_address, "%s/%d ", ip_temp, nat.dst_cidr);

			ip_temp=NULL;
			mask_temp=NULL;
		}
	}

	web_dbg("(4) - Check after destination");


	/* Action Translate*/
	if (!strcmp(translate, "change-source-to"))
		nat.action = snat;
	else
		if (!strcmp(translate, "change-destination-to"))
			nat.action = dnat;

	web_dbg("(5) - Check after translate --> %s",translate);

	/* NAT Address Target */
	if (!strcmp(translate, "change-source-to")){
		if (intf_addr){
			nat.masquerade = 1;
		}
		else {
			if (ip_nat_addr1 != NULL){
				if (ip_nat_addr2 != NULL) {
					strcpy(nat.nat_addr1, ip_nat_addr1);
					strcpy(nat.nat_addr2, ip_nat_addr2);
				}
				else
					strcpy(nat.nat_addr1, ip_nat_addr1);
			}
		}
	}
	else {
		if (ip_nat_addr1 != NULL){
			if (ip_nat_addr2 != NULL) {
				strcpy(nat.nat_addr1, ip_nat_addr1);
				strcpy(nat.nat_addr2, ip_nat_addr2);
			}
			else
				strcpy(nat.nat_addr1, ip_nat_addr1);
		}
	}

	web_dbg("(6) - Check after NAT addr, --> intf_addr =-> %s", intf_addr);


	/* NAT Port Range Target */
	if ( !strcmp(protocol,"tcp") || !strcmp(protocol,"udp"))
		if ( portrange_nat_start != NULL  && portrange_nat_end != NULL){
			strcpy(nat.nat_port1, portrange_nat_start);
			strcpy(nat.nat_port2, portrange_nat_end);
		}

	web_dbg("(7) - Check after NAT P_range");

	ret = librouter_nat_apply_rule(&nat);

	web_dbg("(8) - Check after applied rule --> %s", ret < 0 ? "fail" : "ok");


	if( !librouter_nat_rule_exists(name)){
		ret = -1;
		web_dbg("%% NAT rule NOT created");
	}

	web_dbg("(9) - Check final --> %s", ret < 0 ? "fail" : "ok");

end:
	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");

	return ret;
}

int handle_nat_del_rule(struct request *req, struct response *resp)
{
	int ret = 0;
	char * rule;
	char * warning_string = NULL;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	rule = _get_parameter(req, "del");

	if (!librouter_nat_rule_exists(rule)) {
		ret = -1;
		goto end;
	}

	if (librouter_nat_rule_refcount(rule)) {
		ret = -1;
		warning_string = malloc(115+strlen(rule));
		snprintf(warning_string,115+strlen(rule),"<br/> Rule (<b>%s</b>) in use, can't be deleted!<br/> <br/> First is necessary to disable rule on the interface!",rule);
		goto end;
	}

	ret = librouter_nat_delete_rule(rule);

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

int handle_nat_view_rule(struct request *req, struct response *resp)
{
	char file[]="/var/rule_nat.conf";
	char line[128];
	int ret = 0;
	FILE * nat_file;
	char * rule = NULL, * buffer = NULL;
	long lSize;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	rule = _get_parameter(req, "view");

	nat_file = fopen (file, "w" );
	librouter_nat_dump(rule,nat_file,0);
	fclose(nat_file);

	nat_file = fopen(file, "r");

	// obtain file size:
	fseek (nat_file , 0 , SEEK_END);
	lSize = ftell (nat_file);
	rewind (nat_file);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char)*lSize);
	memset(buffer,0,sizeof(buffer));

	if (buffer == NULL)
		ret = -1;

	// copy the file into the buffer:
	while (fgets(line, 128, nat_file) != NULL){
		strcat(buffer,line);
	};

	fclose(nat_file);

	cgi_response_add_parameter(resp, "view_rule_buf", (void *) buffer, CGI_STRING);
	cgi_response_set_html(resp, "/wn/cgi/templates/nat_rules.html");

	free (buffer);
	return ret;
}
