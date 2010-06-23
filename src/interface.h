/*
 * interface.h
 *
 *  Created on: Jun 22, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

int show_interface_stats (cgi_table *t, char *name);
int handle_config_interface(struct request *req, struct response *resp);

#endif /* INTERFACE_H_ */
