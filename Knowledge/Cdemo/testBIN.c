#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	
	int num, num_b;
	scanf("%d",&num);
	printf("num-%d\r\n",num);
	num_b = num;

	int i, j, k;
	char r[33];
	memset(r,'\0',33);
	for(i = 32; i >=0;)
	{
		j = num % 2;
		k = num / 2;
		num = k;
		if(j == 0)
			r[--i] = '0';
		else	
			r[--i] = '1';
	}
	printf("二进制:%s\r\n",r);
	
	
	int count=0;
	while(num_b)
	{
       		 num_b = num_b & (num_b-1);
       		 count++;
	}
	printf("二进制中1的个数:%d\r\n",count);
	return 0;
}
