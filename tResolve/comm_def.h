#ifndef _COMM_H
#define _COMM_H

#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <fstream>
#include <queue>
#include <map>
#include <set>
#include <strings.h>
#include <vector>
#include <iconv.h>
#include <error.h>
#include <math.h>

typedef	struct _config{
	char path_name[2][256];		/*放置在两个目录区分新旧数据*/
	char cime_path[256];			/*所有Cim放在同一目录，通过标识区分新旧模型*/
	char oracle_path[256];
	char back_path[256];
	char mx_path[256];
	char log_path[256];
	int day;
	int max_thread;
	int poll_interval;
	int max_record;
	char server[256];
	char user[50];
	char password[50];
	int path_num; /*path_name配置目录数*/
}config;

extern config g_config;

void Log(const char* fmt, ...);

int InitXmlMutex();

int InitXmlThread();

bool InitVariable();

int set_default_conf(config &Pa_config);

int read_config(char* filename, config& Pa_config);

void *process_traversal();

void *process_cim();

void *process_queue_xml();

int dequeue_xml_recv(char *filename);

int enqueue_xml_recv(char *filename);

int getorgan();

/* // @brief 读取需要过滤文件名称关键字 */
int ReadFileKeyFromFilterFile();

//校验模型文件是否发送
int checkmodel();

//添加模型校验初始化数据
int init_modelcheck();


void *xmlLoop();

int SplitStrByCh(const char *_str, const char *_home,char _c, char _des[][256]);

#endif


