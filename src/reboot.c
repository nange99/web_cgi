/*
 * reboot.c
 *
 *  Created on: Jun 14, 2010
 *      Author: Thomás Alimena Del Grande (tgrande@pd3.com.br)
 */

int handle_reboot(struct request *req, struct response *resp)
{
	cgi_response_set_html(resp, "/wn/cgi/templates/confirm_reboot.html");

	return 0;
}

int handle_reboot_refuse(struct request *req, struct response *resp)
{
	cgi_response_set_html(resp, "/wn/cgi/templates/do_home.html");

	return 0; /* Not reached */
}

int handle_reboot_confirm(struct request *req, struct response *resp)
{
	sync();
	reboot(0x01234567);
	return 0; /* Not reached */
}
