#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
	FILE *f = NULL;
	f = fopen("/dev/memory", "w");
	if(NULL == f)
	{
		printf("打开设备失败!\r\n");
		return -1;
	}else
		printf("打开设备成功\r\n");
	fclose(f);
	
	return 0;

}
