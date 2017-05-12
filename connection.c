#include "common.h"

#include "output.h"

int create_and_bind()
{
	//1. make socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
	{
		perror("create error\n");
		return -1;
	}

#if 0
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);
	
	//2. bind socket
	if(bind(fd, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		close(fd);
		perror("bind error\n");
		return -1;
	}
#endif

	//3. set socket in noblocking mode
	if(fcntl(fd, F_SETFL,  fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
	{
		close(fd);
		perror("block error\n");
		return -1;
	}
	return fd;
}

int connect_server(int fd, uint32_t ip, uint16_t port)
{
	//1. construct the server
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ip;
	server_addr.sin_port = htons(port);
	
	//2. connect the server
	int ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));

	// 如果是EINPROGRESS,则代表连接正在建立
	if(ret == -1 && errno != EINPROGRESS) 
	{
		perror("connect error\n");
		return -1;
	}

	else
	{
		return 0;
	}
}

// fd 如果超过conn的最大值会出现溢出的问题, 所以还不能直接引用conn[fd],需要修改
int connection_open(int fd, uint32_t ip, uint16_t port, void* arg)
{
	if(0 == connect_server(fd, ip, port))
	{
		worker_t* worker = (worker_t*)arg;
		pthread_mutex_lock(&worker->lock);
		worker->conn[fd].ip			= ip;
		worker->conn[fd].port		= port;
		worker->conn[fd].open		= 1;
		worker->conn[fd].first_time = time(NULL);
		worker->conn[fd].last_time	= time(NULL);
		worker->conn[fd].last_read	= 0;
		worker->cur_open++;
		pthread_mutex_unlock(&worker->lock);
		return 0;
	}
	else
	{
		return -1;
	}
}


//连接关闭后输出接收到的数据
void connection_close(int fd, void* arg)
{
	worker_t* worker = (worker_t*)arg;
	
	pthread_mutex_lock(&(worker->lock));
	worker->cur_open--;
	epoll_ctl(worker->efd, EPOLL_CTL_DEL, fd, 0);
	worker->conn[fd].open = 0;
	if(worker->conn[fd].last_read > 0)
	{
		worker->output_func(worker->conn + fd, worker->conf);
	}
	worker->conn[fd].last_read = 0;
	bzero(worker->conn[fd].buf, MAX_BUF_SIZE);
	worker->conn[fd].ip	= 0;
	worker->conn[fd].port = 0;
	worker->conn[fd].last_time = 0;
	worker->conn[fd].first_time = 0;
	close(fd);
	pthread_mutex_unlock(&(worker->lock));
}

void concurrent_conn_ctrl(void* arg, int max_open)
{
	worker_t* worker = (worker_t*)arg;
	int i = 0;
	int timeout = 10;
	int max_timeout = 30; //某些连接长时间占用，发送无用的数据

	/* 当前连接数过多的话,等待一段时间再建立新的连接 */
	while(worker->cur_open > max_open) 
	{
		printf("cur_open=%d\n", worker->cur_open);
		for(i = 0; i < MAX_CONN; i++)
		{	
			if(worker->conn[i].open == 1)
			{
				int cur_time = time(NULL);
				if(worker->conn[i].last_time + timeout < cur_time || worker->conn[i].first_time + max_timeout < cur_time)
				{
					connection_close(i, worker);
				}
			}
		}
		sleep(1);
	}
}
