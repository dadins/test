#include "common.h"

/*
 * 功能：输出采集到的ps信息,提取关键信息
 */
void output(connection_t* conn, conf_t* conf)
{
	char* buf = conn->buf;

	int full_fd = open("output/ps/ps.txt", O_RDWR|O_CREAT|O_APPEND);
	
	write(full_fd, buf, strlen(buf));

	close(full_fd);

	int fd = open(conf->output, O_RDWR|O_CREAT|O_APPEND);

	while(buf)
	{
		/*过滤查找特定的字符串*/
		char* beg = strstr(buf, "wget");

		if(beg)
		{
			/*查找行尾的位置*/
			int i = 0;
			for(i = 0;  ; i++)
			{
				if(beg[i] == '\n' || beg[i] == '\0' || beg[i] == '&')
				{
					break;
				}
			}

			beg[i] = '\0';
			
			write(fd, beg, strlen(beg));
			write(fd, "\n", 1);

			buf = beg + i + 1;
		}
		else
		{
			break;
		}
	}

	close(fd);
}
