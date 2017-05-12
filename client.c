#include "common.h"

int main(int argc, char** argv)
{
	struct sockaddr_in server_addr;
	
	bzero(&server_addr, sizeof(server_addr));
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(1234);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0)
	{
		perror("socket create error\n");
		return -1;
	}

	if(connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("error\n");
		close(fd);
		return -1;
	}

	if(write(fd, argv[2], strlen(argv[2])) < 0)
	{
		perror("write error\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}
