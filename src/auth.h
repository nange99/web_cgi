/*
 * auth.h
 *
 *  Created on: Jul 26, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef AUTH_H_
#define AUTH_H_

int handle_add_user(struct request *req, struct response *resp);
int handle_apply_auth_type(struct request *req, struct response *resp);
int handle_apply_radius_settings(struct request *req, struct response *resp);
int handle_apply_tacacs_settings(struct request *req, struct response *resp);
int handle_config_auth(struct request *req, struct response *resp);

#endif /* AUTH_H_ */
