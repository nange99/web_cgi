/*
 * cpu.h
 *
 *  Created on: Jul 20, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef CPU_H_
#define CPU_H_

int handle_status_cpu(struct request *req, struct response *resp);
int handle_status_memory(struct request *req, struct response *resp);

#endif /* CPU_H_ */
