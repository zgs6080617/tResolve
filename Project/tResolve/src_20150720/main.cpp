#include "comm_def.h"
#include "resolvexml.h"
#include "oci_db.h"

//class resolvexmldata reslov_file;
char *thr_xml_resolve[MAX_THR_RESOLVE];
pthread_t thr_xml[MAX_THR_RESOLVE];
void * thr_result[MAX_THR_RESOLVE];
config g_config;
xml_arg thr_xml_mutex;
pthread_mutex_t text_tqrelease_mutex;
pthread_mutex_t text_FA_mutex;
pthread_rwlock_t lockfile;
multimap<string,disdesk> disdesk_map;

D5000_DB d5000;
int del;


int main(int argc,char **argv)
{
	/*pid_t pid,sid;
	pid = fork();
	if (pid < 0) 
	{
		printf("fork-1\n");
			exit(1);
	}
	else if(pid > 0)
	{
		printf("fork+1\n");
		exit(0);
	}
	umask(0);
	printf("fork=0\n");
	sid = setsid();
	if (sid < 0) 
	{	
		printf("setsid\n");
		exit(0);
	}
	if(chdir("/") < 0)
	{
		printf("chdir\n");
		exit(0);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);*/
	Log("------------------------the program is starting-------------------\n\n");

	//reslov_file.mem_init();

	del = 0;
	if(argc == 2)
		del = 1;

	if (!InitVariable())
	{
		Log("\n-------------------can't Initialize Variable---------------------------\n");
		exit(0);
	}

	/*if(d5000.d5000_Connect() != 0)
		return -1;*/

	//sleep(5);
	xmlLoop();
	//d5000.d5000_DisConnect();

	Log("\n------------------------------the program is end------------------------------\n");

	return 0;
}
