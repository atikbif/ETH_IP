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

#define HTTPSERVER_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

typedef struct struct_client_socket_t {
  struct sockaddr_in remotehost;
  socklen_t sockaddrsize;
  int accept_sock;
  uint16_t y_pos;
} struct_client_socket;
struct_client_socket client_socket01;

static unsigned char buf[512];

static void http_server_thread(void *arg)
{

	//int sock, accept_sock, ret;
	//struct sockaddr_in address, remotehost;
	//socklen_t sockaddrsize;
	struct fs_file file;
	/*if ((sock = socket(AF_INET,SOCK_STREAM, 0)) >= 0)
	{
		address.sin_family = AF_INET;
		address.sin_port = htons(80);
		address.sin_addr.s_addr = INADDR_ANY;
		if (bind(sock, (struct sockaddr *)&address, sizeof (address)) ==  0)
		{
			listen(sock, 8);
			for(;;)
			{
				accept_sock = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&sockaddrsize);
				if(accept_sock >= 0)
				{
					ret = recvfrom(accept_sock, buf, 512, 0, (struct sockaddr *)&remotehost, &sockaddrsize);
					if(ret > 0)
					{
						if ((ret >=5) && (strncmp((char const *)buf, "GET /", 5) == 0))
						{
							if ((strncmp((char const *)buf,"GET / ",6)==0)||(strncmp((char const *)buf,"GET /index.html",15)==0))
							{
								fs_open(&file, "/index.html");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /index1.html",16)==0)
							{
								fs_open(&file, "/index1.html");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /favicon.ico",16)==0)
							{
								fs_open(&file, "/favicon.ico");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /IMG/img01.jpg",18)==0)
							{
								fs_open(&file, "/IMG/img01.jpg");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /IMG/img02.jpg",18)==0)
							{
								fs_open(&file, "/IMG/img02.jpg");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /IMG/img03.jpg",18)==0)
							{
								fs_open(&file, "/IMG/img03.jpg");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else if (strncmp((char const *)buf,"GET /IMG/img04.jpg",18)==0)
							{
								fs_open(&file, "/IMG/img04.jpg");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
							else
							{
								fs_open(&file, "/404.html");
								write(accept_sock, (const unsigned char*)(file.data), (size_t)file.len);
								fs_close(&file);
							}
						}
					}
					close(accept_sock);
				}
			}
		}else
	    {
	      close(sock);
	      return;
	    }
	}*/
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
