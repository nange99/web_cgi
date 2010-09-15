/*
 * firewall.h
 *
 *  Created on: Jul 19, 2010
 *      Author: Igor Kramer Pinotti (ipinotti@pd3.com.br)
 */

#ifndef FIREWALL_H_
#define FIREWALL_H_

int handle_apply_fw_general_settings(struct request *req, struct response *resp);
int handle_config_firewall(struct request *req, struct response *resp);
int handle_fw_add_rule(struct request *req, struct response *resp);
int handle_fw_del_rule (struct request *req, struct response *resp);
int handle_fw_view_rule(struct request *req, struct response *resp);

#endif /* FIREWALL_H_ */
