/*
 * nat.h
 *
 *  Created on: Jul 19, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef NAT_H_
#define NAT_H_

int handle_apply_nat_general_settings(struct request *req, struct response *resp);
int handle_apply_nat_rules_settings(struct request *req, struct response *resp);
int handle_config_nat(struct request *req, struct response *resp);

#endif /* NAT_H_ */
