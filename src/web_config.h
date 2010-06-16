/*
 * web_config.h
 *
 *  Created on: Jun 15, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef WEB_CONFIG_H_
#define WEB_CONFIG_H_

/* Libconfig includes */
#include <libconfig/options.h>
#include <libconfig/acl.h>
#include <libconfig/args.h>
#include <libconfig/cish_defines.h>
#include <libconfig/config_fetcher.h>
#include <libconfig/device.h>
#include <libconfig/typedefs.h>
#include <libconfig/ip.h>
#include <libconfig/dev.h>
#include <libconfig/dhcp.h>
#include <libconfig/dns.h>
#include <libconfig/lan.h>
#include <libconfig/ppp.h>
#include <libconfig/str.h>
#include <libconfig/libtime.h>
#include <libconfig/flashsave.h>
#include <libconfig/mangle.h>
#include <libconfig/nat.h>
#include <libconfig/ntp.h>
#include <libconfig/nv.h>
#include <libconfig/pam.h>
#include <libconfig/pim.h>
#include <libconfig/defines.h>
#include <libconfig/version.h>
#include <libconfig/debug.h>
#include <libconfig/qos.h>
#include <libconfig/ipsec.h>
#include <libconfig/exec.h>
#include <libconfig/process.h>
#include <libconfig/quagga.h>
#include <libconfig/snmp.h>
#include <libconfig/ppcio.h>
#include <libconfig/md5.h>

#ifdef OPTION_MODEM3G
#include <libconfig/modem3G.h>
#endif
#ifdef OPTION_SMCROUTE
#include <libconfig/smcroute.h>
#endif
#include <libconfig/tunnel.h>
#ifdef OPTION_VRRP
#include <libconfig/vrrp.h>
#endif
#include <libconfig/ssh.h>
#include <libconfig/vlan.h>

#define DEBUG
#ifdef DEBUG
#define web_dbg(x,...) \
		syslog(LOG_INFO, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define web_dbg(x,...)
#endif

#endif /* WEB_CONFIG_H_ */
