/*
 * interface.h
 *
 *  Created on: Jun 22, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#define ETHERNET_IFACE_NUM	2
#define LOOPBACK_IFACE_NUM	1
#define M3G_IFACE_NUM		3

int handle_apply_intf_settings(struct request *req, struct response *resp);
int handle_config_interface(struct request *req, struct response *resp);

#endif /* INTERFACE_H_ */
