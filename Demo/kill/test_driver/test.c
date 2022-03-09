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
	
	char b = '1';
	fwrite(&b, 1, 1, f);//快地址 快大小 快个数 文件指针
	
	char *c;
	fread(c, 1, 1, f);
//	c = &b;	

	printf("out = %c\r\n",*c);

	fclose(f);
	
	return 0;

}
