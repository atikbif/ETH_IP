/*
 * http_server.c
 *
 *  Created on: 28 мая 2019 г.
 *      Author: User
 */


#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "main.h"
#include "lwip/apps/fs.h"
#include <string.h>

static const unsigned char PAGE_HEADER_200_OK[] = {
  //"HTTP/1.1 200 OK"
  0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
  0x0a,
  //zero
  0x00
};

static char PAGE_BODY[768];
uint16_t len = 0;

static const unsigned char PAGE_HEADER_SERVER[] = {
  //"Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)"
  0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x31,0x2e,0x33,
  0x2e,0x31,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,0x6e,
  0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,0x70,
  0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,
  //zero
  0x00
};
static const unsigned char PAGE_HEADER_CONTENT_TEXT[] = {
  //"Content-type: text/html"
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3a,0x20,0x39,0x0d,0x0a,
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x74,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
  0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
  //zero
  0x00
};

#define HTTPSERVER_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

typedef struct struct_client_socket_t {
  struct sockaddr_in remotehost;
  socklen_t sockaddrsize;
  int accept_sock;
  uint16_t y_pos;
} struct_client_socket;
struct_client_socket client_socket01;

static void http_server_thread(void *arg)
{
	struct fs_file file;
	struct netconn *conn, *newconn;
	  err_t err, accept_err;
	  struct netbuf *buf;
	  void *data;
	  u16_t len;

	  LWIP_UNUSED_ARG(arg);

	  conn = netconn_new(NETCONN_TCP);

	  if (conn!=NULL)
	  {
	    err = netconn_bind(conn, NULL, 80);

	    if (err == ERR_OK)
	    {
	      netconn_listen(conn);

	      while (1)
	      {
	         accept_err = netconn_accept(conn, &newconn);

	        if (accept_err == ERR_OK)
	        {
	          while (netconn_recv(newconn, &buf) == ERR_OK)
	          {
	            do
	            {
	            	netbuf_data(buf, &data, &len);

	              	if ((len >=5) && (strncmp((char const *)data, "GET /", 5) == 0))
					{
						if ((strncmp((char const *)data,"GET / ",6)==0)||(strncmp((char const *)data,"GET /index.html",15)==0))
						{
							fs_open(&file, "/index.html");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /index1.html",16)==0)
						{
							fs_open(&file, "/index1.html");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /favicon.ico",16)==0)
						{
							fs_open(&file, "/favicon.ico");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /IMG/img01.jpg",18)==0)
						{
							fs_open(&file, "/IMG/img01.jpg");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /IMG/img02.jpg",18)==0)
						{
							fs_open(&file, "/IMG/img02.jpg");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /IMG/img03.jpg",18)==0)
						{
							fs_open(&file, "/IMG/img03.jpg");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
						else if (strncmp((char const *)data,"GET /IMG/img04.jpg",18)==0)
						{
							fs_open(&file, "/IMG/img04.jpg");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}else if(strncmp((char const *)data,"GET /write.html?node=",21)==0) {
							sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
							len = strlen(PAGE_BODY);
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '2';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '3';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							netconn_write(newconn, PAGE_BODY, len, NETCONN_COPY);
						}else if(strncmp((char const *)data,"GET /reset.html",15)==0) {
							sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
							len = strlen(PAGE_BODY);
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '2';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '3';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							netconn_write(newconn, PAGE_BODY, len, NETCONN_COPY);
						}else if(strncmp((char const *)data,"GET /read.html",14)==0) {
							sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
							len = strlen(PAGE_BODY);
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '1';
							PAGE_BODY[len++] = '2';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '3';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							PAGE_BODY[len++] = '0';
							netconn_write(newconn, PAGE_BODY, len, NETCONN_COPY);
						}
						else
						{
							fs_open(&file, "/404.html");
							netconn_write(newconn, file.data, file.len, NETCONN_COPY);
							fs_close(&file);
						}
					}
	            }
	            while (netbuf_next(buf) >= 0);
	            netbuf_delete(buf);
	          }


	          netconn_close(newconn);
	          netconn_delete(newconn);
	        }
	      }
	    }
	    else
	    {
	      netconn_delete(newconn);
	    }
	  }
}

void http_server_init(void)
{
  sys_thread_new("http_thread", http_server_thread, NULL, DEFAULT_THREAD_STACKSIZE*2, HTTPSERVER_THREAD_PRIO);
}
