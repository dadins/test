#include "common.h"
#include "worker.h"

int main(int argc, char** argv)
{
	struct sockaddr_in server_addr;
	while(1)
	{	
		bzero(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(1234);
		
		int fd = socket(AF_INET, SOCK_STREAM,  0);
		
		if(fd < 0)
		{
			perror("socket create error\n");
			continue;
		}
		
		if(bind(fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0)
		{
			perror("socket bind error\n");
			continue;
		}

		listen(fd, 5);
		
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		char buf[256];
		while(1)
		{
			int new_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
			bzero(buf,256);
			if(read(new_fd, buf, 255) < 0)
			{
				perror("error reading from socket");
				close(new_fd);
			}
			pthread_t task_thread;
			pthread_create(&task_thread, NULL, task_process, buf);
		}
	}
	return 0;
}
