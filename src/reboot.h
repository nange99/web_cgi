/*
 * reboot.h
 *
 *  Created on: Jun 14, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef REBOOT_H_
#define REBOOT_H_

int handle_reboot(struct request *req, struct response *resp);
int handle_reboot_refuse(struct request *req, struct response *resp);
int handle_reboot_confirm(struct request *req, struct response *resp);

#endif /* REBOOT_H_ */
