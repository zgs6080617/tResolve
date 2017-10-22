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

typedef	struct _config{
	char path_name[256];
	char cime_path[256];
	char oracle_path[256];
	char back_path[256];
	char mx_path[256];
	int day;
	int max_thread;
	int poll_interval;
	int max_record;
	char server[256];
	char user[50];
	char password[50];
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

void *xmlLoop();

#endif


