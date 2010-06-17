/*
 * firmware.h
 *
 *  Created on: Jun 17, 2010
 *      Author: Pedro Kiefer (pkiefer@pd3.com.br)
 */

#ifndef FIRMWARE_H_
#define FIRMWARE_H_

int handle_firmware_version(struct request *req, struct response *resp);
int handle_firmware_upgrade(struct request *req, struct response *resp);
int handle_firmware_receive_file(struct request *req, struct response *resp);

#endif /* FIRMWARE_H_ */
