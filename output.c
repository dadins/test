#include "common.h"

#include "output.h"

pf_output default_output_func = output;

/*
 * 功能：输出采集到的ps信息;
 */
void output(connection_t* conn, conf_t* conf)
{
	char* buf = conn->buf;
	
	int fd1 = open(conf->output, O_RDWR|O_CREAT|O_APPEND);
	
	if(NULL != strstr(buf, "<HTML>"))
	{
		close(fd1);
		return;
	}

	write(fd1, buf, strlen(buf)+1);
	
	close(fd1);
}

void sample_download(conf_t* conf)
{
	//format a strings to specified the sample name
	time_t t = time(NULL);
	
	char sample_dir[128] = {0};
	
	strftime(sample_dir, 128, "sample-%Y%m%d-%H%M%S", localtime(&t));

	char cmd[256] = {0};

	sprintf(cmd, "%s %s %s", conf->script, conf->output, sample_dir);

	printf("cmd=%s\n", cmd);

	system(cmd);
}
