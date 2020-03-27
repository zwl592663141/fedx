/*
Copyright (C), 1995-2005, Si-Tech Information Technology Ltd. 
file_name:  o2n_cmp_ck.c
author:     caoshun
version:    1.01
date:       2005-04-30
description:
			o2n_cmp ¸¨Öú³ÌÐò
others:
history:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUTO_DEBUG printf("debug,%s,%d\n", __FILE__, __LINE__);

#define APPLICATION_NAME "o2n_cmp_ck"
#define APPLICATION_VERSION "1.01"

void busi_version(void)
{
	printf("Copyrights(C) 1995-2005 SI-TECH CO.,LTD\n");
	printf("The iBOSS Billing %s. Version %s\n", APPLICATION_NAME, APPLICATION_VERSION);
	printf("Usage:%s <file>\n", APPLICATION_NAME);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char buffer1[4096+1];
	char buffer2[4096+1];
	char buffer3[4096+1];
	long  i=0;

	if(argc != 2)
	{
		busi_version();
		return -1;
	}

	fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		fprintf(stderr, "file error!\n");
		return -1;
	}

	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
	memset(buffer3, 0, sizeof(buffer3));
	fgets(buffer1, sizeof(buffer1), fp);
	fgets(buffer2, sizeof(buffer2), fp);
	fgets(buffer3, sizeof(buffer3), fp);
	
	while(!feof(fp))
	{
		i++;
	
		if(buffer1[0] != '-')
		{
			printf("loop=%ld,%ld\n", i, i*3);
			printf("check file error!\n");
			fclose(fp);
			return -1;
		}
	
	    	memset(buffer1, 0, sizeof(buffer1));
		memset(buffer2, 0, sizeof(buffer2));
		memset(buffer3, 0, sizeof(buffer3));
		fgets(buffer1, sizeof(buffer1), fp);
		fgets(buffer2, sizeof(buffer2), fp);
		fgets(buffer3, sizeof(buffer3), fp);
	}

	fclose(fp);

	printf("count reocrd=%ld,%ld\n", i, i*3);

	return 0;
}
