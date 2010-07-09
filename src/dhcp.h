/*
 * dhcp.h
 *
 *  Created on: Jul 8, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef DHCP_WEB_H_
#define DHCP_WEB_H_

int handle_apply_dhcpd_settings(struct request *req, struct response *resp);
int handle_config_dhcpd(struct request *req, struct response *resp);

#endif /* DHCP_WEB_H_ */
