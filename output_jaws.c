#include "common.h"

/*
 * 功能：输出server类型为jaws的IP
 */
void output(connection_t* conn, conf_t* conf)
{
	char* buf = conn->buf;
	int fd = open(conf->output, O_RDWR|O_CREAT|O_APPEND);
	
	if(fd < 0)
	{
		printf("%s, error\n", __func__);
		return;
	}

	/*过滤查找特定的字符串*/
	if(NULL != strstr(buf, "JAWS"))
	{
		struct in_addr addr; 
		addr.s_addr = conn->ip;
		char buf[32] = {0};
		sprintf(buf, "%s:%d\n", inet_ntoa(addr), conn->port);
		write(fd, buf, strlen(buf));
	}
	close(fd);
}
