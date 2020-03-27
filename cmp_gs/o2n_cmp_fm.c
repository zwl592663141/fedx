/*
Copyright (C), 1995-2005, Si-Tech Information Technology Ltd. 
file_name:  o2n_cmp_fm.c
author:     caoshun
version:    2.00
date:       2005-05-04
description:
			o2n_cmp ¸¨Öú³ÌÐò
others:
history:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUTO_DEBUG printf("debug,%s,%d\n", __FILE__, __LINE__);

#define APPLICATION_NAME "o2n_cmp_fm"
#define APPLICATION_VERSION "1.01"

struct stApp_Data
{
	int line_number;
	int opt_number;

	FILE *fp_in;
	FILE *fp_cmp;
	FILE *fp_nocmp;

	char cmp_filename[128+1];
	char nocmp_filename[128+1];
	char in_filename[128+1];	

	char line_buffer[10][4096+1];
};

struct stApp_Opt
{
	int line_no;
	int begin_pos;
	int seek_pos;
	int equ_opt;
	char content[1024+1];
	int line_no1;
	int begin_pos1;
	int seek_pos1;
};

struct stApp_Opt g_stAppOpt[10];
struct stApp_Data g_stAppData;

void busi_version(void)
{
	printf("Copyrights(C) 1995-2005 SI-TECH CO.,LTD\n");
	printf("The iBOSS Billing %s. Version %s\n", APPLICATION_NAME, APPLICATION_VERSION);
	printf("Usage:%s <in_file> <cmp_file> <nocmp_file> <line_number> <opt_strings...>\n", APPLICATION_NAME);
	printf("\n");
	printf("[opt_string]\n");
	printf("{1@30,5}=\"abcde\"\n");
	printf("{1@30,5}!=\"abcde\"\n");
	printf("{1@30,5}like\"abc\"\n");
	printf("{2@30,5}=\"abcde\"\n");
	printf("{2@30,5}={3@32,5}\n");
	printf("{1@30,5}\">=\"1\n");
	printf("{1@30,5}\">\"1\n");
	printf("{1@30,5}\"<=\"1\n");
	printf("{1@30,5}\"<\"1\n");
}

int busi_argv(int argc, char *argv[])
{
	int i;
	char buffer[1024+1];
	char *p_buffer;
	
	if(argc < 5)
	{
		return -1;
	}
	
	strcpy(g_stAppData.in_filename, argv[1]);
	strcpy(g_stAppData.cmp_filename, argv[2]);
	strcpy(g_stAppData.nocmp_filename, argv[3]);
	
	g_stAppData.line_number = atoi(argv[4]);
	
	g_stAppData.opt_number = argc - 5;
	
	for(i = 0; i < g_stAppData.opt_number; i++)
	{
		memset(buffer, 0, sizeof(buffer));
		
		sscanf(
			argv[5+i], 
			"{%d@%d,%d}", 
			&g_stAppOpt[i].line_no,
			&g_stAppOpt[i].begin_pos,
			&g_stAppOpt[i].seek_pos
		);

		strcpy(buffer, strstr(argv[5+i], "}")+1);

		switch(buffer[0])
		{
			case '=':
				p_buffer = buffer+1;
				g_stAppOpt[i].equ_opt = 1;
				break;
			case '!':
				p_buffer = buffer+2;
				g_stAppOpt[i].equ_opt = 2;
				break;
			case 'l':
				p_buffer =  buffer+4;
				g_stAppOpt[i].equ_opt = 3;
				break;
			case '>':
				p_buffer=buffer+1;
				if(*p_buffer == '=')
				{
					g_stAppOpt[i].equ_opt = 5;
					p_buffer++;
				}
				else
				{
					g_stAppOpt[i].equ_opt = 4;
				}
				break;
			case '<':
				p_buffer=buffer+1;
				if(*p_buffer == '=')
				{
					g_stAppOpt[i].equ_opt = 7;
					p_buffer++;
				}
				else
				{
					g_stAppOpt[i].equ_opt = 6;
				}				
				break;
			default:
				return -1;
		}
		
		if(p_buffer[0] != '{')
		{
			strcpy(g_stAppOpt[i].content, p_buffer);
		}
		else
		{
			sscanf(
				p_buffer, 
				"{%d@%d,%d}", 
				&g_stAppOpt[i].line_no1,
				&g_stAppOpt[i].begin_pos1,
				&g_stAppOpt[i].seek_pos1
				);			
		}

/*		
printf("%d %d %d %d %s %d %d %d\n",
g_stAppOpt[i].line_no,
g_stAppOpt[i].begin_pos,
g_stAppOpt[i].seek_pos,
g_stAppOpt[i].equ_opt,
g_stAppOpt[i].content,
g_stAppOpt[i].line_no1,
g_stAppOpt[i].begin_pos1,
g_stAppOpt[i].seek_pos1
);
*/

		g_stAppOpt[i].line_no--;
		g_stAppOpt[i].begin_pos--;
		g_stAppOpt[i].line_no1--;
		g_stAppOpt[i].begin_pos1--;
	}

	return 0;
}

int busi_cmp(void)
{
	int iLoop;
	char tmp_buffer[1024+1];
	char content_buffer[1024+1];

	for(iLoop = 0; iLoop < g_stAppData.opt_number; iLoop++)
	{
		memset(content_buffer, 0, sizeof(content_buffer));
		
		if(g_stAppOpt[iLoop].line_no1 < 0)
		{
			strcpy(content_buffer, g_stAppOpt[iLoop].content);
		}
		else
		{
			memcpy(
				content_buffer,
				g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no1]+g_stAppOpt[iLoop].begin_pos1, 
				g_stAppOpt[iLoop].seek_pos1
			);
		}

		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		switch(g_stAppOpt[iLoop].equ_opt)
		{
			case 1:

				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);

				if(memcmp(tmp_buffer, content_buffer, g_stAppOpt[iLoop].seek_pos) == 0)
				{
					;
				}
				else
				{
					return -1;
				}
				break;
			case 2:
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);

				if(memcmp(tmp_buffer, content_buffer, g_stAppOpt[iLoop].seek_pos) != 0)
				{
					;
				}
				else
				{
					return -1;
				}					
				break;
			case 3:				
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);

				if(strstr(tmp_buffer, content_buffer) != NULL)
				{
					;
				}
				else
				{
					return -1;
				}
				break;
			case 4:
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);
				if(atol(tmp_buffer) > atol(content_buffer))
				{
					;
				}
				else
				{
					return -1;
				}
				break;
			case 5:
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);
				if(atol(tmp_buffer) >= atol(content_buffer))
				{
					;
				}
				else
				{
					return -1;
				}				
				break;
			case 6:
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);
				if(atol(tmp_buffer) < atol(content_buffer))
				{
					;
				}
				else
				{
					return -1;
				}				
				break;
			case 7:
				memcpy(tmp_buffer, g_stAppData.line_buffer[g_stAppOpt[iLoop].line_no]+g_stAppOpt[iLoop].begin_pos, g_stAppOpt[iLoop].seek_pos);
				if(atol(tmp_buffer) <= atol(content_buffer))
				{
					;
				}
				else
				{
					return -1;
				}				
				break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int iRcode;
	int iLoop;

	iRcode = busi_argv(argc, argv);
	if(iRcode)
	{
		busi_version();
		return -1;
	}

	g_stAppData.fp_in = fopen(g_stAppData.in_filename, "r");
	g_stAppData.fp_cmp = fopen(g_stAppData.cmp_filename, "w");
	g_stAppData.fp_nocmp = fopen(g_stAppData.nocmp_filename, "w");

	if(g_stAppData.fp_in == NULL || g_stAppData.fp_cmp == NULL || g_stAppData.fp_nocmp == NULL)
	{
		fprintf(stderr, "file error!\n");
		return -1;
	}

	for(iLoop = 0; iLoop < g_stAppData.line_number; iLoop++)
	{
		memset(g_stAppData.line_buffer[iLoop], 0, sizeof(g_stAppData.line_buffer[iLoop]));		
		fgets(g_stAppData.line_buffer[iLoop], sizeof(g_stAppData.line_buffer[iLoop]), g_stAppData.fp_in);
	}

	while(!feof(g_stAppData.fp_in))
	{
		iRcode = busi_cmp();
		if(iRcode)
		{
			for(iLoop = 0; iLoop < g_stAppData.line_number; iLoop++)
			{
				fputs(g_stAppData.line_buffer[iLoop], g_stAppData.fp_nocmp);
			}
		}
		else
		{
			for(iLoop = 0; iLoop < g_stAppData.line_number; iLoop++)
			{
				fputs(g_stAppData.line_buffer[iLoop], g_stAppData.fp_cmp);
			}
		}
	
		for(iLoop = 0; iLoop < g_stAppData.line_number; iLoop++)
		{
			memset(g_stAppData.line_buffer[iLoop], 0, sizeof(g_stAppData.line_buffer[iLoop]));		
			fgets(g_stAppData.line_buffer[iLoop], sizeof(g_stAppData.line_buffer[iLoop]), g_stAppData.fp_in);
		}
	}

	fclose(g_stAppData.fp_cmp);
	fclose(g_stAppData.fp_nocmp);
	fclose(g_stAppData.fp_in);

	printf("ok!\n");

	return 0;
}

