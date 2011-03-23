/*
 * interface.c
 *
 *  Created on: May 28, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
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

#include <librouter/libtime.h>
#include <librouter/options.h>
#include <librouter/device.h>
#include <librouter/dhcp.h>
#include <librouter/exec.h>
#include <librouter/ip.h>
#include <librouter/dev.h>
#include <librouter/ppp.h>
#include <librouter/modem3G.h>
#include <librouter/usb.h>
#include <librouter/ppp.h>

#include "web_config.h"
#include "interface.h"


/**
 * _convert_string_web_to_linux		Convert linux to web string
 *
 * @param type
 * @param linux_interface
 * @return Pointer to web string, needs to be free'd after use
 */
static char *_convert_string_linux_to_web(device_type type, char *linux_interface)
{
	char * web_interface;
	dev_family *fam = librouter_device_get_family_by_type(type);

	if (fam == NULL)
		return NULL;

	web_interface = malloc(32);

	/* Position the string at the index */
	linux_interface += strlen(fam->linux_string);
	snprintf(web_interface, 32, "%s%s", fam->web_string, linux_interface);

	return web_interface;
}

/**
 * _convert_string_web_to_linux		Convert web to linux string
 *
 * @param type
 * @param web_interface
 * @return Pointer to linux string, needs to be free'd after use
 */
static char *_convert_string_web_to_linux(device_type type, char *web_interface)
{
	char *linux_interface;
	dev_family *fam = librouter_device_get_family_by_type(type);

	if (fam == NULL)
		return NULL;

	linux_interface = malloc(32);

	/* Position the string at the index */
	web_interface += strlen(fam->web_string);
	snprintf(linux_interface, 32, "%s%s", fam->linux_string, web_interface);

	return linux_interface;
}

/**
 * _apply_eth_settings	Apply settings to an ethernet interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_eth_settings(struct request *req, struct response *resp)
{
	char *interface, *dhcpc, *ipaddr, *ipmask, *speed, *up;
	char daemon_dhcpc[32];
	char *linux_interface;

	/* Handle Ethernet */
	interface = _get_parameter(req, "name");
	dhcpc = _get_parameter(req, "dhcpc");
	ipaddr = _get_parameter(req, "ipaddr");
	ipmask = _get_parameter(req, "ipmask");
	speed = _get_parameter(req, "speed");
	up = _get_parameter(req, "up");

	web_dbg("interface is %s\n", interface);
	linux_interface = _convert_string_web_to_linux(eth, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}

	snprintf(daemon_dhcpc, 32, DHCPC_DAEMON, linux_interface);

	if (dhcpc)
		librouter_exec_daemon (daemon_dhcpc);
	else {
		/* Kill DHCP client daemon if it is running */
		if (librouter_exec_check_daemon(daemon_dhcpc))
			librouter_kill_daemon(daemon_dhcpc);

		librouter_ip_ethernet_set_addr(linux_interface, ipaddr, ipmask);
	}

	if (up)
		librouter_dev_noshutdown(linux_interface);
	else
		librouter_dev_shutdown(linux_interface);

	free(linux_interface);
	return 0;
}

/**
 * _apply_lo_settings	Apply settings to a loopback interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_lo_settings(struct request *req, struct response *resp)
{
	char *interface, *ipaddr, *ipmask, *up;
	char *linux_interface;

	interface = _get_parameter(req, "name");
	ipaddr = _get_parameter(req, "ipaddr");
	ipmask = _get_parameter(req, "ipmask");
	up = _get_parameter(req, "up");

	web_dbg("interface is %s\n", interface);
	linux_interface = _convert_string_web_to_linux(lo, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}

	librouter_ip_interface_set_addr(linux_interface, ipaddr, ipmask);

	if (up)
		librouter_dev_noshutdown(linux_interface);
	else
		librouter_dev_shutdown(linux_interface);

	free(linux_interface);
	return 0;
}

#ifdef OPTION_MODEM3G
/**
 * _apply_3g_settings	Apply settings to a 3G interface
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
static int _apply_3g_settings(struct request *req, struct response *resp)
{
	char *apn0, *user0, *pass0, *apn1, *user1, *pass1;
	char *backup_method, *backup_interface, *ping_addr, *sim_order, *up;
	char *linux_interface, *interface;
#ifdef AVOID_SAME_BCKP_INTF
	char *intf_return_backup_error = malloc(24);
#endif
	char *warning_string = NULL;
	char *default_route, *default_route_metric;
	int major=-1, ret=-1, warning_intf=0;


	interface = _get_parameter(req, "name");
	apn0 = _get_parameter(req, "apn0");
	user0 = _get_parameter(req, "user0");
	pass0 = _get_parameter(req, "pass0");
	apn1 = _get_parameter(req, "apn1");
	user1 = _get_parameter(req, "user1");
	pass1 = _get_parameter(req, "pass1");
	sim_order = _get_parameter(req, "sim_order");
	backup_method = _get_parameter(req,"backup_method");
	ping_addr = _get_parameter(req,"ping_addr");
	backup_interface = _get_parameter(req,"backup_interface");
	default_route = _get_parameter(req,"gw");
	default_route_metric = _get_parameter(req,"gwmetric");
	up = _get_parameter(req, "up");


	web_dbg("(1) - interface is %s\n", interface);

	linux_interface = _convert_string_web_to_linux(ppp, interface);
	if (linux_interface == NULL) {
		log_err("Linux interface not found\n");
		return -1;
	}
	major = librouter_device_get_major(linux_interface, str_linux);
	web_dbg("(2) - Major is %d, Linux Intf is %s\n", major, linux_interface);

	/* Realiza o shutdown da interface 3G para aplicar as configurações */
	ret = librouter_ppp_backupd_set_shutdown_3Gmodem(linux_interface);
	web_dbg("(3) - Check for problems after shutdown 3Gmodem for config. (ret = %s)\n", ret < 0 ? "fail" : "ok" );

	ret = librouter_ppp_reload_backupd();
	web_dbg("(4) - Check for problems after reload backupd (ret = %s)\n", ret < 0 ? "fail" : "ok" );

	/* Verificação necessaria para confirmar que a interface 3G realmente foi desativada antes de configura-la*/
	while (librouter_dev_exists(linux_interface) == 1)
		usleep (4800000);

	ret = librouter_ppp_backupd_set_no_backup_interface(linux_interface);
	web_dbg("(5) - Check for problems after set no backup interface - shutdown intf (ret = %s)\n", ret < 0 ? "fail" : "ok" );

	if (ret < 0)
		goto end;
	else
		ret = -1;

	/* Should install default route */
	librouter_ppp_backupd_set_default_route(linux_interface, (default_route != NULL ? 1 : 0));
	if (default_route_metric != NULL)
		librouter_ppp_backupd_set_default_metric(linux_interface, atoi(default_route_metric));


	if (major != BTIN_M3G_ALIAS){
		ret = librouter_modem3g_set_apn(apn0, major);
		ret = librouter_modem3g_set_username(user0, major);
		ret = librouter_modem3g_set_password(pass0, major);
		web_dbg("(6) - Check for problems after set APN,USER,PASS [simple sim] (ret = %s)\n", ret < 0 ? "fail" : "ok" );

		if (ret < 0)
			goto end;
		else
			ret = -1;
	}
	else{
		ret = librouter_modem3g_sim_set_info_infile(0,"apn",apn0);
		ret = librouter_modem3g_sim_set_info_infile(0,"username",user0);
		ret = librouter_modem3g_sim_set_info_infile(0,"password",pass0);
		web_dbg("(6) - Check for problems after set APN,USER,PASS [dual sim-0] (ret = %s)\n", ret < 0 ? "fail" : "ok" );
		ret = librouter_modem3g_sim_set_info_infile(1,"apn",apn1);
		ret = librouter_modem3g_sim_set_info_infile(1,"username",user1);
		ret = librouter_modem3g_sim_set_info_infile(1,"password",pass1);
		web_dbg("(6) - Check for problems after set APN,USER,PASS [dual sim-1] (ret = %s)\n", ret < 0 ? "fail" : "ok" );

		if (ret < 0)
			goto end;
		else
			ret = -1;

		switch (atoi(sim_order)){
			case 0:
				ret = librouter_modem3g_sim_order_set_allinfo(0, 1,linux_interface, major);
				break;
			case 1:
				ret = librouter_modem3g_sim_order_set_allinfo(1, 0,linux_interface, major);
				break;
			case 2:
				ret = librouter_modem3g_sim_order_set_allinfo(0, -1,linux_interface, major);
				break;
			case 3:
				ret = librouter_modem3g_sim_order_set_allinfo(1, -1,linux_interface, major);
				break;
		}
		web_dbg("(6.1) - Check for problems after set SIM Order (ret = %s)\n", ret < 0 ? "fail" : "ok" );

		if (ret < 0)
				goto end;
			else
				ret = -1;
	}

	if (atoi(backup_interface) != -1)
		switch (atoi(backup_method)){
			case 0:
				ret = librouter_ppp_backupd_set_backup_method(linux_interface,"link",NULL);
				break;
			case 1:
				ret = librouter_ppp_backupd_set_backup_method(linux_interface,"ping",ping_addr);
				break;
		}
	else
		ret = 0;

	web_dbg("(7) - Check for problems after set backup method (ret = %s)\n", ret < 0 ? "fail" : "ok" );

	if (ret < 0)
		goto end;
	else
		ret = -1;

	switch (atoi(backup_interface)){
		case -1:
			ret = librouter_ppp_backupd_set_no_backup_interface(linux_interface);
#ifdef AVOID_SAME_BCKP_INTF
			intf_return_backup_error = NULL;
#endif
			web_dbg("(8) - Check for problems after set no backup interface - (ret = %s)\n", ret < 0 ? "fail" : "ok" );
			break;
		case 0:
#ifdef AVOID_SAME_BCKP_INTF
			//WARNING: Linha de código a seguir substituida para possibilitar duas interfaces backups distintas de monitorar uma mesma interface
			ret = librouter_ppp_backupd_set_backup_interface_avoiding_same_bckp_intf(linux_interface,"ethernet0",intf_return_backup_error);
#else
			ret = librouter_ppp_backupd_set_backup_interface(linux_interface,"ethernet0");
#endif
			web_dbg("(8) - Check for problems after set backup interface - ethernet0 - (ret = %s)\n", ret < 0 ? "fail" : "ok");
			break;
		case 1:
#ifdef AVOID_SAME_BCKP_INTF
			//WARNING: Linha de código a seguir substituida para possibilitar duas interfaces backups distintas de monitorar uma mesma interface
			ret = librouter_ppp_backupd_set_backup_interface_avoiding_same_bckp_intf(linux_interface,"ethernet1",intf_return_backup_error);
#else
			ret = librouter_ppp_backupd_set_backup_interface(linux_interface,"ethernet1");
#endif
			web_dbg("(8) - Check for problems after set backup interface - ethernet1 - (ret = %s)\n", ret < 0 ? "fail" : "ok");
			break;
		case 2:
#ifdef AVOID_SAME_BCKP_INTF
			//WARNING: Linha de código a seguir substituida para possibilitar duas interfaces backups distintas de monitorar uma mesma interface
			ret = librouter_ppp_backupd_set_backup_interface_avoiding_same_bckp_intf(linux_interface,"m3G1",intf_return_backup_error);
#else
			ret = librouter_ppp_backupd_set_backup_interface(linux_interface,"m3G1");
#endif
			web_dbg("(8) - Check for problems after set backup interface - m3G1 - (ret = %s)\n", ret < 0 ? "fail" : "ok");
			break;
		case 3:
#ifdef AVOID_SAME_BCKP_INTF
			//WARNING: Linha de código a seguir substituida para possibilitar duas interfaces backups distintas de monitorar uma mesma interface
			ret = librouter_ppp_backupd_set_backup_interface_avoiding_same_bckp_intf(linux_interface,"m3G2",intf_return_backup_error);
#else
			ret = librouter_ppp_backupd_set_backup_interface(linux_interface,"m3G2");
#endif
			web_dbg("(8) - Check for problems after set backup interface - m3G2 - (ret = %s)\n", ret < 0 ? "fail" : "ok");
			break;
	}

#ifdef AVOID_SAME_BCKP_INTF
//WARNING: Bloco de código retirado para possibilitar duas interfaces backups distintas de monitorar uma mesma interface

	web_dbg("(9) - Check for problems after set backup interface (ret = %s) - Returned interface error = %s\n", ret < 0 ? "fail" : "ok", intf_return_backup_error);

	if ((intf_return_backup_error != NULL) && (ret < 0)){
		warning_string = malloc(256);
		sprintf(warning_string,"<br/> An error happened when applying changes!<br/> <br/> The backup interface selected is already set by %s! ",_convert_string_linux_to_web(ppp,intf_return_backup_error) );
	}

#else
	web_dbg("(9) - Check for problems after set backup interface (ret = %s)", ret < 0 ? "fail" : "ok");

	if (ret < 0){
		warning_string = malloc(256);
		sprintf(warning_string,"<br/> An error happened when applying changes!<br/>");
	}
#endif

	web_dbg("(10) - Check for interface status during configuration = %s",librouter_ppp_backupd_is_interface_3G_enable(linux_interface) == 1 ? "ON" : "OFF");


	if (up != NULL){
		if (librouter_usb_device_is_modem( librouter_usb_get_realport_by_aliasport(major) ) < 0){
			if (ret < 0)
				strcat(warning_string, "<br/><br/> The interface is not connected or is not a modem!");
			else{
				warning_string = malloc(64);
				strcpy(warning_string,"<br/> The interface is not connected or is not a modem!");
				warning_intf = 1;
			}
		}
		if (librouter_ppp_backupd_set_no_shutdown_3Gmodem(linux_interface) < 0)
			ret = -1;
		web_dbg("(11) - Check for problems after set no shutdown 3gmodem (ret = %s) && (warning_intf = %d)\n", ret < 0 ? "fail" : "ok", warning_intf);
	}


end:
	if (ret == 0)
		ret = librouter_ppp_reload_backupd(); /* Tell backupd that configuration has changed */

	if (warning_intf)
		ret = -1;

	web_dbg("(F-12) - Check for problems after all setted (ret = %s) && (warning_intf = %d)\n", ret < 0 ? "fail" : "ok", warning_intf);

	if (warning_string != NULL)
		cgi_response_add_parameter(resp, "err_msg", (void *) warning_string, CGI_STRING);

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);

	free(linux_interface);
	linux_interface=NULL;

#ifdef AVOID_SAME_BCKP_INTF
	free(intf_return_backup_error);
	intf_return_backup_error=NULL;
#endif

	if (warning_string)
		free(warning_string);

	return 0;
}
#endif /* OPTION_MODEM3G */

/**
 * handle_apply_intf_settings	Apply settings to a network interface
 *
 * This function will be called via CGI by the web browser, to apply
 * the form with interface information. Depending on the interface's type,
 * the appropriate handler will be called.
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
int handle_apply_intf_settings(struct request *req, struct response *resp)
{
	char *interface = NULL;
	int ret = -1;
	dev_family *fam;

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	interface = _get_parameter(req, "name");
	if (interface == NULL) {
		log_err("NULL interface received by handler\n");
		return ret;
	}

	fam = librouter_device_get_family_by_name(interface, str_web);

	/* Test for interface type */
	switch (fam->type) {
	case eth:
		ret = _apply_eth_settings(req, resp);
		break;
	case lo:
		ret = _apply_lo_settings(req, resp);
		break;
#ifdef OPTION_MODEM3G
	case ppp:
		ret = _apply_3g_settings(req, resp);
		break;
#endif
	default:
		break;
	}

	cgi_response_add_parameter(resp, "success", (void *) ret, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/msgs.html");
	return ret;
}

/**
 * _fetch_eth_info	Fetch ethernet information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_eth_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;
	dev_family *fam;

	fam = librouter_device_get_family_by_type(eth);

	if (fam == NULL){
		web_dbg("result NULL after librouter_device_get_family_by_type");
		return NULL;
	}
	/* Create ethernet table */
	t = cgi_table_create(6, "name", "speed", "ipaddr", "ipmask", "dhcpc", "up");

	for (i = 0; i < ETHERNET_IFACE_NUM; i++) {
		snprintf(iface, 16, "%s%d", fam->linux_string, i);

		if (librouter_ip_iface_get_config(iface, &conf, NULL) < 0){
			web_dbg("result NULL after librouter_ip_iface_get_config -- (%s)",iface);
			return NULL;
		}

		snprintf(iface, 16, "%s%d", fam->web_string, i);

		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "link", (void *) conf.running, CGI_INTEGER);
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "dhcpc", (void *) conf.dhcpc, CGI_INTEGER);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}

/**
 * _fetch_lo_info	Fetch loopback information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_lo_info(void)
{
	cgi_table *t;
	char iface[16];
	int i;
	struct interface_conf conf;
	dev_family *fam;

	fam = librouter_device_get_family_by_type(lo);

	if (fam == NULL)
		return NULL;

	/* Create loopback table */
	t = cgi_table_create(4, "name", "ipaddr", "ipmask", "up");

	for (i = 0; i < LOOPBACK_IFACE_NUM; i++) {
		snprintf(iface, 16, "%s%d", fam->linux_string, i);
		if (librouter_ip_iface_get_config(iface, &conf, NULL) < 0){
			web_dbg("return NULL after librouter_ip_iface_get_config --(%s)",iface);
			return NULL;
		}

		snprintf(iface, 16,"%s%d", fam->web_string, i);

		cgi_table_add_row(t);

		cgi_table_add_value(t, "name", (char *) iface, CGI_STRING);
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "up", (void *) conf.up, CGI_INTEGER);
	}

	return t;
}


#ifdef OPTION_MODEM3G
/**
 * _fetch_3g_info	Fetch 3G information
 *
 * @return Pointer to cgi_table to be put in response
 */
static cgi_table * _fetch_3g_info(void)
{
	cgi_table *t;
	char iface[16];
	char intf_back[24];
	int i;
	struct interface_conf conf;
	ppp_config ppp_cfg;
	dev_family *fam;

	fam = librouter_device_get_family_by_type(ppp);

	if (fam == NULL)
		return NULL;

	/* Create loopback table */
	t = cgi_table_create(17, "name", "apn0", "user0", "pass0", "apn1",
			         "user1", "pass1", "sim_order", "up",
			         "backup_method", "backup_interface",
			         "ping_addr", "ipaddr", "ipmask", "ippeer", "gw", "gwmetric");

	for (i = 0; i < M3G_IFACE_NUM; i++) {
		snprintf(iface, 16, "%s%d", fam->linux_string, i);
		if (librouter_ip_iface_get_config(iface, &conf, NULL) < 0)
			return NULL;

		if (librouter_ppp_get_config(i, &ppp_cfg) < 0)
			return NULL;

		snprintf(iface, 16, "%s%d", fam->web_string, i);
		cgi_table_add_row(t);

		cgi_table_add_value(t, "name",  (char *) iface, CGI_STRING);
		if (i == BTIN_M3G_ALIAS){
			if (!ppp_cfg.sim_main.sim_num){
				cgi_table_add_value(t, "apn0",  (char *) ppp_cfg.sim_main.apn, CGI_STRING);
				cgi_table_add_value(t, "user0", (char *) ppp_cfg.sim_main.username, CGI_STRING);
				cgi_table_add_value(t, "pass0", (char *) ppp_cfg.sim_main.password, CGI_STRING);
				cgi_table_add_value(t, "apn1",  (char *) ppp_cfg.sim_backup.apn, CGI_STRING);
				cgi_table_add_value(t, "user1", (char *) ppp_cfg.sim_backup.username, CGI_STRING);
				cgi_table_add_value(t, "pass1", (char *) ppp_cfg.sim_backup.password, CGI_STRING);
			}
			else {
				cgi_table_add_value(t, "apn1",  (char *) ppp_cfg.sim_main.apn, CGI_STRING);
				cgi_table_add_value(t, "user1", (char *) ppp_cfg.sim_main.username, CGI_STRING);
				cgi_table_add_value(t, "pass1", (char *) ppp_cfg.sim_main.password, CGI_STRING);
				cgi_table_add_value(t, "apn0",  (char *) ppp_cfg.sim_backup.apn, CGI_STRING);
				cgi_table_add_value(t, "user0", (char *) ppp_cfg.sim_backup.username, CGI_STRING);
				cgi_table_add_value(t, "pass0", (char *) ppp_cfg.sim_backup.password, CGI_STRING);
			}

			if (librouter_modem3g_sim_order_is_enable()){
				if (!librouter_modem3g_sim_order_get_mainsim())
					cgi_table_add_value(t, "sim_order", (char *) "sim-order-0", CGI_STRING);
				else
					cgi_table_add_value(t, "sim_order", (char *) "sim-order-1", CGI_STRING);
			}
			else
				if (!librouter_modem3g_sim_order_get_mainsim())
					cgi_table_add_value(t, "sim_order", (char *) "sim-order-2", CGI_STRING);
				else
					cgi_table_add_value(t, "sim_order", (char *) "sim-order-3", CGI_STRING);
		}
		else{
			cgi_table_add_value(t, "apn0", (char *) ppp_cfg.sim_main.apn, CGI_STRING);
			cgi_table_add_value(t, "user0", (char *) ppp_cfg.sim_main.username, CGI_STRING);
			cgi_table_add_value(t, "pass0", (char *) ppp_cfg.sim_main.password, CGI_STRING);
		}

		snprintf(intf_back,24,"backup-intf-%s",ppp_cfg.bckp_conf.main_intf_name);
		cgi_table_add_value(t, "backup_interface", (char *) intf_back, CGI_STRING);

		if (ppp_cfg.bckp_conf.method == BCKP_METHOD_LINK)
			cgi_table_add_value(t, "backup_method", (char *) "backup_method-link", CGI_STRING);
		else
			cgi_table_add_value(t, "backup_method", (char *) "backup_method-ping", CGI_STRING);

		/* Default route */
		cgi_table_add_value(t, "gw", (void *) (ppp_cfg.bckp_conf.is_default_gateway), CGI_INTEGER);
		cgi_table_add_value(t, "gwmetric", (void *) (ppp_cfg.bckp_conf.default_route_distance), CGI_INTEGER);

		cgi_table_add_value(t, "up", (void *) (conf.up || (!ppp_cfg.bckp_conf.shutdown)), CGI_INTEGER);
		cgi_table_add_value(t, "ping_addr",(char *) ppp_cfg.bckp_conf.ping_address, CGI_STRING);

		/* Interface Status purpose */
		cgi_table_add_value(t, "ipaddr", (char *) conf.main_ip.ipaddr, CGI_STRING);
		cgi_table_add_value(t, "ipmask", (char *) conf.main_ip.ipmask, CGI_STRING);
		cgi_table_add_value(t, "ippeer", (char *) conf.main_ip.ippeer, CGI_STRING);

	}

	return t;
}
#endif /* OPTION_MODEM3G */

/**
 * handle_config_interface	Main handler for interface configuration
 *
 * @param req
 * @param resp
 * @return 0 if success, -1 if failure
 */
int handle_config_interface(struct request *req, struct response *resp)
{
	cgi_table *eth_table, *lo_table;
#ifdef OPTION_MODEM3G
	cgi_table *m3g_table;
#endif

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	web_dbg("Fetching ethernet info...\n");
	eth_table = _fetch_eth_info();
	if (eth_table == NULL) {
		log_err("Failed to fetch ethernet information\n");
		return -1;
	}

	web_dbg("Fetching loopback info...\n");
	lo_table = _fetch_lo_info();
	if (lo_table == NULL) {
		log_err("Failed to fetch loopback information\n");
		return -1;
	}

#ifdef OPTION_MODEM3G
	web_dbg("Fetching 3G info...\n");
	m3g_table = _fetch_3g_info();
	if (m3g_table == NULL) {
		log_err("Failed to fetch 3G information\n");
		return -1;
	}
#endif

	web_dbg("Filling tables...\n");
	cgi_response_add_parameter(resp, "menu_config", (void *) 1, CGI_INTEGER);
	cgi_response_add_parameter(resp, "eth_table", (void *) eth_table, CGI_TABLE);
	cgi_response_add_parameter(resp, "lo_table", (void *) lo_table, CGI_TABLE);
#ifdef OPTION_MODEM3G
	cgi_response_add_parameter(resp, "m3g_table", (void *) m3g_table, CGI_TABLE);
#endif
	cgi_response_add_parameter(resp, "ethernet_iface_num", (void *) ETHERNET_IFACE_NUM, CGI_INTEGER);
	cgi_response_set_html(resp, "/wn/cgi/templates/config_interfaces.html");
	web_dbg("done...\n");
	return 0;
}

int handle_config_interface_status(struct request *req, struct response *resp)
{
	cgi_table *eth_table, *lo_table;
#ifdef OPTION_MODEM3G
	cgi_table *m3g_table;
#endif
	char uptime_buf[256];

	if (!cgi_session_exists(req)) {
		cgi_response_set_html(resp, "/wn/cgi/templates/do_login.html");
		return 0;
	}

	eth_table = _fetch_eth_info();
	if (eth_table == NULL) {
		log_err("Failed to fetch ethernet information\n");
		return -1;
	}

	lo_table = _fetch_lo_info();
	if (lo_table == NULL) {
		log_err("Failed to fetch loopback information\n");
		return -1;
	}

#ifdef OPTION_MODEM3G
	m3g_table = _fetch_3g_info();
	if (m3g_table == NULL) {
		log_err("Failed to fetch 3G information\n");
		return -1;
	}
#endif

	if (librouter_time_get_uptime(uptime_buf) < 0){
		log_err("Failed to fetch Up Time information\n");
		return -1;
	}

	web_dbg("Filling tables for status...\n");
	cgi_response_add_parameter(resp, "menu_status", (void *) 1, CGI_INTEGER);
	cgi_response_add_parameter(resp, "eth_table", (void *) eth_table, CGI_TABLE);
	cgi_response_add_parameter(resp, "lo_table", (void *) lo_table, CGI_TABLE);
#ifdef OPTION_MODEM3G
	cgi_response_add_parameter(resp, "m3g_table", (void *) m3g_table, CGI_TABLE);
#endif
	cgi_response_add_parameter(resp, "uptime", (void *) uptime_buf, CGI_STRING);
	cgi_response_set_html(resp, "/wn/cgi/templates/status_interfaces.html");

	return 0;
}
