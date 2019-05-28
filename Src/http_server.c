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

#define HTTPSERVER_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )

typedef struct struct_client_socket_t {
  struct sockaddr_in remotehost;
  socklen_t sockaddrsize;
  int accept_sock;
  uint16_t y_pos;
} struct_client_socket;
struct_client_socket client_socket01;

static unsigned char rx_buf[512];

static void client_socket_thread(void *arg)
{
	int ret, accept_sock;
	struct sockaddr_in remotehost;
	socklen_t sockaddrsize;
	struct_client_socket *arg_client_socket;
	arg_client_socket = (struct_client_socket*) arg;
	remotehost = arg_client_socket->remotehost;
	sockaddrsize  = arg_client_socket->sockaddrsize;
	accept_sock = arg_client_socket->accept_sock;
	for(;;)
	{
		ret = recvfrom( accept_sock,rx_buf, 512, 0, (struct sockaddr *)&remotehost, &sockaddrsize);
		if(ret > 0)
		{
			sendto(accept_sock,&rx_buf[0],ret,0,(struct sockaddr *)&remotehost, sockaddrsize);
		}else {
			close(accept_sock);
			osThreadTerminate(NULL);
		}
	}
}

static void http_server_thread(void *arg)
{

	int sock, accept_sock;
	struct sockaddr_in address, remotehost;
	socklen_t sockaddrsize;
	if ((sock = socket(AF_INET,SOCK_STREAM, 0)) >= 0)
	{
		address.sin_family = AF_INET;
		address.sin_port = htons(1024);
		address.sin_addr.s_addr = INADDR_ANY;
		if (bind(sock, (struct sockaddr *)&address, sizeof (address)) ==  0)
		{
			listen(sock, 5);
			for(;;)
			{
				accept_sock = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&sockaddrsize);
				if(accept_sock >= 0)
				{
					client_socket01.accept_sock = accept_sock;
					client_socket01.remotehost = remotehost;
					client_socket01.sockaddrsize = sockaddrsize;
					sys_thread_new("client_socket_thread", client_socket_thread, (void*)&client_socket01, DEFAULT_THREAD_STACKSIZE, osPriorityNormal );
				}
			}
		}else
	    {
	      close(sock);
	      return;
	    }
	}
}

void http_server_init(void)
{
  sys_thread_new("http_thread", http_server_thread, NULL, DEFAULT_THREAD_STACKSIZE, HTTPSERVER_THREAD_PRIO);
}
