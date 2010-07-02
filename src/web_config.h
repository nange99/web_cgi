/*
 * web_config.h
 *
 *  Created on: Jun 15, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef WEB_CONFIG_H_
#define WEB_CONFIG_H_

/* Needed for librouter includes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>


/* LibCGI includes*/
#include <libcgiservlet/cgi_servlet.h>
#include <libcgiservlet/cgi_session.h>
#include <libcgiservlet/cgi_table.h>
#include <libcgiservlet/cgi_list.h>

/* Libconfig includes */
#include <librouter/options.h>
#include <librouter/acl.h>
#include <librouter/args.h>
#include <librouter/cish_defines.h>
#include <librouter/config_fetcher.h>
#include <librouter/device.h>
#include <librouter/typedefs.h>
#include <librouter/ip.h>
#include <librouter/dev.h>
#include <librouter/dhcp.h>
#include <librouter/dns.h>
#include <librouter/lan.h>
#include <librouter/ppp.h>
#include <librouter/str.h>
#include <librouter/libtime.h>
#include <librouter/flashsave.h>
#include <librouter/mangle.h>
#include <librouter/nat.h>
#include <librouter/ntp.h>
#include <librouter/nv.h>
#include <librouter/pam.h>
#include <librouter/pim.h>
#include <librouter/defines.h>
#include <librouter/version.h>
#include <librouter/debug.h>
#include <librouter/qos.h>
#include <librouter/ipsec.h>
#include <librouter/exec.h>
#include <librouter/process.h>
#include <librouter/quagga.h>
#include <librouter/snmp.h>
#include <librouter/ppcio.h>
#include <librouter/md5.h>
#include <librouter/sha1.h>

#ifdef OPTION_MODEM3G
#include <librouter/modem3G.h>
#endif
#ifdef OPTION_SMCROUTE
#include <librouter/smcroute.h>
#endif
#include <librouter/tunnel.h>
#ifdef OPTION_VRRP
#include <librouter/vrrp.h>
#endif
#include <librouter/ssh.h>
#include <librouter/vlan.h>

#define DEBUG
#ifdef DEBUG
#define web_dbg(x,...) \
		syslog(LOG_INFO, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define web_dbg(x,...)
#endif

#define log_err(x,...) \
	syslog(LOG_ERR, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif /* WEB_CONFIG_H_ */
