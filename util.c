#include <sys/resource.h>
#include "common.h"
void resource_limit()
{
	struct rlimit limit;
	getrlimit(RLIMIT_NOFILE, &limit);
	printf("the soft limit is: %zu\n", limit.rlim_cur);
	printf("the hard limit is: %zu\n", limit.rlim_max);
	limit.rlim_cur = 64*10240;
	limit.rlim_max = 64*10240;
	setrlimit(RLIMIT_NOFILE, &limit);
}

int parse_ip_and_port(char* line, uint32_t* ip, uint16_t* port)
{
	char* ptr = NULL;
	/* strtok()函数是不可重入，也是线程不安全的，因为内部采用了一个static变量存储分割后的字符指针位置;
	 * strtok_r() 是可重入和线程安全的 
	 * */
	char* scheme = NULL;
	char* p_ip = NULL;
	char* p_port = NULL;
	
	char* tmp = NULL;
	
	//如果有"http://"前缀,要过滤掉该前缀
	if(tmp = strstr(line, "http://"))
	{
		line = tmp + 7; 
	}

	p_ip = strtok_r(line, ":", &ptr);
	
	if(NULL == p_ip)
	{
		return -1;
	}

	struct in_addr addr;
	inet_aton(p_ip, &addr);
	
	*ip = addr.s_addr;
	p_port = strtok_r(NULL, ":", &ptr);
	
	//无端口的默认是80端口
	if(NULL == p_port)
	{
		*port = 80;
	}
	else
	{
		*port = atoi(p_port);
	}
	return 0;
}
