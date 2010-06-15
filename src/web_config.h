/*
 * web_config.h
 *
 *  Created on: Jun 15, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef WEB_CONFIG_H_
#define WEB_CONFIG_H_

#define DEBUG
#ifdef DEBUG
#define web_dbg(x,...) \
		syslog(LOG_INFO, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define web_dbg(x,...)
#endif

#endif /* WEB_CONFIG_H_ */
