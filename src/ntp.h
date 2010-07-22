/*
 * ntp.h
 *
 *  Created on: Jul 19, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef NTP_H_
#define NTP_H_

int handle_apply_date_settings(struct request *req, struct response *resp);
int handle_apply_timezone_settings(struct request *req, struct response *resp);
int handle_config_ntp(struct request *req, struct response *resp);

#endif /* NTP_H_ */
