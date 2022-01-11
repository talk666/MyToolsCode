#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "print.h"

int main()
{

	unsigned char test[] = {0x01,0x02,0x03,0x04};
	unsigned char *test2 = "1234";
	print_hex("testdate:\r\n", 0, test, sizeof(test));	
	print_hex("testdate2:\r\n", 0, test2, strlen(test2));	
	return 0;
}
