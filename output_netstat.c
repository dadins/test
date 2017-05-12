#include "common.h"

/*
 * 功能：输出采集到的netstat信息,提取关键信息
 */
void output(connection_t* conn, conf_t* conf)
{
	char* buf = conn->buf;

	int fd = open(conf->output, O_RDWR|O_CREAT|O_APPEND);
	
	while(buf)
	{
		//过滤查找特定的字符串:127.0.0.1
		char* beg = strstr(buf, "127.0.0.1");
		
		if(NULL != beg)
		{
			int i = 0;

			for(i = 0;  ; i++)
			{
				//换行或文件结束
				if(beg[i] == '\n' || beg[i] == '\0')
				{
					break;
				}
			}

			beg[i] = '\0';

			//查找另一个字符串:LISTEN
			if(strstr(beg, "LISTEN"))
			{
				write(fd, beg, strlen(beg));
				write(fd, "\n", 1);
			}

			buf = beg + i + 1;
		}

		else
		{
			break;
		}

	}

	close(fd);
}
