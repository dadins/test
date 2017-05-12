#include <dlfcn.h>
#include "common.h"
#include "util.h"
#include "conf.h"
#include "connection.h"
#include "output.h"

void handle_event(int efd, struct epoll_event* ev, worker_t* worker)
{
	if(ev->events & EPOLLERR || ev->events & EPOLLHUP || ev->events & EPOLLRDHUP)
	{
		return;
	}

	int fd = ev->data.fd;
	
	struct sockaddr_in peer;
	
	socklen_t len = sizeof(struct sockaddr);
	
	int ret = getpeername(fd, (struct sockaddr*)&peer, &len);
	
	char dst[16] = {0};
	
	inet_ntop(AF_INET, &(peer.sin_addr), dst, len);
	
	char* request = malloc(1024);
	
	bzero(request, 1024);
	
	if(worker->conn[fd].port != 80)
	{
		sprintf(request, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36\r\n\r\n", worker->conf->cmd, dst, worker->conn[fd].port);
	}
	else
	{
		sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.71 Safari/537.36\r\n\r\n", worker->conf->cmd, dst);
	}

	if(ev->events & EPOLLOUT)
	{
		send(fd, request, strlen(request), MSG_NOSIGNAL);
		ev->data.fd = fd;
		ev->events = EPOLLIN | EPOLLET;
		epoll_ctl(efd, EPOLL_CTL_MOD, fd, ev);
	}

	free(request);

	if(ev->events & EPOLLIN)
	{
		char	buf[10240];
		int		nread = worker->conn[fd].last_read;
		int		n = 0;
		while((n = read(fd, buf, 10240)) > 0)
		{
			if(n == -1)
			{
				if(errno != EAGAIN) //errno = EAGAIN mean we have read all of data
				{
					perror("read error");
				}
				connection_close(fd, worker);
				break;
			}
			
			//读取的字节数超过了最大容量,则停止读取数据,否则会出现缓冲区溢出
			if(nread + n > MAX_BUF_SIZE)
			{
				connection_close(fd, worker);
				break;
			}
			
			//将读取的数据copy到对应的buf中
			else
			{
				memcpy(worker->conn[fd].buf + nread, buf, n);
				nread += n;
				worker->conn[fd].last_read = nread;
				worker->conn[fd].last_time = time(NULL);
			}
		}
		
		//对方关闭了连接
		if(n == 0)
		{
			connection_close(fd, worker);
		}
	}
}

worker_t* worker_init(void* args)
{
	worker_t* worker = (worker_t*)malloc(sizeof(worker_t));
	
	if(worker)
	{
		bzero(worker, sizeof(worker_t));
	}
	else
	{
		printf("worker init error\n");
		return NULL;
	}

	worker->conn = (connection_t*)malloc(MAX_CONN * sizeof(connection_t));
	
	if(worker->conn)
	{
		bzero(worker->conn, MAX_CONN * sizeof(connection_t));
	}
	else
	{
		printf("conn init error\n");
		return NULL;
	}
	
	worker->conf = (conf_t*)malloc(sizeof(conf_t));
	
	if(worker->conf)
	{
		bzero(worker->conf, sizeof(conf_t));
	}
	else
	{
		return NULL;
	}
	
	get_conf((char*)args, worker->conf);
	
	worker->dlp = NULL;
	char* error = NULL;
	
	/* 插件化处理: 采用动态加载库的方式,
	 * 1. 读取配置信息,通过dlopen加载动态库 
	 * 2. 调用dlsym获取要执行函数的地址
	 */
	worker->dlp = dlopen(worker->conf->shared_lib, RTLD_LAZY);
	//printf("%s: %p\n", worker->conf->shared_lib, worker->dlp);
	if(!(worker->dlp))
	{
		printf("dlopen error: %s\n", (char*)dlerror());
	}
	else
	{
		if(NULL == (worker->output_func = dlsym(worker->dlp, worker->conf->func)))
		{
			printf("dlopen error: %s\n", (char*)dlerror());
			
			/* use defaut output process */
			worker->output_func = default_output_func;
		}
	}

	return worker;
}

void* worker_process(void* arg)
{
	worker_t* worker = (worker_t*)arg;
	
	while(1)
	{
		struct epoll_event events[128];

		int n = epoll_wait(worker->efd, events, 127, -1);
		
		if(n == -1)
		{
			perror("epoll_wait\n");
		}

		int i = 0;

		for(i = 0; i < n; i++)
		{
			handle_event(worker->efd, &events[i], worker);
		}
	}
}

void worker_fini(worker_t* worker)
{
	if(worker->conf)
	{
		free(worker->conf);
	}
	worker->conf = NULL;
	if(worker->conn)
	{
		free(worker->conn);
	}

	worker->conn = NULL;
	
	if(worker->dlp)
	{
		printf("call dlclose()\n");

		if(0 != dlclose(worker->dlp))
		{
			printf("dlclose error:%s\n", (char*)dlerror());
		}
	}

	if(worker)
	{
		free(worker);
	}
	worker = NULL;
}

void* task_process(void* args)
{
	/* 资源控制：控制连接数,防止受连接数的影响 */
	resource_limit();
	
	worker_t* worker = worker_init(args);
	
	if(!worker)
	{
		return NULL;
	}

	FILE* fp = fopen(worker->conf->input, "r");
	
	if(NULL == fp)
	{
		printf("%s\n", (char*)strerror(errno));
	}
	else
	{
		worker->efd = epoll_create1(0);
		pthread_create(&(worker->worker_thread), NULL,	worker_process, (void*)worker);
		char* line = (char*)malloc(100);
		bzero(line, 100);
		while(NULL != fgets(line, 100, fp))
		{
			uint32_t ip;
			uint16_t port;
			int fd;
			
			if(-1 == parse_ip_and_port(line, &ip, &port))
			{
				continue;
			}
			
			while(-1 == (fd = create_and_bind()))
			{
				sleep(1);
			}

			/* 与对端建立连接,connect之后将socket加入epoll */
			if(0 == connection_open(fd, ip, port, (void*)worker))
			{
				struct epoll_event ev;
				ev.data.fd = fd;
				ev.events = EPOLLOUT;
				epoll_ctl(worker->efd, EPOLL_CTL_ADD, fd, &ev);
			}
			else
			{
				if(0 != close(fd))
				{
					perror("close fd error\n");
				}
			}
			concurrent_conn_ctrl(worker, worker->conf->max_open);
			bzero(line, 100);
		}

		concurrent_conn_ctrl(worker, 0);

		if(0 != close(worker->efd))
		{
			perror("close epoll fd error\n");
		}

		if(0 != fclose(fp))
		{
			printf("fclose errno=%d, %s\n", errno, (char*)strerror(errno));
		}
		free(line);
		//todo: 退出process线程
	}

	/* before finish the worker, we need to do some process */
	
	sample_download(worker->conf);

	worker_fini(worker);
}
