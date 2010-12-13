/*
 * web_config.h
 *
 *  Created on: Jun 15, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

#ifndef WEB_CONFIG_H_
#define WEB_CONFIG_H_

#define CGI_MAX_PARAM_LEN	64

//#define DEBUG
#ifdef DEBUG
#define web_dbg(x,...) \
		syslog(LOG_INFO, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define web_dbg(x,...)
#endif

#define log_err(x,...) \
	syslog(LOG_ERR, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);

char *_get_parameter(struct request *req, char *param);

#endif /* WEB_CONFIG_H_ */
