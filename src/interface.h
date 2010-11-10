/*
 * interface.h
 *
 *  Created on: Jun 22, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */
#include <librouter/options.h>

#ifndef INTERFACE_H_
#define INTERFACE_H_

#ifdef OPTION_NO_WAN
#define ETHERNET_IFACE_NUM	1
#else
#define ETHERNET_IFACE_NUM	2
#endif

#define LOOPBACK_IFACE_NUM	1

#ifdef OPTION_MODEM3G
#define M3G_IFACE_NUM		3
#else
#define M3G_IFACE_NUM		0
#endif

int handle_apply_intf_settings(struct request *req, struct response *resp);
int handle_config_interface(struct request *req, struct response *resp);
int handle_config_interface_status(struct request *req, struct response *resp);


#endif /* INTERFACE_H_ */
