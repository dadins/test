#include "common.h"

/*
 * 功能：输出采集到的ls信息,提取关键信息
 */
void output(connection_t* conn, conf_t* conf)
{
	char* buf = conn->buf;

	int fd = open(conf->output, O_RDWR|O_CREAT|O_APPEND);
	
	//1.过滤掉HTML等字符;
	//2.记录设备IP PORT 文件MD5值等,统计;
	//3.

	write(fd, buf, strlen(buf));

	close(fd);
}
