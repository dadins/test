#include "common.h"

#include "conf.h"

int get_conf(char* name, conf_t* conf)
{
	FILE* fp = fopen(name, "r");
	if(NULL == fp)
	{
		printf("%s error\n", __func__);
		return -1;
	}

	size_t	 size = 128;
	uint8_t* buf = (uint8_t*)malloc(size);
	while(-1 != getline((char**)&buf, &size, fp))
	{
		char* tmp = NULL;
		int len = 0;
		if(NULL != (tmp = strstr(buf, "input=")))
		{
			len = strlen("input=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->input, tmp+len, strlen(tmp+len)-1);
		}
		else if(NULL != (tmp = strstr(buf, "output=")))
		{
			len = strlen("output=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->output, tmp+len, strlen(tmp+len)-1);
		}
		else if(NULL != (tmp = strstr(buf, "cmd=")))
		{
			len = strlen("cmd=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->cmd, tmp+len, strlen(tmp+len)-1);
		}
		else if(NULL != (tmp = strstr(buf, "max_open=")))
		{
			len = strlen("max_open=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			conf->max_open = atoi(tmp+len);
		}
		else if(NULL != (tmp = strstr(buf, "shared_lib=")))
		{
			len = strlen("shared_lib=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->shared_lib, tmp+len, strlen(tmp+len)-1);
		}
		else if(NULL != (tmp = strstr(buf, "func=")))
		{
			len = strlen("func=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->func, tmp+len, strlen(tmp+len)-1);
		}
		else if(NULL != (tmp = strstr(buf, "script=")))
		{
			len = strlen("script=");
			while(tmp[len] == ' ')
			{
				len++;
			}
			strncpy(conf->script, tmp+len, strlen(tmp+len)-1);
		}
		else
		{
			continue;
		}
		bzero(buf, size);
	}

	fclose(fp);
}
