#include "resolvexml.h"

using namespace std;

char *file_visit[256];

#define INT_LEN 4
#define FLOAT_LEN 4 
#define STR_LEN_DD 10
#define STR_LEN_SS 25
#define STR_LEN_CHAR 64
#define STR_LEN_TYPE 10
#define STR_LEN_NAME 100
#define STR_LEN_ID 128
#define STR_LEN_ZDNAME 256
#define STR_LEN_DETAILS 4000
#define STR_LEN_ZLCOT 3000
#define STR_LEN_LONGNAME 10000
#define _2M_ 2*1024*1024
#define ZLP_CTL_SIZE 2//
#define DC_SIZE 9 //地级市数量

void* threadlogwrite(void *param);

int mvxmlfile(const char *filename,int type);

void Log(const char* fmt, ...)
{
	char log_buf[1024] = {0};
	va_list		ap;
	va_start(ap, fmt);
	vsprintf(log_buf + strlen(log_buf), fmt, ap);
	va_end(ap);
	cout << log_buf;	//always printf log in console
}

int getmonday()
{
	int y,m,d;
	time_t time_temp;
	struct tm *tm_temp;
	
	time_temp = time(NULL);
	tm_temp = localtime(&time_temp);
	m = tm_temp->tm_mon+1;
	y = tm_temp->tm_year+1900;
	switch(m)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		d = 31;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		d = 30;
		break;
	case 2:
		if(y%4 == 0 && y%100 == 0 || y%400 == 0)
			d=29;
		else
			d=28;
		break;
	}
	return d-tm_temp->tm_mday+1;
}

int remainderday()
{
	int y,m,d,w,retd;
	time_t time_temp;
	struct tm *tm_temp;

	time_temp = time(NULL);
	tm_temp = localtime(&time_temp);
	m = tm_temp->tm_mon+1;
	y = tm_temp->tm_year+1900;
	d = tm_temp->tm_mday;

	w = tm_temp->tm_wday;

	int month_num = (tm_temp->tm_yday - tm_temp->tm_wday) / 7 + 1;
	if(month_num % 2 != 0 )	//判断是否单周
	{
		if (tm_temp->tm_wday == 0)
		{
			retd = 1;
		}
		else
		{
			retd = 14 - tm_temp->tm_wday + 1;
		}
	}
	else
	{
		if (tm_temp->tm_wday == 0)
		{
			retd = 8;
		}
		else
		{
			retd = 7-tm_temp->tm_wday +1;
		}
	}
	return retd;
}

//移除dmsxml数据文件，深夜移除当天dmsxml文件，以免干扰凌晨指标计算，add by lcm 20150727
int RemoveFile()
{
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];

	memset(path_name, 0x00, sizeof(path_name));

	sprintf(path_name, "%s", g_config.path_name);

	if ((dirp = opendir(path_name)) == NULL)
	{
		printf("open dir %s error!", path_name);
		return -1;
	}

	while ((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
			continue;
		else
		{
			mvxmlfile(dirt->d_name, 0);
			//remove(dirt->d_name);
		}
	}
	closedir(dirp);
	
	char timenow[100];
	char oldname[100];
	char newname[100];
	memset(oldname, 0x00, sizeof(oldname));
	memset(newname, 0x00, sizeof(newname));
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
	memset(path_name, 0x00, sizeof(path_name));

	sprintf(path_name, "%s", g_config.cime_path);

	if ((dirp = opendir(path_name)) == NULL)
	{
		printf("open dir %s error!", path_name);
		return -1;
	}

	while ((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
			continue;
		else
		{
			sprintf(oldname,"%s/%s",g_config.cime_path,dirt->d_name);
			sprintf(newname,"%s/%s/%s",g_config.back_path,timenow,dirt->d_name);
			rename(oldname,newname);
			//remove(dirt->d_name);
		}
	}
	closedir(dirp);
	return 0;
}

int TruncateHisrecord()
{
	char qury[200];
	int y,m,d,w;
	time_t time_temp;
	struct tm *tm_temp;

	time_temp = time(NULL);
	tm_temp = localtime(&time_temp);
	m = tm_temp->tm_mon+1;
	y = tm_temp->tm_year+1900;
	d = tm_temp->tm_mday;

	w = tm_temp->tm_wday;

	int month_num = (tm_temp->tm_yday - tm_temp->tm_wday) / 7 + 1;
	if(month_num % 2 != 0 && tm_temp->tm_wday == 1)	//判断是否单周周一
	{
		sprintf(qury,"TRUNCATE TABLE EVALUSYSTEM.CONFIG.HISRECORD;");
		if(d5000.d5000_ExecSingle(qury)!=0)
		{
			return -1;
		}
	}
	return 0;
}

int getdisdesk()
{

	if(d5000.d5000_Connect() != 0)
		return -1;

	char query[200],err_info[200],tempstr[100],tmp[20];
	char *buf = NULL,*m_presult = NULL;
	struct ColAttr *attrs = NULL;
	memset(query,0x00,sizeof(query));
	memset(err_info,0x00,sizeof(err_info));
	strcpy(query,"select organ_code,area,desk from evalusystem.config.disdesk;");
	int rec_num = 0,attr_num = 0,rec = 0,col = 0,offset = 0;
	if(d5000.d5000_ReadData(query,&rec_num,&attr_num,&attrs,&buf,err_info) == 0)
	{
		m_presult = buf;
		disdesk temp_disdesk;
		for (rec = 0; rec < rec_num; rec++)
		{
			memset(&temp_disdesk,0x00,sizeof(temp_disdesk));
			for(col = 0; col < attr_num; col++)
			{
				memset(tempstr, 0x00 , 100);
				memcpy(tempstr, m_presult, attrs[col].data_size);
				switch(col)
				{
				case 0:
					temp_disdesk.organ_code = *(int*)tempstr;
					break;
				case 1:
					strncpy(temp_disdesk.area,tempstr,sizeof(temp_disdesk.area));
					break;
				case 2:
					strncpy(temp_disdesk.desk,tempstr,sizeof(temp_disdesk.desk));
					break;
				default:
					break;
				}
				m_presult += attrs[col].data_size;
			}
			disdesk_map.insert(make_pair(temp_disdesk.desk,temp_disdesk));
		}
	}
	else
	{
		d5000.d5000_DisConnect();
		return -1;
	}
	if(rec_num > 0)
		d5000.g_CDci.FreeReadData(attrs,attr_num,buf);
	else
		d5000.g_CDci.FreeColAttrData(attrs, attr_num);
	d5000.d5000_DisConnect();
	return 0;
}

/********************************
IN arg:成功失败：1成功 2失败
IN code:供电公司编码
IN str:表名称或文件名
IN info:信息
OUT threadlog:存储
********************************/

int SeperateStringsFromOneLine( char* strBuffer, char (* strSubString_s)[300] )
{
	int i=0, num=0, nCount=0, nWaingNewFlag=1;
	for(;strBuffer[i]!='\0'&&strBuffer[i]!='\n'&&strBuffer[i]!='\r';i++)
	{
		if(strBuffer[i]==' '||strBuffer[i]=='\t'||strBuffer[i]=='-'||strBuffer[i]=='_')
			nWaingNewFlag=1;
		else 
		{
			if(1==nWaingNewFlag)
			{
				num++;
				nWaingNewFlag=0;
				nCount=0;
			}
			if(nCount == 0 && strBuffer[i] == '_')//处理_和ID连在一起的不规则文件格式
			{
				strSubString_s[num-1][nCount]=strBuffer[i];
				strSubString_s[num-1][nCount+1]='\0';
				num++;
				nCount = 0;
			}
			else if(nCount != 0 && strBuffer[i] == '_')
			{
				num++;
				nCount = 0;
				strSubString_s[num-1][nCount]=strBuffer[i];
				strSubString_s[num-1][nCount+1]='\0';
				num++;
			}
			else
			{
				strSubString_s[num-1][nCount]=strBuffer[i];
				strSubString_s[num-1][nCount+1]='\0';
				nCount++;
			}
		}
	}
	return num;
}

void resolvexmldata::SetGError(int arg,int code,const char * str,int watch,const char * info,logstr& threadlog)
{
	/*char temp[100];
	memset(temp,0x00,sizeof(temp));*/

	if(threadlog.arg != 2)
		threadlog.arg = arg;

	if(code != 0)
		threadlog.code = code;

	if (watch != -1)
		threadlog.name = watch;
	if(str != NULL)
		strcat(threadlog.log,str);
	if(info != NULL)
		strcat(threadlog.log,info);
}

void resolvexmldata::GetGError(const char *filename,logstr& threadlog)
{
	if (strlen(threadlog.log) == 0)
		return;
	char temp[500],timenow[20];
	char error_info[100];
	memset(error_info,0x00,sizeof(error_info));
	
	const char *g_indexname[] = {"配电主站月平均运行率","FA投运率","遥信正确率","挂牌信息一致率","拓扑正确率","终端在线率","遥控成功率","遥控使用率",
		"DMS与用采公专变台账匹配率","DMS公专变采集成功率","配电母线数完整率","配电开关数完整率","配电刀闸数完整率","设备状态一致率","配电变压器完整率","配电站房数完整率",
		"DMS停电信息发布完整率","自动成图率","自动成图数量","停电信息确认率","停电信息发布及时率","停电信息发布时间","FA动作次数","公专变明细","停电故障率","停电故障明细","GPMS指令票明细","DMS指令票明细","小水电明细"};

	memset(temp,0x00,sizeof(temp));
	char path_name[100];
	memset(timenow,0x00,sizeof(timenow));
	memset(path_name,0x00,sizeof(path_name));
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
	sprintf(path_name,"%s/%s",g_config.back_path,timenow);
	
	int type = 1,level = threadlog.arg,code = threadlog.code;

	if(threadlog.arg == 0)
	{
		sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"指标文件内容为空",threadlog.code,g_indexname[threadlog.name]);
	}
	else if(threadlog.arg == 1)
	{
		sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"指标数据正确",threadlog.code,g_indexname[threadlog.name]);
	}
	else
	{
		sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,threadlog.log,threadlog.code,g_indexname[threadlog.name]);
	}
	if(d5000.d5000_ExecSingle(temp)!=0)
	{
		return;
	}
	
	if (threadlog.name == 8 || threadlog.name == 12 || threadlog.name == 17 || threadlog.name == 19)//"DMS与用采公专变台账匹配率","配电开关数完整率","自动成图率","停电信息确认率"
	{
		if(threadlog.arg == 0)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,"所给文件内容为空",threadlog.code,g_indexname[threadlog.name+1]);
		else if(threadlog.arg == 1)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,"指标数据正确",threadlog.code,g_indexname[threadlog.name+1]);
		else
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,threadlog.log,threadlog.code,g_indexname[threadlog.name+1]);
				
		if(d5000.d5000_ExecSingle(temp)!=0)
		{
			return;
		}
	}
	
	if(threadlog.name == 19)//"停电信息确认率"
	{
		if(threadlog.arg == 0)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"所给文件内容为空",threadlog.code,g_indexname[threadlog.name+2]);
		else if(threadlog.arg == 1)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"指标数据正确",threadlog.code,g_indexname[threadlog.name+2]);
		else
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,threadlog.log,threadlog.code,g_indexname[threadlog.name+2]);
		if(d5000.d5000_ExecSingle(temp)!=0)
		{
			return;
		}
	}

	sprintf(temp,"INSERT INTO EVALUSYSTEM.MANAGE.LOG(TYPE,ORGAN_CODE,CONTENT,LEVEL,GTIME) VALUES(%d,%d,'%s',%d,sysdate)",type,code,threadlog.log,level);
	
	if(d5000.d5000_ExecSingle(temp)!=0)
	{
		return;
	}
	
	//数据校验数据写入数据库 add by lcm 20150917
	if(threadlog.arg == 0)
	{
		int offsetpos = 0;
		char query[200];
		memset(query,0x00,sizeof(query));
		struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
		if(attrs == NULL)
			return;
		attrs[0].data_type = DCI_STR;
		attrs[0].data_size = STR_LEN_NAME;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_NAME;
		attrs[2].data_type = DCI_INT;
		attrs[2].data_size = INT_LEN;
		attrs[3].data_type = DCI_STR;
		attrs[3].data_size = STR_LEN_NAME;
		char *temp_err = (char *)malloc((size_t)(1000));
		if(temp_err == NULL)
			return;
		memset(temp_err,0x00,1000);
		
		memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_err+offsetpos,&(threadlog.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		
		sprintf(query,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
			"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'指标文件内容为空','dms');");
		if(d5000.d5000_WriteData("DMSCBL_IMAGE",query,temp_err,1,4,attrs,error_info) != 0)
		{
			return;
		}

		free(temp_err);
		free(attrs);
		attrs = NULL;
		temp_err = NULL;
	}
	else if(threadlog.arg == 2)
	{
		strcat(path_name,"/err");
		int offsetpos = 0;
		char query[200];
		memset(query,0x00,sizeof(query));
		struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
		if(attrs == NULL)
			return;
		attrs[0].data_type = DCI_STR;
		attrs[0].data_size = STR_LEN_NAME;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_NAME;
		attrs[2].data_type = DCI_INT;
		attrs[2].data_size = INT_LEN;
		attrs[3].data_type = DCI_STR;
		attrs[3].data_size = STR_LEN_NAME;
		attrs[4].data_type = DCI_STR;
		attrs[4].data_size = STR_LEN_ZDNAME;
		char *temp_err = (char *)malloc((size_t)(1000));
		if(temp_err == NULL)
			return;
		memset(temp_err,0x00,1000);
		
		memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_err+offsetpos,&(threadlog.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_err+offsetpos,&(threadlog.log),STR_LEN_ZDNAME);
		offsetpos += STR_LEN_ZDNAME;
		
		sprintf(query,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
			"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5,'dms');");
		if(d5000.d5000_WriteData("DMSCBL_IMAGE",query,temp_err,1,5,attrs,error_info) != 0)
		{
			return;
		}

		free(temp_err);
		free(attrs);
		attrs = NULL;
		temp_err = NULL;
	}
}



//文件名存入文件，代表已处理
int insertfile(const char *filename)
{
	if(filename == NULL)
		return -1;

	int ret = 0;

	pthread_rwlock_rdlock(&lockfile);
	FILE *fp = fopen("traversalfile.txt","a+");
	if(fp == NULL)
	{
		pthread_rwlock_unlock(&lockfile);
		Log("fopen fail,%s can't write in.%s\n",filename);
		return -1;
	}

	fprintf(fp,"%s\n",filename);

	fclose(fp);
	pthread_rwlock_unlock(&lockfile);

	return 0;
}

int insertxmlfile(const char *filename,int citycode)
{
	if(filename == NULL)
		return -1;
	int organ_code[] = {35401,35402,35403,35404,35405,35406,35407,35408,35409};
	//int ret = 0;
	//FILE *fp = NULL;
	//char messSplit[2][3][200];
	//const char *g_logname[] = {"fz.txt","xm.txt","nd.txt","pt.txt","qz.txt","zz.txt","ly.txt","sm.txt","np.txt"};
	//fp = fopen(g_logname[citycode],"a+");
	//if(fp == NULL)
	//{
	//	Log("fopen fail,%s can't write in.%s\n",filename);
	//	return -1;
	//}
	//memset(messSplit,0,sizeof(messSplit));
	////SeperateStringsFromOneLine(filename,messSplit[0]);
	////fprintf(fp,"%s\n",messSplit[0][0]);
	//fprintf(fp,"%s\n",filename);

	//fclose(fp);

	char qury[200];
	memset(qury,0,sizeof(qury));
	sprintf(qury,"INSERT INTO EVALUSYSTEM.CONFIG.HISRECORD(ORGAN_CODE,NAME,GTIME) values(%d,'%s',to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'));",citycode,filename);
	if ( d5000.d5000_ExecSingle(qury)!=0)
	{
		return -1;
	}
	return 0;
}

/*****************************
type:0:成功 1:失败
*****************************/
int mvxmlfile(const char *filename,int type)
{
	if(filename == NULL)
		return -1;

	int ret = 0;
	char newfile[256];
	char oldfile[256];
	memset(newfile,0x00,sizeof(newfile));
	memset(oldfile,0x00,sizeof(oldfile));
	char timenow[100];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);

	//将文件移除到保存目录
	if(type == 0)
		sprintf(newfile,"%s/%s/%s",g_config.back_path,timenow,filename);
	else if(type == 1)
		sprintf(newfile,"%s/%s/err/%s",g_config.back_path,timenow,filename);
	else if(type == 2)
		sprintf(newfile,"%s/%s",g_config.oracle_path,filename);

	/*if(strstr(filename,"TZ") != NULL || strstr(filename,"tzlist") != NULL)
		sprintf(newfile,"%s/%s/TZ/%s",g_config.back_path,timenow,filename);*/

	sprintf(oldfile,"%s/%s",g_config.path_name,filename);

	ret = rename(oldfile,newfile);
	if(ret == -1)
	{
		Log("%s mv fail.errno:%s.newfile:%s.\n",oldfile,strerror(errno),newfile);
		return -1;
	}
	return 0;
}

int InitXmlMutex()
{
	int ret = pthread_mutex_init(&thr_xml_mutex.queue_mutex_file, NULL);//初使化线程锁
	if(ret < 0)
	{
		Log("----- 初使化解析xml文件线程锁失败 ----------\n");
	}

	ret = pthread_cond_init(&thr_xml_mutex.queue_cond_file, NULL);   //初使化条件变量
	if(ret < 0)
	{
		Log("----- 初使化解析xml文件线程条件变量失败  ----------\n");
	}
	ret = pthread_rwlock_init(&lockfile,NULL);
	if(ret < 0)
	{
		Log("----- 初使化文件锁失败  ----------\n");
	}
	return ret;
}

int InitXmlThread()
{
	int ret = 0,i = 0;
	pthread_t trathr,cimthr;
	time_t time_temp;
	struct tm *tm_temp;
	void * trathrresult = NULL;
	int maxthr = (g_config.max_thread < MAX_THR_RESOLVE)? g_config.max_thread:MAX_THR_RESOLVE;

	//创建写日志线程
	pthread_t pt;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	if (pthread_create(&pt, &attr, threadlogwrite, NULL) != 0)
	{
		printf("创建写日志线程失败！\r\n");
		return -1;
	}
	pthread_detach(pt);

	for (;i<maxthr;i++)
	{
		ret=pthread_create(&thr_xml[i],NULL,(void*(*)(void*))process_queue_xml,NULL);
		if(ret<0)
		{
			Log("------- 创建xml处理线程%d失败 : %s ---------------------\n",i,strerror(ret));
		}
	}

	while(1)
	{

		if((ret = pthread_create(&cimthr,NULL,(void*(*)(void*))process_cim,NULL))<0)
		{
			Log("------- 创建文件夹遍cim历线程失败 : %s ---------------------\n",strerror(ret));
		}
		pthread_join(cimthr,&trathrresult);

		if((ret = pthread_create(&trathr,NULL,(void*(*)(void*))process_traversal,NULL))<0)
		{
			Log("------- 创建文件夹遍历线程失败 : %s ---------------------\n",strerror(ret));
		}

		pthread_join(trathr,&trathrresult);


		Log("thread quit.\n");
		while(1)
		{
#ifdef _TEST_
			g_config.poll_interval = 30;
#endif
			sleep(g_config.poll_interval);
#ifdef _TEST_
			break;
#else
			time_temp = time(NULL);
			tm_temp = localtime(&time_temp);
			/*if(tm_temp->tm_hour == 0 || tm_temp->tm_hour == 1 || tm_temp->tm_hour == 2 || tm_temp->tm_hour == 3 || tm_temp->tm_hour == 4 || tm_temp->tm_hour == 5)*/
			if (tm_temp->tm_hour == 0)
			{
				if(d5000.d5000_Connect() != 0)
					return -1;
				remove("traversalfile.txt");
				TruncateHisrecord();
				break;
			}
			else if(tm_temp->tm_hour == 3)
			{
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (90-tm_temp->tm_min)*60;
				continue;
			}
			else if(tm_temp->tm_hour == 4)
			{
				del = 0;
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if(tm_temp->tm_hour == 5)//为解析gpms停电信息单独遍历
			{
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (90-tm_temp->tm_min)*60;
				continue;
			}
			else if(tm_temp->tm_hour == 6)
			{
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if(tm_temp->tm_hour == 7)//为解析小水电单独遍历
			{
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if (tm_temp->tm_hour == 12)
			{
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (70 - tm_temp->tm_min) * 60;
				d5000.d5000_DisConnect();
				continue;
			}
			else if (tm_temp->tm_hour == 13)
			{
				if(d5000.d5000_Connect() != 0)
					return -1;
				del = 1;
				remove("traversalfile.txt");
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if (tm_temp->tm_hour == 23)//删除dmsxml数据文件，深夜删除当天dmsxml文件，以免干扰凌晨指标计算，add by lcm 20150727
			{
				if(RemoveFile()!=0)
				{
					printf("删除文件出错!");
				}else
				{
					printf("移除文件成功!");
				}
				d5000.d5000_DisConnect();
				continue;
			}
			else
				continue;
#endif
		}
	}
	//pthread_join(trathr,&trathrresult);

	for (i=0;i<maxthr;i++)
	{
		pthread_join(thr_xml[i],&thr_result[i]);
	}
}

//文件名出队列
int dequeue_xml_recv(char *filename)
{
	pthread_mutex_lock(&thr_xml_mutex.queue_mutex_file);
	while (thr_xml_mutex.queue_file.size()== 0 )
	{
		pthread_cond_wait(&thr_xml_mutex.queue_cond_file,&thr_xml_mutex.queue_mutex_file);
	}
	//Log("queue_file: %s,back:%s,size:%d\n",	thr_xml_mutex.queue_file.front().c_str(),thr_xml_mutex.queue_file.back().c_str(),thr_xml_mutex.queue_file.size());
	strcpy(filename,thr_xml_mutex.queue_file.front().c_str());
	if(thr_xml_mutex.queue_file.size() != 0)
		thr_xml_mutex.queue_file.pop();
	pthread_mutex_unlock(&thr_xml_mutex.queue_mutex_file);
	//Log("filename:%s.\n",filename);
	return 0;
}

//处理文件线程回调函数
void *process_queue_xml()
{
	char full_src_path[250];
	char full_des_path[250];
	char tempname[256];
	while(1)
	{
		//cout << "in process_queue_xml" << endl;
		memset(tempname,0x00,sizeof(tempname));
		memset(full_src_path,0x00,sizeof(full_src_path));
		memset(full_des_path,0x00,sizeof(full_des_path));
		dequeue_xml_recv(tempname);
		if(tempname == NULL)
			break;
		//cout << "in process_queue_xml_2" << endl;
		mylog threadlog;
		memset(&threadlog,0x00,sizeof(mylog));
		class resolvexmldata reslov_file;
		//reslov_file.mem_init();
		//reslov_file.readdata_file(tempname,threadlog);
		reslov_file.readdata_xml(tempname,threadlog);
	}
}

int set_default_conf(config &Pa_config)
{
	strcpy(Pa_config.path_name,"/home/d5000/dpkf/Server/doc");
	strcpy(Pa_config.cime_path,"/home/d5000/dpkf/Server/doc");
	Pa_config.max_thread = 100;
	Pa_config.poll_interval = 5;
	Pa_config.max_record =  5000;
	return true;
}

//读取配置文件
int read_config(char* filename, config& Pa_config)
{
	if( NULL == filename)
		return -1;

	bool is_exist = access(filename, F_OK) < 0 ? false : true;
	if(!is_exist)
	{
		Log("config file %s don't exist!!\n", filename);
		return -1;
	}

	TiXmlDocument doc(filename);
	if (!doc.LoadFile())
	{
		Log("Can't load file %s.\n", filename);
		return -1;
	}
	else
		Log("Load file %s success.\n",filename);

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);
	pElem = hDoc.FirstChildElement().Element();
	if (!pElem) 
		return false;

	hRoot = TiXmlHandle(pElem);

	pElem = hRoot.FirstChild("Body").FirstChild().Element();
	if(!pElem)
		return false;

	const char* pattrname = NULL;
	const char* pattrvalue = NULL;

	for(; pElem != NULL; pElem=pElem->NextSiblingElement())
	{
		/*std::string paraname;
		std::string paravalue;*/

		pattrname = pElem->Attribute("paramname");
		if(!pattrname)
			return false;
		//paraname = std::string(pattr);
		pattrvalue = pElem->Attribute("paramvalue");
		if(!pattrvalue)
			return false;

		//paravalue = std::string(pattr);

		if(strcmp(pattrname,"max_thread") == 0)
			Pa_config.max_thread = atoi(pattrvalue);
		else if(strcmp(pattrname,"poll_interval") == 0)
			Pa_config.poll_interval = atoi(pattrvalue);
		else if(strcmp(pattrname,"path_name") == 0)
			strcpy(Pa_config.path_name,pattrvalue);
		else if(strcmp(pattrname,"oracle_path") == 0)
			strcpy(Pa_config.oracle_path,pattrvalue);
		else if(strcmp(pattrname,"cime_path") == 0)
			strcpy(Pa_config.cime_path ,pattrvalue);
		else if (strcmp(pattrname,"back_path") == 0)
			strcpy(Pa_config.back_path,pattrvalue);
		else if (strcmp(pattrname,"mx_path") == 0)
			strcpy(Pa_config.mx_path,pattrvalue);
		else if (strcmp(pattrname,"log_path") == 0)
			strcpy(Pa_config.log_path,pattrvalue);
		else if (strcmp(pattrname,"day") == 0)
			Pa_config.day = atoi(pattrvalue);
		else if(strcmp(pattrname,"max_record") == 0)
			Pa_config.max_record = atoi(pattrvalue);
		else if (strcmp(pattrname,"db_server") == 0)
		{
			if(pattrvalue!=NULL)
				strcpy(Pa_config.server,pattrvalue);
			else
				strcpy(Pa_config.server,getenv("DMS_OCI_SRV"));
		}
		else if(strcmp(pattrname,"db_user") == 0)
			strcpy(Pa_config.user,pattrvalue);
		else if(strcmp(pattrname,"db_password") == 0)
			strcpy(Pa_config.password,pattrvalue);
	}
	//Pa_config.day = getmonday();
	return 0;
}

//创建多级目录
int CreateMultFolder(char *path)
{
	int i,len;
	char path_temp[256];
	memset(path_temp,0x00,sizeof(path_temp));

	strcpy(path_temp,path);

	len = strlen(path_temp);
	if(path_temp[len-1] != '/')
	{
		strcat(path_temp,"/");
		len++;
	}

	//默认格式"/"开始，处理忽略掉开始"/"
	for (i=1;i<len;i++)
	{
		if(path_temp[i] == '/')
		{
			path_temp[i] = 0;
			if(mkdir(path_temp,0777) == -1)
			{
				if(errno != EEXIST)
					return -1;
			}
			path_temp[i] = '/';
		}
	}
	return 0;
}

//创建存储文件夹
int CreateFolder()
{
	int ret = 0;
	char path[256];
	memset(path,0x00,sizeof(path));

	/*sprintf(path,"%s/dms/",g_config.cime_path);
	if (CreateMultFolder(path))
	{
		Log("%s create fail.Need to manually create.\n",path);
		ret = -1;
	}
	sprintf(path,"%s/gpms/",g_config.cime_path);
	if (CreateMultFolder(path))
	{
		Log("%s create fail.Need to manually create.\n",path);
		ret = -1;
	}*/
	sprintf(path,"%s/dms/",g_config.back_path);
	if (CreateMultFolder(path))
	{
		Log("%s create fail.Need to manually create.\n",path);
		ret = -1;
	}
	sprintf(path,"%s/gpms/",g_config.back_path);
	if (CreateMultFolder(path))
	{
		Log("%s create fail.Need to manually create.\n",path);
		ret = -1;
	}
	sprintf(path,"%s/err/",g_config.back_path);
	if (CreateMultFolder(path))
	{
		Log("%s create fail.Need to manually create.\n",path);
		ret = -1;
	}
	return ret;
}

bool InitVariable()
{
	set_default_conf(g_config);
	if(read_config("config.xml", g_config) !=0)
		return false;
	Log("Traverse Folder:%s\n",g_config.path_name);
	if(getdisdesk() != 0)
		return false;
	//创建必须文件夹
	if(CreateFolder() !=0)
		return false;
	return true;
}

//文件名入队列
int enqueue_xml_recv(char *filename)
{
	pthread_mutex_lock(&thr_xml_mutex.queue_mutex_file);
	//Log("initialsize:%d.\n",thr_xml_mutex.queue_file.size());
	//cout << "initial size:" << thr_xml_mutex.queue_file.size() << endl;
	thr_xml_mutex.queue_file.push(filename);
	pthread_cond_signal(&thr_xml_mutex.queue_cond_file);
	//Log("queue_file.front:%s.\n",thr_xml_mutex.queue_file.front());
	//cout << "queue_file.front:" << thr_xml_mutex.queue_file.front() << " queue_file.back:" << thr_xml_mutex.queue_file.back() << endl;
	pthread_mutex_unlock(&thr_xml_mutex.queue_mutex_file);
	return 0;
}

/*************************
//判断文件是否已经处理过
**************************/
int judexit(const char *name)
{
	int ret = 0;
	char text[100];
	char temp_name[100];
	char temp_path[256];

	pthread_rwlock_wrlock(&lockfile);
	FILE *fp = fopen("traversalfile.txt","r");
	if(fp == NULL)
	{
		pthread_rwlock_unlock(&lockfile);
		if(errno == ENOENT)
			return 1;
		else
		{
			perror("fopen");
			return -1;
		}
	}
	memset(text,0x00,sizeof(text));
	memset(temp_name,0x00,sizeof(temp_name));

	strcpy(temp_name,name);
	while (!feof(fp))
	{
		if( fgets( text,100,fp) == NULL )
			continue;
		
		if (strncmp(text,temp_name,strlen(temp_name)) == 0)
		{
			fclose(fp);
			sprintf(temp_path,"%s/%s",g_config.path_name,temp_name);
			remove(temp_path);
#ifdef _DEBUG
			Log("The file  %s Repeat.\n",temp_name);
#endif // _DEBUG
			pthread_rwlock_unlock(&lockfile);
			return 0;
		}
	}
	/*fflush(fp);
	fseek(fp,0,SEEK_END);
	fprintf(fp,"%s\n",name);*/
	fclose(fp);
	pthread_rwlock_unlock(&lockfile);
	return 1;
}

/*************************
//多层遍历文件夹
**************************/
int TraversalFolder()
{
	int ret = -1;
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];

	memset(path_name,0x00,sizeof(path_name));

	strcpy(path_name,g_config.path_name);

	if((dirp = opendir(path_name)) == NULL)
	{
		perror("open dir error!");
		return -1;
	}

	while((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name,".")==0 || strcmp(dirt->d_name,"..") == 0)
			continue; 

		//if(lstat(dirt->d_name,&st) < 0)
		//{
		//	perror("lstat:");
		//	return -1;
		//}
		//if(S_ISDIR(st.st_mode))
		//{
		//	if(TraversalFolder() == -1)
		//		return -1;
		//}
		//else
		//{
		//	ret = judexit(dirt->d_name);
		//	if (ret == 1)
		//	{
		//		//enqueue_xml_recv(dirt->d_name);
		//	}
		//}
		ret = judexit(dirt->d_name);
		if (ret == 1)
		{
			enqueue_xml_recv(dirt->d_name);
		}
	}
	closedir(dirp);

	return 0;
}

int mvcim::getfilenum(char *path)
{
	int ret = -1,num = 0,len = 0;
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];

	xml_vector.clear();

	memset(path_name,0x00,sizeof(path_name));

	strcpy(path_name,path);

	if((dirp = opendir(path_name)) == NULL)
	{
		perror("open dir error!");
		return -1;
	}

	while((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name,".")==0 || strcmp(dirt->d_name,"..") == 0)
			continue; 
		len = strlen(dirt->d_name);
		if(strcmp(dirt->d_name+len-3,"xml") == 0)
			xml_vector.push_back(dirt->d_name);
	}
	closedir(dirp);

	return 0;
}

int mvcim::mvtocim(char *code)
{
	int num = 0,seq = 0,type = 0,cityhavesize = 0,citycode = 0;
	char trapath[256];
	char oldname[1024];
	char newname[1024];
	memset(trapath,0,sizeof(trapath));
	char timenow[100];
	char temp[200];
	char messSplit[2][3][300];
	char temp_filename[500];
	char temp_file[300];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
	const char *g_logname[] = {"fz.txt","xm.txt","nd.txt","pt.txt","qz.txt","zz.txt","ly.txt","sm.txt","np.txt"};
	int citytype[] = {35401,35402,35403,35404,35405,35406,35407,35408,35409};
	//int cityhavesize[] = {fz_vector.size(),xm_vector.size(),nd_vector.size(),pt_vector.size(),qz_vector.size(),zz_vector.size(),ly_vector.size(),sm_vector.size(),np_vector.size()};
	if(strstr(code,"35401") != NULL)
	{
		cityhavesize = fz_map.count(code);
		citycode = 0;
	}
	else if(strstr(code,"35402") != NULL)
	{
		cityhavesize = xm_map.count(code);
		citycode = 1;
	}
	else if(strstr(code,"35403") != NULL)
	{
		cityhavesize = nd_map.count(code);
		citycode = 2;
	}
	else if(strstr(code,"35404") != NULL)
	{
		cityhavesize = pt_map.count(code);
		citycode = 3;
	}
	else if(strstr(code,"35405") != NULL)
	{
		cityhavesize = qz_map.count(code);
		citycode = 4;
	}
	else if(strstr(code,"35406") != NULL)
	{
		cityhavesize = zz_map.count(code);
		citycode = 5;
	}
	else if(strstr(code,"35407") != NULL)
	{
		cityhavesize = ly_map.count(code);
		citycode = 6;
	}
	else if(strstr(code,"35408") != NULL)
	{
		cityhavesize = sm_map.count(code);
		citycode = 7;
	}
	else if(strstr(code,"35409") != NULL)
	{
		cityhavesize = np_map.count(code);
		citycode = 8;
	}
	sprintf(trapath,"%s/All_Model_%s",g_config.cime_path,code);

	if(getfilenum(trapath) == 0)
	{
		if (xml_vector.size() >= cityhavesize)
		{
			num = (xml_vector.size()-cityhavesize)/g_config.day;
		}
		else//漳州中间换专题图，馈线数目小于已访问馈线数目
		{
			num = xml_vector.size()/30;
		}
		/*if(num == 0)
		{
			num = xml_vector.size();
			type = 1;
		}
*/
		cout << "code:" << code << "\tcim_num:" << xml_vector.size() << "\tcityhave_num:" << cityhavesize << "\tnum:" << num << endl;
		if(xml_vector.size() == cityhavesize)
		{
			//remove(g_logname[citycode]);
			if(citycode == 0)
			{
				fz_map.clear();
				fz_vector.clear();
			}
			else if(citycode == 1)
			{
				xm_map.clear();
				xm_vector.clear();
			}
			else if(citycode == 2)
			{
				nd_map.clear();
				nd_vector.clear();
			}
			else if(citycode == 3)
			{
				pt_map.clear();
				pt_vector.clear();
			}
			else if(citycode == 4)
			{
				qz_map.clear();
				qz_vector.clear();
			}
			else if(citycode == 5)
			{
				zz_map.clear();
				zz_vector.clear();
			}
			else if(citycode == 6)
			{
				ly_map.clear();
				ly_vector.clear();
			}
			else if(citycode == 7)
			{
				sm_map.clear();
				sm_vector.clear();
			}
			else if(citycode == 8)
			{
				np_map.clear();
				np_vector.clear();
			}
		}

		vector<string>::iterator it = xml_vector.begin();
		for (;it != xml_vector.end();it++)
		{
			sprintf(oldname,"%s/%s",trapath,it->c_str());
			memset(messSplit,0,sizeof(messSplit));
			memset(temp_filename,0,sizeof(temp_filename));
			strncpy(temp_filename,it->c_str(),sizeof(temp_filename));
			SeperateStringsFromOneLine(temp_filename,messSplit[0]);
			memset(temp_file,0,sizeof(temp_file));
			strncpy(temp_file,messSplit[0][0],sizeof(temp_file));
			//temp_file[strslen(temp_file)-1] = 0;
			temp_file[strlen(temp_file)] = 0;
			if(seq < num)
			{
				if(citycode == 0)
				{
					vector<string>::iterator it_find = find(fz_vector.begin(),fz_vector.end(),temp_file);
					if(it_find == fz_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 1)
				{
					vector<string>::iterator it_find = find(xm_vector.begin(),xm_vector.end(),temp_file);
					if(it_find == xm_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 2)
				{
					vector<string>::iterator it_find = find(nd_vector.begin(),nd_vector.end(),temp_file);
					if(it_find == nd_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 3)
				{
					vector<string>::iterator it_find = find(pt_vector.begin(),pt_vector.end(),temp_file);
					if(it_find == pt_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 4)
				{
					vector<string>::iterator it_find = find(qz_vector.begin(),qz_vector.end(),temp_file);
					if(it_find == qz_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 5)
				{
					vector<string>::iterator it_find = find(zz_vector.begin(),zz_vector.end(),temp_file);
					if(it_find == zz_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 6)
				{
					vector<string>::iterator it_find = find(ly_vector.begin(),ly_vector.end(),temp_file);
					if(it_find == ly_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 7)
				{
					vector<string>::iterator it_find = find(sm_vector.begin(),sm_vector.end(),temp_file);
					if(it_find == sm_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					}
					remove(oldname);
				}
				else if(citycode == 8)
				{
					vector<string>::iterator it_find = find(np_vector.begin(),np_vector.end(),temp_file);
					if(it_find == np_vector.end())
					{
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						insertxmlfile(it->c_str(),atoi(code));
					 }
					remove(oldname);
				}
			}
			else
			{
				sprintf(oldname,"%s/%s",trapath,it->c_str());
				remove(oldname);
			}
		}
	}
	//if((seq == num && type ==1) || seq < num)
	if(g_config.day == 1)//每月最后一天
	{
		//remove(g_logname[citycode]);
	}
	memset(temp,0,sizeof(temp));
	sprintf(temp,"update result.datarec set value = 1,reason = '指标数据正确' where organ_code = %d and name = '拓扑正确率' and cur_time = to_date(sysdate,'YYYY-MM-DD')",citytype[citycode]);
	
	d5000.d5000_ExecSingle(temp);
	
	if(rmdir(trapath) == -1)
		perror("rmdir:");
	return 0;
}

int mvcim::unzip_fun(char *filename,char *code)
{
	char query[200];
	memset(query,0,sizeof(query));
	sprintf(query,"unzip %s/%s -d %s/All_Model_%s",g_config.cime_path,filename,g_config.cime_path,code);
	system(query);

	mvtocim(code);
	return 0;
}

//遍历线程回调函数
void *process_traversal()
{
	int ret = -1,i = -1,j = -1;
	static int sign = 1;
	char path[256];
	memset(path,0,sizeof(path));
	char timenow[100];
	char error_info[100];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
	sprintf(path,"%s/%s/err",g_config.back_path,timenow);
	if(CreateMultFolder(path) != 0)
		return NULL;
	/*sprintf(path,"%s/%s/TZ",g_config.back_path,timenow);
	if(CreateMultFolder(path) != 0)
		return NULL;*/


	if(tm_temp->tm_hour == 0 && tm_temp->tm_min <= 30) //一小时运行一次，在0点期间运行时，插入此信息
		sign = 0;

#ifdef _DATAREC
	else
		sign = 1;
#endif // _DATAREC



	if(sign == 0)
	{
		int offsetpos = 0,number = 0;
		char query[200];
		memset(query,0x00,sizeof(query));

		//删除当天已有表
		sprintf(query,"delete from EVALUSYSTEM.RESULT.DATAREC where cur_time = to_date(sysdate,'YYYY-MM-DD')");
		d5000.DeleteDBDate(query);

		//int code[] = {35401,35402,35403,35404,35405,35406,35407,35408,35409};
		int acvalue[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		char *index[] = {"配电主站月平均运行率","FA投运率","遥信正确率","挂牌信息一致率","拓扑正确率","终端在线率","遥控成功率","遥控使用率",
			"DMS与用采公专变台账匹配率","DMS公专变采集成功率","配电母线数完整率","配电开关数完整率","配电刀闸数完整率","设备状态一致率","配电站房数完整率",
			"配电变压器完整率","DMS停电信息发布完整率","自动成图率","自动成图数量","停电信息确认率","停电信息发布及时率","停电信息发布时间","FA动作次数","公专变明细","停电故障率","停电故障明细"};

		datarec temp_datarec;
		vector<datarec> vec_datarec;
		vec_datarec.clear();
		for (i = 0;i < vec_organ.size();i++)
		{
			for (j = 0;j < 26;j++)
			{
				memset(&temp_datarec,0,sizeof(datarec));
				temp_datarec.organ_code = vec_organ[i];
				temp_datarec.value = acvalue[j];
				strcpy(temp_datarec.name,index[j]);
				strcpy(temp_datarec.reason,"未提供数据");
				vec_datarec.push_back(temp_datarec);
			}
		}
		//添加模型校验初始化数据
		if(init_modelcheck() != 0)
		{
			printf("模型校验数据初始化失败！");
		}

		struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
		if(attrs == NULL)
			return NULL;
		attrs[0].data_type = DCI_INT;
		attrs[0].data_size = INT_LEN;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_NAME;
		attrs[2].data_type = DCI_INT;
		attrs[2].data_size = INT_LEN;
		attrs[3].data_type = DCI_STR;
		attrs[3].data_size = STR_LEN_NAME;
		attrs[4].data_type = DCI_STR;
		attrs[4].data_size = STR_LEN_SS;


		number = vec_datarec.size();
		char *tmp_datarec = (char *)malloc((size_t)(number*300));
		if(tmp_datarec == NULL)
			return NULL;
		memset(tmp_datarec,0x00,number*300);
		vector<datarec>::iterator it_datarec;
		offsetpos = 0;
		for(it_datarec = vec_datarec.begin();it_datarec != vec_datarec.end();++it_datarec)
		{
			memcpy(tmp_datarec+offsetpos,&(it_datarec->organ_code),INT_LEN);
			offsetpos += INT_LEN;
			memcpy(tmp_datarec+offsetpos,it_datarec->name,STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(tmp_datarec+offsetpos,&(it_datarec->value),INT_LEN);
			offsetpos += INT_LEN;	
			memcpy(tmp_datarec+offsetpos,it_datarec->reason,STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
		}
		strcpy(query,"INSERT INTO EVALUSYSTEM.RESULT.DATAREC(ORGAN_CODE,NAME,VALUE,REASON,CUR_TIME) VALUES(:1,:2,:3,:4,to_date(sysdate,'YYYY-MM-DD'))");
		if(d5000.d5000_WriteData("DATAREC",query,tmp_datarec,number,4,attrs,error_info)!= 0)
		{
			return NULL;
		}
		
		free(tmp_datarec);
		free(attrs);
		attrs = NULL;
		tmp_datarec = NULL;
		sign = 1;
	}

	if(TraversalFolder() != 0) 
	{
		//失败发送"INVA",清空文件序列号
		//enqueue_xml_recv("INVA");
		return NULL;
	}
	//最后发送“END”结束
	//enqueue_xml_recv("END");


	
	printf("insert end\n");
	return NULL;
}

int mvcim::gethavexml()
{
	int i = 0;
	char temp_line[300];
	char temp_name[300];
	char messSplit[2][3][300];
	FILE * filefp = NULL;
	const char *g_logname[] = {"fz.txt","xm.txt","nd.txt","pt.txt","qz.txt","zz.txt","ly.txt","sm.txt","np.txt"};

	for (i = 0;i < 9;i++)
	{
		filefp = fopen(g_logname[i],"r");
		if (filefp == NULL)
		{
			if(errno == 2)
				continue;
			printf("%s:%s\n",g_logname[i],strerror(errno));
			return -1;
		}
		while(!feof(filefp))
		{
			memset(temp_line,0x00,sizeof(temp_line));
			if(fgets(temp_line,sizeof(temp_line),filefp) == NULL)
				break;

			memset(messSplit,0,sizeof(messSplit));
			SeperateStringsFromOneLine(temp_line,messSplit[0]);
			//fprintf(fp,"%s\n",messSplit[0][0]);
			memset(temp_name,0x00,sizeof(temp_name));
			//strcpy(temp_name,GetFileName(temp_line));
			//strcpy(temp_name,temp_line);
			strncpy(temp_name,messSplit[0][0],sizeof(temp_name));
			//temp_name[strlen(temp_name)-1] = 0;
			temp_name[strlen(temp_name)] = 0;
			if(i == 0)
			{
				fz_map.insert(make_pair("35401",temp_name));
				fz_vector.push_back(temp_name);
			}
			else if (i == 1)
			{
				xm_map.insert(make_pair("35402",temp_name));
				xm_vector.push_back(temp_name);
			}
			else if (i == 2)
			{
				nd_map.insert(make_pair("35403",temp_name));
				nd_vector.push_back(temp_name);
			}
			else if (i == 3)
			{
				pt_map.insert(make_pair("35404",temp_name));
				pt_vector.push_back(temp_name);
			}
			else if (i == 4)
			{
				qz_map.insert(make_pair("35405",temp_name));
				qz_vector.push_back(temp_name);
			}
			else if (i == 5)
			{
				zz_map.insert(make_pair("35406",temp_name));
				zz_vector.push_back(temp_name);
			}
			else if (i == 6)
			{
				ly_map.insert(make_pair("35407",temp_name));
				ly_vector.push_back(temp_name);
			}
			else if (i == 7)
			{
				sm_map.insert(make_pair("35408",temp_name));
				sm_vector.push_back(temp_name);
			}
			else if (i == 8)
			{
				np_map.insert(make_pair("35409",temp_name));
				np_vector.push_back(temp_name);
			}
		}
		if(ferror(filefp)!=0)
		{
			fclose(filefp);
			perror("read or write");
			return -1;
		}
		fclose(filefp);
	}

	int citycode;
	char ccitycode[20];
	char qury[100];
	time_t time_temp;
	struct ColAttr *attrs = NULL;
	struct tm *tm_temp;
	char *buf = NULL,*m_presult = NULL;
	char err_info[200],tempstr[300];
	memset(err_info,0,sizeof(err_info));
	memset(qury,0,sizeof(qury));
	time_temp = time(NULL);
	tm_temp = localtime(&time_temp);
	//sprintf(qury,"SELECT ORGAN_CODE,NAME FROM EVALUSYSTEM.CONFIG.HISRECORD WHERE GTIME >= '%d-%d-01';",1900+tm_temp->tm_year,1+tm_temp->tm_mon);
	sprintf(qury,"SELECT ORGAN_CODE,NAME FROM EVALUSYSTEM.CONFIG.HISRECORD WHERE 1=1;");

	int rec_num = 0,attr_num = 0,rec = 0,col = 0,offset = 0;
	if(d5000.d5000_ReadData(qury,&rec_num,&attr_num,&attrs,&buf,err_info) == 0)
	{
		m_presult = buf;
		for (rec = 0; rec < rec_num; rec++)
		{
			citycode = 0;
			for(col = 0; col < attr_num; col++)
			{
				memset(tempstr, 0x00 , 300);
				memcpy(tempstr, m_presult, attrs[col].data_size);
				switch(col)
				{
				case 0:
					citycode = *(int*)(tempstr);
					break;
				case 1:
					memset(messSplit,0,sizeof(messSplit));
					SeperateStringsFromOneLine(tempstr,messSplit[0]);
					memset(temp_name,0x00,sizeof(temp_name));
					strncpy(temp_name,messSplit[0][0],sizeof(temp_name));
					//temp_name[strlen(temp_name)-1] = 0;
					temp_name[strlen(temp_name)] = 0;
					sprintf(ccitycode,"%d",citycode);
					if(strstr(ccitycode,"35401") != NULL)
					{
						fz_map.insert(make_pair(ccitycode,temp_name));
						fz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35402") != NULL)
					{
						xm_map.insert(make_pair(ccitycode,temp_name));
						xm_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35403") != NULL)
					{
						nd_map.insert(make_pair(ccitycode,temp_name));
						nd_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35404") != NULL)
					{
						pt_map.insert(make_pair(ccitycode,temp_name));
						pt_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35405") != NULL)
					{
						qz_map.insert(make_pair(ccitycode,temp_name));
						qz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35406") != NULL)
					{
						zz_map.insert(make_pair(ccitycode,temp_name));
						zz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35407") != NULL)
					{
						ly_map.insert(make_pair(ccitycode,temp_name));
						ly_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35408") != NULL)
					{
						sm_map.insert(make_pair(ccitycode,temp_name));
						sm_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35409") != NULL)
					{
						np_map.insert(make_pair(ccitycode,temp_name));
						np_vector.push_back(temp_name);
					}
					break;
				default:
					break;
				}
				m_presult += attrs[col].data_size;
			}
		}
	}
	else
	{
		return -1;
	}
	if(rec_num > 0)
		d5000.g_CDci.FreeReadData(attrs,attr_num,buf);
	else
		d5000.g_CDci.FreeColAttrData(attrs, attr_num);

	cout << "fz:" << fz_vector.size() << "\txm:" << xm_vector.size() << "\tnd:" << nd_vector.size() << "\npt:" << pt_vector.size()
		<< "\tqz:" << qz_vector.size() << "\tzz:" << zz_vector.size() << "\tly:" << ly_vector.size() << "\tsm:" << sm_vector.size()
		<< "\tnp:" << np_vector.size() << endl;
	return 0;
}

int mvcim::transzip()
{
	int ret = -1,i = 0,len = 0,citycode = 0;
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];
	char file_name[256];
	char oldname[256];
	char newname[256];
	char code[10];
	char *endpos = NULL;

	memset(path_name,0x00,sizeof(path_name));
	char timenow[100];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);

	strcpy(path_name,g_config.cime_path);

	if((dirp = opendir(path_name)) == NULL)
	{
		perror("open dir error!");
		return -1;
	}

	while((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name,".")==0 || strcmp(dirt->d_name,"..") == 0)
			continue; 
		strncpy(file_name,dirt->d_name,sizeof(file_name));
		len = strlen(file_name);
		
		if(strcmp(file_name+len-3,"zip") == 0)
		{
			if(strstr(file_name,"3540") != NULL )
			{
				endpos = strchr(file_name,'_');
				if(endpos != NULL && (endpos-file_name == 5 || endpos-file_name == 7))
				{
					memset(code,0,sizeof(code));
					strncpy(code,file_name,endpos-file_name);
					unzip_fun(file_name,code);
				}
			}
		}
		/*sprintf(oldname,"%s/%s",g_config.cime_path,dirt->d_name);
		sprintf(newname,"%s/%s/%s",g_config.back_path,timenow,dirt->d_name);*/
		sprintf(oldname,"%s/%s",g_config.cime_path,file_name);
		sprintf(newname,"%s/%s/%s",g_config.back_path,timenow,file_name);
		rename(oldname,newname);
		i++;
	}
	closedir(dirp);

	return 0;
}

void *process_cim()
{	
	class mvcim mvcim;

	//g_config.day = getmonday();
	g_config.day = remainderday();
	cout << "days:" << g_config.day << endl;

	if(mvcim.gethavexml() != 0)
		return NULL;

	if(mvcim.transzip() != 0)
		return NULL;
	return NULL;
}

void *xmlLoop()
{
	if(d5000.d5000_Connect() != 0)
		return;
	int ret = 0;
	InitXmlMutex();
	InitXmlThread();
	d5000.d5000_DisConnect();
}

//开始处理xml文件
int resolvexmldata::readdata_xml(const char* file,logstr& threadlog)
{
	const char *xmlfile = file;
	char tempname[250];
	TiXmlDocument doc;

	memset(tempname,0x00,sizeof(tempname));

	sprintf(tempname,"%s/%s",g_config.path_name,xmlfile);

	//tinyxml加载文件
	if (doc.LoadFile(tempname,TIXML_ENCODING_LEGACY))
	{
		SetGError(1,0,xmlfile,-1,"加载成功.",threadlog);
		//SetGError(1,0,tempname,"加载成功.",threadlog);
		Log("Load file %s success.\n" ,xmlfile);
		import_xml(doc,file,threadlog);
	}
	else
	{
		SetGError(2,0,xmlfile,-1,"加载文件失败,文件格式不正确.",threadlog);
		Log("Load file %s fail.\n" ,xmlfile);
		mvxmlfile(xmlfile,1);
	}
	GetGError(xmlfile,threadlog);

	return 0;
}

//有新cime传送，将原文件夹中数据移动到back文件夹
int mvcimefile(char * filename,int type,int sys)
{
	if(filename == NULL)
		return -1;

	int ret = 0;
	char newfile[256];
	char oldfile[256];
	memset(newfile,0x00,sizeof(newfile));
	memset(oldfile,0x00,sizeof(oldfile));

	//将文件移除到保存目录
	if(type == 0 && sys == 0)
		sprintf(newfile,"%s/dms/%s",g_config.back_path,filename);
	else if(type == 0 && sys == 1)
		sprintf(newfile,"%s/gpms/%s",g_config.back_path,filename);
	else if(type == 1)
		sprintf(newfile,"%s/err/%s",g_config.back_path,filename);

	sprintf(oldfile,"%s/%s",g_config.path_name,filename);

	ret = rename(oldfile,newfile);
	if(ret == -1)
	{
		Log("%s mv fail.errno:%s.newfile:%s.\n",filename,strerror(errno),newfile);
		return -1;
	}
	return 0;
}

//开始处理xml/cime、data文件
int resolvexmldata::readdata_file(const char *filename,logstr& threadlog)
{
	if(filename == NULL)
		return -1;
	char full_src_path[256];
	char full_des_path[256];
	char tempname[256];
	static int enumFlag = -1,min_dmscime = -1,max_dmscime = -2,min_dmsdata = -1,max_dmsdata = -2,min_gpmscime = -1,max_gpmscime = -2,min_gpmsdata = -1,max_gpmsdata = -2;
	int i,cimeret = 0;
	time_t time_now;
	struct tm *tm_now;

	enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_DMSCIME_,_GPMSCIME_,_DMSDATA_,_GPMSDATA_,_END_,_INVA_,_UnKnownFileType_}g_filetype;
	const char *g_strfiletypeName[] = {"ZZZX","DATT","DAGC","YXBW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","DMSCIME","GPMSCIME","DMSDATA","GPMSDATA","END","INVA"};

	memset(tempname,0x00,sizeof(tempname));
	strncpy(tempname,filename,sizeof(tempname));

	for (enumFlag=_ZDZX_;enumFlag<_UnKnownFileType_;++enumFlag)
	{
		if(strstr(tempname,g_strfiletypeName[enumFlag])!=NULL)
		{
			g_filetype = (enum filetype)enumFlag;
			break;
		}
	}
	switch(g_filetype)
	{
	case _ZZZX_:
	case _DATT_:
	case _DAGC_:
	case _YXBW_:
	case _DMSGPZW_:
	case _GPMSGPZW_:
	case _ZDZX_:
	case _YKCZ_:
	case _YXINFO_:
		readdata_xml(tempname,threadlog);
		break;
	case _DMSCIME_://存储最大序列号及最小序列号
		i = atoi(strstr(tempname,"-")+1);
		if(min_dmscime < 0)
			min_dmscime = i;
		else
			min_dmscime = (min_dmscime < i)?min_dmscime:i;
		max_dmscime = (max_dmscime < i)?i:max_dmscime;
		break;
	case _GPMSCIME_:
		i = atoi(strstr(tempname,"-")+1);
		if(min_gpmscime < 0)
			min_gpmscime = i;
		else
			min_gpmscime = (min_gpmscime < i)?min_gpmscime:i;
		max_gpmscime = (max_gpmscime < i)?i:max_gpmscime;
		break;
	case _DMSDATA_:
		i = atoi(strstr(tempname,"-")+1);
		if(min_dmsdata < 0)
			min_dmsdata = i;
		else
			min_dmsdata = (min_dmsdata < i)?min_dmsdata:i;
		max_dmsdata = (max_dmsdata < i)?i:max_dmsdata;
		break;
	case _GPMSDATA_:
		i = atoi(strstr(tempname,"-")+1);
		if(min_gpmsdata < 0)
			min_gpmsdata = i;
		else
			min_gpmsdata = (min_gpmsdata < i)?min_gpmsdata:i;
		max_gpmsdata = (max_gpmsdata < i)?i:max_gpmsdata;
		break;
	case _END_:
		time_now = time(NULL);
		tm_now = localtime(&time_now);

		//cout << "max_dmscime:" << max_dmscime << " min_dmscime:" << min_dmscime << " max_dmsdata:" << max_dmsdata << " min_dmsdata:" << min_dmsdata << " max_gpmscime:" << max_gpmscime << " min_gpmscime:" << min_gpmscime << " max_gpmsdata:" << max_gpmsdata << " min_gpmsdata:" << min_gpmsdata << endl;
		
		//判断是否有新cime文件
		/*if(min_dmscime != -1)
			remvcime(1);
		if(min_gpmscime != -1)
			remvcime(2);*/

		for (i=min_dmscime;i<=max_dmscime;++i)
		{
			sprintf(full_des_path,"%d%d%d.DMSCIME",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);
			sprintf(full_src_path,"%d%d%d.DMSCIME-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			if(Combination_cime(full_src_path,full_des_path,1) == 0)
			{
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,0);
			}
			else
			{
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,0);
			}
		}
		for (i=min_dmsdata;i<=max_dmsdata;++i)
		{
			sprintf(full_src_path,"%d%d%d.DMSDATA-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.DMSDATA",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);
			if(Combination_cime(full_src_path,full_des_path,2) == 0)
			{
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,0);
			}
			else
			{
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,0);
			}
		}
		for (i=min_gpmscime;i<=max_gpmscime;++i)
		{
			sprintf(full_src_path,"%d%d%d.GPMSCIME-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.GPMSCIME",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);
			if(Combination_cime(full_src_path,full_des_path,3) == 0)
			{
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,1);
			}
			else
			{
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,1);
			}
		}
		for (i=min_gpmsdata;i<=max_gpmsdata;++i)
		{
			sprintf(full_src_path,"%d%d%d.GPMSDATA-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.GPMSDATA",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);
			if(Combination_cime(full_src_path,full_des_path,4) == 0)
			{
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,1);
			}
			else
			{
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,1);
			}
		}
		min_dmscime = -1;max_dmscime = -2;min_dmsdata = -1;max_dmsdata = -2;min_gpmscime = -1;max_gpmscime = -2;min_gpmsdata = -1;max_gpmsdata = -2;
		break;
	case _INVA_:
		min_dmscime = -1;max_dmscime = -2;min_dmsdata = -1;max_dmsdata = -2;min_gpmscime = -1;max_gpmscime = -2;min_gpmsdata = -1;max_gpmsdata = -2;
		break;
	default:
		cout << "cime file error." << endl;
		break;
	}
}

//合并CIME文件
int resolvexmldata::Combination_cime(const char *src_file,const char *des_file,int flag)
{
	long int read_len = 0,all_read_len = 0,one_read_len = 0,write_len = 0,all_write_len = 0,all_len = 0,lef_len = 0;
	struct stat src_file_stat;
	FILE *fp_read = NULL,*fp_write = NULL;
	char *temp_buf;
	char temp_des_file[250];
	char temp_src_file[250];

	switch(flag)
	{
	case 1:
		sprintf(temp_src_file,"%s/%s",g_config.path_name,src_file);
		sprintf(temp_des_file,"%s/dms/%s",g_config.cime_path,des_file);
		break;
	case 2:
		sprintf(temp_src_file,"%s/%s",g_config.path_name,src_file);
		sprintf(temp_des_file,"%s/%s",g_config.back_path,des_file);
		break;
	case 3:
		sprintf(temp_src_file,"%s/%s",g_config.path_name,src_file);
		sprintf(temp_des_file,"%s/gpms/%s",g_config.cime_path,des_file);
		break;
	case 4:
		sprintf(temp_src_file,"%s/%s",g_config.path_name,src_file);
		sprintf(temp_des_file,"%s/%s",g_config.back_path,des_file);
		break;
	default:
		break;
	}
	
	cout << "temp_src_file:" << temp_src_file << " temp_des_file:" << temp_des_file << endl;
	temp_buf = (char*)malloc(_2M_);
	if (temp_buf == NULL)
	{
		perror("malloc");
		return -1;
	}
	fp_read = fopen(temp_src_file,"r");
	fp_write = fopen(temp_des_file,"a+");
	if (fp_read == NULL || fp_write == NULL)
	{
		free(temp_buf);
		temp_buf = NULL;
		perror("open");
		return -1;
	}
	if(stat(temp_src_file,&src_file_stat) == -1)
	{
		free(temp_buf);
		temp_buf = NULL;
		perror("stat");
		return -1;
	}

	all_len = src_file_stat.st_size;
	while(write_len < all_len)
	{
		memset(temp_buf,0x00,_2M_);
		lef_len = all_len - write_len;
		read_len = (_2M_ < lef_len)?_2M_:lef_len;
		one_read_len = fread(temp_buf,sizeof(char),read_len,fp_read);
		if (one_read_len > 0)
		{
			fflush(fp_read);
			all_read_len += one_read_len;
		}
		else
		{
			free(temp_buf);
			temp_buf = NULL;
			perror("read");
			return -1;
		}
		write_len = fwrite(temp_buf,sizeof(char),one_read_len,fp_write);
		if (write_len > 0)
		{
			fflush(fp_write);
			all_write_len += write_len;
		}
		else
		{
			free(temp_buf);
			temp_buf = NULL;
			perror("write");
			return -1;
		}
	}
	fclose(fp_read);
	fclose(fp_write);
	free(temp_buf);
	temp_buf = NULL;
	if (all_write_len != all_read_len)
		return -1;
	//insertfile(src_file);
	return 0;
}

void *memcpy_struct(const mes_header header_src,mes_header header_des)
{
	header_des.code = header_src.code;
	strcpy(header_des.send,header_src.send);
}

int resolvexmldata::convert(const char *fcode,const char *tcode,const char *inbuf,char *outbuf)
{
	iconv_t ret_open;
	char temp_bom[3];
	size_t inlen,outlen,ret_conv;

	inlen = strlen(inbuf);
	outlen = inlen*4;
	memset(outbuf,0x00,outlen);	
	memset(temp_bom,0x00,sizeof(temp_bom));

	memcpy(temp_bom,inbuf,3);

	if(temp_bom[0] == (-17) && temp_bom[1] == (-69) && temp_bom[2] == (-65))//去bom
		inbuf += 3;

	char *pin = inbuf;
	char *pout = outbuf;
	ret_open = iconv_open("gb18030","utf-8");
	//ret_open = iconv_open(tcode,fcode);

	if(ret_open == (iconv_t)(-1))
		return -1;

	if((ret_conv = iconv(ret_open,&pin,&inlen,&pout,&outlen)) == (size_t)(-1))
	{
		Log("iconv fail.error_no:%d,error_info:%s.\n",errno,strerror(errno));
		iconv_close(ret_open);
		return -1;
	}
	iconv_close(ret_open);
	return 0;
}

int resolvexmldata::convertUTF8toANSI(const char *inbuf,char *outbuf,int outlen)
{
	int ret = 0;
	char *locallang = NULL;
	memset(outbuf,0x00,outlen);

	locallang = getenv("LANG");
	if (locallang == NULL)
		locallang = "GB18030";
	ret = convert("utf-8",locallang,inbuf,outbuf);
	if(ret != 0)
		return -1;
	return 0;
}

//将xml数据存储入内存
int resolvexmldata::import_xml(TiXmlDocument &doc,const char *filename,logstr& threadlog)
{
	int enumFlag,retCode;
	int step_temp = 0;
	int stadagc = 0;// 为dagc向量清空设置变量，只有第一次清空
	char da_temp[30];
	char gtime_temp[30];
	mes_header header;
	header_sign s_header;

	TiXmlElement *requestElement,*headElement;
	TiXmlAttribute *attributeOfhead;

	//enum xmltype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_UnKnownFileType_,_GPZW_DMS_,_GPZW_GPMS_,_LOG_}g_xmltype;
	//const char *g_xmltypeName[] = {"ZZZX","DATT","DAGC","YXBW","GPZW","ZDZX","YKCZ","YXINFO"};
	enum xmltype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_RATE_,_BUS_,_CB_,_DSC_,_TRANS_,_SUBS_,_TDTRANS_,_AUTOMAP_,_TQREALEASE_,_FASTR_,_TZLIST_,_GPMSTD_,_GPMSTDDET_,_GPMSZLPDET_,_DMSZLPDET_,_DMSXSD_,_ZTGJ_,_DMSYD_,_GPMSYD_,_END_,_LOG_,_UnKnownFileType_}g_xmltype;

	const char *g_xmltypeName[] = {"ZZZX","DATT","DAGC","YXBW","GPZW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","dmsindex","buslist","cblist","dsclist","translist","subslist","tdtrans","automap","tdrelease","fastr","tzlist","gpmstd","gpmstddet","gpmszlpdet","zlp","wkwh","ztgj","dmsyd","gpmsydddet","END","LOG"};

	requestElement = doc.RootElement(); //request

	headElement = requestElement->FirstChildElement();//head

	for (;headElement != NULL;headElement = headElement->NextSiblingElement())
	{
		if (strcasecmp(headElement->Value(),"head") == 0)
		{
			memset(&header,0,sizeof(header));
			memset(&s_header,0,sizeof(s_header));
			/*time_t temp_time;
			struct tm *temp_tm;
			temp_time = time(NULL)-86400;
			temp_tm = localtime(&temp_time);
			sprintf(header.count,"%d-%d-%d",1900+temp_tm->tm_year,1+temp_tm->tm_mon,temp_tm->tm_mday);*/

			attributeOfhead = headElement->FirstAttribute();
			for(;attributeOfhead != NULL;attributeOfhead = attributeOfhead->Next())
			{
				//cout << attributeOfhead->Name() << ":" << attributeOfhead->Value() << endl;

				const char *attname = attributeOfhead->Name();
				const char *attvalue = attributeOfhead->Value();

				if(strcasecmp(attname,"code") == 0)
				{
					header.code = atoi(attvalue);
					SetGError(1,header.code,NULL,-1,NULL,threadlog);
				}
				else if(strcasecmp(attname,"source") == 0)
					strcpy(s_header.source,attvalue);
				else if(strcasecmp(attname,"type") == 0)
				{
					strcpy(s_header.type,attvalue);
				}
				else if(strcasecmp(attname,"sequence") == 0)
				{
					strcpy(s_header.sequence,attvalue);
				}
				else if(strcasecmp(attname,"count") == 0)
					strcpy(header.count,attvalue);
				else if(strcasecmp(attname,"send") == 0)
					strcpy(header.send,attvalue);
				else
					cout << "Unknown :" << attname << endl;
			}

		}
		else if (strcasecmp(headElement->Value(),"body") == 0)//body
		{
			for (enumFlag=_ZZZX_;enumFlag<_UnKnownFileType_;enumFlag++)
			{
				//printf("enumFlag:%s\n",g_xmltypeName[enumFlag]);
				//printf("type:%s\n",s_header.type);
				if(strcmp(g_xmltypeName[enumFlag],s_header.type) == 0)
				{
					g_xmltype = (enum xmltype)enumFlag;
					break;
				}
			}

			if(enumFlag == _UnKnownFileType_)
				g_xmltype = (enum xmltype)enumFlag;

			switch(g_xmltype)
			{
			case _ZZZX_:
				//if(strcasecmp(s_header.type,"ZZZX") == 0)
				//{
				dmsop_vector.clear();
				dmsop dmsop_temp;
				TiXmlElement *thrElement_zzzx;
				thrElement_zzzx = headElement->FirstChildElement();
				for (;thrElement_zzzx != NULL;thrElement_zzzx = thrElement_zzzx->NextSiblingElement())
				{
					//cout << thrElement->Value() << ":" << thrElement->GetText() << endl;

					//memcpy_struct(header,dmsop_temp.header);
					memcpy(&dmsop_temp.header,&header,sizeof(header));
					const char * elvalue = thrElement_zzzx->Value();
					const char * eltext = thrElement_zzzx->GetText();

					if(strcasecmp(elvalue,"online") == 0)
						dmsop_temp.online = atoi(eltext);
					else
						cout << "Unknown:" << elvalue << endl;
					dmsop_vector.push_back(dmsop_temp);
				}
				/*if (deleteDBDate(header.count,_ZZZX_) == 0)
				{
					retCode = insertDB(_ZZZX_);
					insertDBLOG(_ZZZX_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else if (strcasecmp(s_header.type,"DATT") == 0)
				//{
			case _DATT_:
				datt_vector.clear();
				datt datt_temp;
				TiXmlElement *thrElement_datt;
				TiXmlAttribute *attributeOfthr_datt;
				thrElement_datt = headElement->FirstChildElement();
				for (;thrElement_datt != NULL;thrElement_datt = thrElement_datt->NextSiblingElement())
				{
					attributeOfthr_datt = thrElement_datt->FirstAttribute();
					for(;attributeOfthr_datt != NULL;attributeOfthr_datt = attributeOfthr_datt->Next())
					{
						//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

						const char *attname = attributeOfthr_datt->Name();
						const char *attvalue = attributeOfthr_datt->Value();
						//memcpy_struct(datt_temp.header,header);
						memcpy(&datt_temp.header,&header,sizeof(header));
						if(strcasecmp(attname,"dv") == 0)
							strncpy(datt_temp.dv,attvalue,sizeof(datt_temp.dv));
						else if(strcasecmp(attname,"gtime") == 0)
							strncpy(datt_temp.gtime,attvalue,sizeof(datt_temp.gtime));
						else if(strcasecmp(attname,"src") == 0)
							datt_temp.src = atoi(attvalue);
						else if(strcasecmp(attname,"type") == 0)
							datt_temp.type = atoi(attvalue);
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}
					datt_vector.push_back(datt_temp);
				}
				/*if(deleteDBDate(header.count,_DATT_) == 0)
				{
					retCode = insertDB(_DATT_);
					insertDBLOG(_DATT_,header.code,retCode,filename);
				}*/
				break;
				//}
				//	else if(strcasecmp(s_header.type,"DAGC") == 0)
				//{
			case _DAGC_:
				if(stadagc == 0)
				{
					dagc_vector.clear();
					dagc_info_vector.clear();
					stadagc++;
				}
				memset(da_temp,0x00,sizeof(da_temp));
				memset(gtime_temp,0x00,sizeof(gtime_temp));
				dagc dagc_temp;
				dagc_info dagc_info_temp;
				TiXmlElement *thrElement_dagc,*fouElenment_dagc;
				TiXmlAttribute *attributeOfthr_dagc,*fouAttrOffou_dagc;
				thrElement_dagc = headElement->FirstChildElement();
				for (;thrElement_dagc != NULL;thrElement_dagc = thrElement_dagc->NextSiblingElement())
				{
					memset(&dagc_temp,0x00,sizeof(dagc_temp));
					attributeOfthr_dagc = thrElement_dagc->FirstAttribute();
					for(;attributeOfthr_dagc != NULL;attributeOfthr_dagc = attributeOfthr_dagc->Next())
					{
						//cout << attributeOfthr_dagc->Name() << ":" << attributeOfthr_dagc->Value() << endl;

						const char *attname = attributeOfthr_dagc->Name();
						const char *attvalue = attributeOfthr_dagc->Value();
						//memcpy_struct(header,dagc_temp.header);
						memcpy(&dagc_temp.header,&header,sizeof(header));
						if(strcasecmp(attname,"fault") == 0)
						{
							strncpy(dagc_temp.da,attvalue,sizeof(dagc_temp.da));
							strncpy(da_temp,attvalue,sizeof(da_temp));
						}
						else if(strcasecmp(attname,"dv") == 0)
							strncpy(dagc_temp.dv,attvalue,sizeof(dagc_temp.dv));
						else if(strcasecmp(attname,"step") == 0)
						{
							dagc_temp.step = atoi(attvalue);
							step_temp = atoi(attvalue);
						}
						else if(strcasecmp(attname,"gtime") == 0)
						{
							strncpy(dagc_temp.gtime,attvalue,sizeof(dagc_temp.gtime));
							strncpy(gtime_temp,attvalue,sizeof(gtime_temp));
						}
						else if(strcasecmp(attname,"result") == 0)
							dagc_temp.result = atoi(attvalue);
						else if(strcasecmp(attname,"cb") == 0)
							strncpy(dagc_temp.cb,attvalue,sizeof(dagc_temp.cb));
						else
							cout << "Unknown :" << attname << endl;
					}
					dagc_vector.push_back(dagc_temp);

					fouElenment_dagc = thrElement_dagc->FirstChildElement();
					for(;fouElenment_dagc != NULL;fouElenment_dagc = fouElenment_dagc->NextSiblingElement())
					{
						fouAttrOffou_dagc = fouElenment_dagc->FirstAttribute();
						for (;fouAttrOffou_dagc != NULL;fouAttrOffou_dagc = fouAttrOffou_dagc->Next())
						{
							//cout << fouAttrOffou_dagc->Name() << ":" << fouAttrOffou_dagc->Value() << endl;

							const char *attname = fouAttrOffou_dagc->Name();
							const char *attvalue = fouAttrOffou_dagc->Value();
							//memcpy_struct(header,dagc_info_temp.header);
							memcpy(&dagc_info_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"cb") == 0)
								strncpy(dagc_info_temp.cb,attvalue,sizeof(dagc_info_temp.cb));
							else if(strcasecmp(attname,"src") == 0)
								dagc_info_temp.src = atoi(attvalue);
							else
								cout << "Unknown:" << attname << endl;

							memcpy(dagc_info_temp.da,da_temp,sizeof(da_temp));
							dagc_info_temp.step = step_temp;
							memcpy(dagc_info_temp.gtime,gtime_temp,sizeof(gtime_temp));
						}
						dagc_info_vector.push_back(dagc_info_temp);
					}
				}
				/*if(deleteDBDate(header.count,_DAGC_) == 0)
				{
					retCode = insertDB(_DAGC_);
					insertDBLOG(_DAGC_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else if(strcasecmp(s_header.type,"YXBW") == 0)
				//{
			case _YXBW_:
				yxbw_vector.clear();
				yxbw yxbw_temp;
				TiXmlElement *thrElement_yxbw;
				TiXmlAttribute *attributeOfthr_yxbw;
				thrElement_yxbw = headElement->FirstChildElement();
				for (;thrElement_yxbw != NULL;thrElement_yxbw = thrElement_yxbw->NextSiblingElement())
				{
					attributeOfthr_yxbw = thrElement_yxbw->FirstAttribute();
					for(;attributeOfthr_yxbw != NULL;attributeOfthr_yxbw = attributeOfthr_yxbw->Next())
					{
						//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

						const char *attname = attributeOfthr_yxbw->Name();
						const char *attvalue = attributeOfthr_yxbw->Value();
						//memcpy_struct(yxbw_temp.header,header);
						memcpy(&yxbw_temp.header,&header,sizeof(header));
						if(strcasecmp(attname,"cb") == 0)
							strncpy(yxbw_temp.cb,attvalue,sizeof(yxbw_temp.cb));
						else if(strcasecmp(attname,"gtime") == 0)
							strncpy(yxbw_temp.gtime,attvalue,sizeof(yxbw_temp.gtime));
						else if(strcasecmp(attname,"src") == 0)
							yxbw_temp.src = atoi(attvalue);
						else if(strcasecmp(attname,"type") == 0)
							yxbw_temp.type = atoi(attvalue);
						else
							cout << "Unknown :" << attvalue << endl;
					}
					yxbw_vector.push_back(yxbw_temp);
				}
				/*if (deleteDBDate(header.count,_YXBW_) == 0)
				{
					retCode = insertDB(_YXBW_);
					insertDBLOG(_YXBW_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else if (strcasecmp(s_header.type,"GPZW") == 0)
				//{
			case _GPZW_:
				/*if (strcasecmp(s_header.source,"DMS") == 0)
				{*/
					dmsbrand_vector.clear();
					dmsstatus_vector.clear();
					dmsbrand dmsbrand_temp;
					dmsstatus dmsstatus_temp;
					TiXmlElement *thrElement_dmsgpzw;
					TiXmlAttribute *attributeOfthr_dmsgpzw;
					thrElement_dmsgpzw = headElement->FirstChildElement();
					for (;thrElement_dmsgpzw != NULL;thrElement_dmsgpzw = thrElement_dmsgpzw->NextSiblingElement())
					{
						attributeOfthr_dmsgpzw = thrElement_dmsgpzw->FirstAttribute();
						for(;attributeOfthr_dmsgpzw != NULL;attributeOfthr_dmsgpzw = attributeOfthr_dmsgpzw->Next())
						{
							//cout << attributeOfthr_dmsgpzw->Name() << ":" << attributeOfthr_dmsgpzw->Value() << endl;

							const char *attname = attributeOfthr_dmsgpzw->Name();
							const char *attvalue = attributeOfthr_dmsgpzw->Value();
							//memcpy_struct(brand_temp.header,header);
							memcpy(&dmsbrand_temp.status.header,&header,sizeof(header));
							if(strcasecmp(attname,"devid") == 0)
								strncpy(dmsbrand_temp.status.devid,attvalue,sizeof(dmsbrand_temp.status));
							else if(strcasecmp(attname,"devtype") == 0)
								strncpy(dmsbrand_temp.status.devtype,attvalue,sizeof(dmsbrand_temp.status.devtype));
							else if(strcasecmp(attname,"gtime") == 0)
								strncpy(dmsbrand_temp.status.gtime,attvalue,sizeof(dmsbrand_temp.status.gtime));
							else if(strcasecmp(attname,"src") == 0)
								dmsbrand_temp.status.src = atoi(attvalue);
							else if(strcasecmp(attname,"info") == 0)
								dmsbrand_temp.info = atoi(attvalue);
							else
								cout << "Unknown :" << attname << endl;
						}
						if (dmsbrand_temp.status.src == 1 || dmsbrand_temp.status.src == 0)
						{
							dmsbrand_vector.push_back(dmsbrand_temp);
						}
						/*else if(dmsbrand_temp.status.src == 2 || dmsbrand_temp.status.src == 3)
						{
							dmsstatus_vector.push_back(dmsbrand_temp.status);
						}*/
					}
					g_xmltype = (enum xmltype)(enumFlag + 1);
					/*if (deleteDBDate(header.count,_DMSGPZW_) == 0)
					{
						retCode = insertDB(_DMSGPZW_);
						insertDBLOG(_DMSGPZW_,header.code,retCode,filename);
					}*/
				//}
				//else if (strcasecmp(s_header.source,"GPMS") == 0)
				//{
				//	gpmsbrand_vector.clear();
				//	gpmsstatus_vector.clear();
				//	dmsbrand gpmsbrand_temp;
				//	dmsstatus gpmsstatus_temp;
				//	TiXmlElement *thrElement_gpmsgpzw;
				//	TiXmlAttribute *attributeOfthr_gpmsgpzw;
				//	thrElement_gpmsgpzw = headElement->FirstChildElement();
				//	for (;thrElement_gpmsgpzw != NULL;thrElement_gpmsgpzw = thrElement_gpmsgpzw->NextSiblingElement())
				//	{
				//		attributeOfthr_gpmsgpzw = thrElement_gpmsgpzw->FirstAttribute();
				//		for(;attributeOfthr_gpmsgpzw != NULL;attributeOfthr_gpmsgpzw = attributeOfthr_gpmsgpzw->Next())
				//		{
				//			//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

				//			const char *attname = attributeOfthr_gpmsgpzw->Name();
				//			const char *attvalue = attributeOfthr_gpmsgpzw->Value();
				//			//memcpy_struct(brand_temp.header,header);
				//			memcpy(&gpmsbrand_temp.status.header,&header,sizeof(header));
				//			if(strcasecmp(attname,"devid") == 0)
				//				strcpy(gpmsbrand_temp.status.devid,attvalue);
				//			else if(strcasecmp(attname,"devtype") == 0)
				//				strcpy(gpmsbrand_temp.status.devtype,attvalue);
				//			else if(strcasecmp(attname,"gtime") == 0)
				//				strcpy(gpmsbrand_temp.status.gtime,attvalue);
				//			else if(strcasecmp(attname,"src") == 0)
				//				gpmsbrand_temp.status.src = atoi(attvalue);
				//			else if(strcasecmp(attname,"info") == 0)
				//				gpmsbrand_temp.info = atoi(attvalue);
				//			else
				//				cout << "Unknown :" << attname << endl;
				//		}
				//		if (gpmsbrand_temp.status.src == 1 || gpmsbrand_temp.status.src == 0)
				//		{
				//			gpmsbrand_vector.push_back(gpmsbrand_temp);
				//		}
				//		else if(gpmsbrand_temp.status.src == 2 || gpmsbrand_temp.status.src == 3)
				//		{
				//			gpmsstatus_vector.push_back(gpmsbrand_temp.status);
				//		}
				//	}
				//	g_xmltype = (enum xmltype)(enumFlag + 2);
				//	/*if(deleteDBDate(header.count,_GPMSGPZW_) == 0)
				//	{
				//		retCode = insertDB(_GPMSGPZW_);
				//		insertDBLOG(_GPMSGPZW_,header.code,retCode,filename);
				//	}*/
				//}
				break;
				//}
				//else if (strcasecmp(s_header.type,"ZDZX") == 0)
				//{
			case _ZDZX_:
				{
					zdzx_vector.clear();
					zdzx_name_vector.clear();
					zdzx zdzx_temp;
					int sign = 0;
					TiXmlElement *thrElement_zdzx;
					TiXmlAttribute * attributeOfthr_zdzx;
					thrElement_zdzx = headElement->FirstChildElement();
					for (;thrElement_zdzx != NULL;thrElement_zdzx = thrElement_zdzx->NextSiblingElement())
					{
						memset(&zdzx_temp,0x00,sizeof(zdzx));

						attributeOfthr_zdzx = thrElement_zdzx->FirstAttribute();
						for(;attributeOfthr_zdzx != NULL;attributeOfthr_zdzx = attributeOfthr_zdzx->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;
							const char *attname = attributeOfthr_zdzx->Name();
							const char *attvalue = attributeOfthr_zdzx->Value();
							//memcpy_struct(zdzx_temp.header,header);
							memcpy(&zdzx_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"link") == 0)
								strncpy(zdzx_temp.link,attvalue,sizeof(zdzx_temp.link));
							else if(strcasecmp(attname,"name") == 0)
							{
								//convertUTF8toANSI(attvalue,zdzx_temp.name,sizeof(zdzx_temp.name));
								if(find(zdzx_name_vector.begin(),zdzx_name_vector.end(),attvalue) == zdzx_name_vector.end())
								{
									strncpy(zdzx_temp.name,attvalue,sizeof(zdzx_temp.name));
									zdzx_name_vector.push_back(attvalue);
								}
								else
									sign = 1;
							}
							else if(strcasecmp(attname,"online") == 0)
							{
								zdzx_temp.online = atoi(attvalue);
							}
							else
								cout << "Unknown :" << attname << endl;
						}
						if(sign == 0)
							zdzx_vector.push_back(zdzx_temp);
						else
							sign = 0;
					}
				}
				/*if (deleteDBDate(header.count,_ZDZX_) == 0)
				{
					retCode = insertDB(_ZDZX_);
					insertDBLOG(_ZDZX_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else if (strcasecmp(s_header.type,"YKCZ") == 0)
				//{
			case _YKCZ_:
				ykcz_vector.clear();
				ykcz ykcz_temp;
				TiXmlElement *thrElement_ykcz;
				TiXmlAttribute *attributeOfthr_ykcz;
				thrElement_ykcz = headElement->FirstChildElement();
				for (;thrElement_ykcz != NULL;thrElement_ykcz = thrElement_ykcz->NextSiblingElement())
				{
					attributeOfthr_ykcz = thrElement_ykcz->FirstAttribute();
					for(;attributeOfthr_ykcz != NULL;attributeOfthr_ykcz = attributeOfthr_ykcz->Next())
					{
						//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

						const char *attname = attributeOfthr_ykcz->Name();
						const char *attvalue = attributeOfthr_ykcz->Value();
						//memcpy_struct(ykcz_temp.header,header);
						memcpy(&ykcz_temp.header,&header,sizeof(header));
						if(strcasecmp(attname,"cb") == 0)
							strncpy(ykcz_temp.cb,attvalue,sizeof(ykcz_temp.cb));
						else if(strcasecmp(attname,"gtime") == 0)
							strncpy(ykcz_temp.gtime,attvalue,sizeof(ykcz_temp.gtime));
						else if(strcasecmp(attname,"src") == 0)
							ykcz_temp.src = atoi(attvalue);
						else if(strcasecmp(attname,"result") == 0)
							ykcz_temp.result = atoi(attvalue);
						else
							cout << "Unknown :" << attname << endl;
					}
					ykcz_vector.push_back(ykcz_temp);
				}
				/*if(deleteDBDate(header.count,_YKCZ_) == 0)
				{
					retCode = insertDB(_YKCZ_);
					insertDBLOG(_YKCZ_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else if (strcasecmp(s_header.type,"YXINFO") == 0)
				//{
			case _YXINFO_:
				yxinfo_vector.clear();
				yxinfo yxinfo_temp;
				TiXmlElement *thrElement_yxinfo;
				thrElement_yxinfo = headElement->FirstChildElement();
				for(;thrElement_yxinfo != NULL;thrElement_yxinfo = thrElement_yxinfo->NextSiblingElement())
				{
					const char * elvalue = thrElement_yxinfo->Value();
					const char * eltext = thrElement_yxinfo->GetText();

					//memcpy_struct(yxinfo_temp.header,header);
					memcpy(&yxinfo_temp.header,&header,sizeof(header));
					if(strcasecmp(elvalue,"total") == 0)
						yxinfo_temp.total = atoi(eltext);
					else if(strcasecmp(elvalue,"matchsoe") == 0)
						yxinfo_temp.matchsoe = atoi(eltext);
					else if(strcasecmp(elvalue,"local") == 0)
						yxinfo_temp.local = atoi(eltext);
					else
						cout << "Unknown :" << elvalue << endl;
				}
				yxinfo_vector.push_back(yxinfo_temp);
				/*if (deleteDBDate(header.count,_YXINFO_) == 0)
				{
					retCode = insertDB(_YXINFO_);
					insertDBLOG(_YXINFO_,header.code,retCode,filename);
				}*/
				break;
				//}
				//else
			case _RATE_:
				{
					rate_vector.clear();
					rate tate_temp;
					float x = 0.0;
					const float float_border = 0.000001;
					tate_temp.xsd_rate = 0.0;
					TiXmlElement *thrElement_rate;
					thrElement_rate = headElement->FirstChildElement();
					for(;thrElement_rate != NULL;thrElement_rate = thrElement_rate->NextSiblingElement())
					{
						const char * elvalue = thrElement_rate->Value();
						const char * eltext = thrElement_rate->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&tate_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"dysim") == 0)
							tate_temp.tq = atof(eltext);
						else if(strcasecmp(elvalue,"datasuc") == 0)
							tate_temp.datasuc = atof(eltext);
						else if(strcasecmp(elvalue,"xsd") == 0)
						{
							char o_code[10];
                            sprintf(o_code,"%d",tate_temp.header.code);
							if(strlen(o_code) == 5)
								tate_temp.xsd_rate = atof(eltext);
						}
						else
							cout << "Unknown :" << elvalue << endl;
					}
					char m_code[10];
                    sprintf(m_code,"%d",tate_temp.header.code);
					if(strlen(m_code) > 5)
						tate_temp.xsd_rate = 1.0;
					
					//校验台账匹配率、采集成功率数值是否为零 add by lcm 20150910
					if(fabs(tate_temp.tq)<=float_border)
					{
						char timenow[20];
						char path_name[100];
						memset(timenow,0x00,sizeof(timenow));
						memset(path_name,0x00,sizeof(path_name));
						time_t time_temp= time(NULL);
						struct tm *tm_temp = localtime(&time_temp);
						strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
						sprintf(path_name,"%s/%s",g_config.back_path,timenow);
						
						int offsetpos = 0;
						char qury[200];
						memset(qury,0x00,sizeof(qury));
						char error_info[200];
						memset(error_info,0x00,sizeof(error_info));
						struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
						if(attrs == NULL)
							return -1;
						attrs[0].data_type = DCI_STR;
						attrs[0].data_size = STR_LEN_NAME;
						attrs[1].data_type = DCI_STR;
						attrs[1].data_size = STR_LEN_NAME;
						attrs[2].data_type = DCI_INT;
						attrs[2].data_size = INT_LEN;
						attrs[3].data_type = DCI_STR;
						attrs[3].data_size = STR_LEN_NAME;
						char *temp_err = (char *)malloc((size_t)(1000));
						if(temp_err == NULL)
							return -1;
						memset(temp_err,0x00,1000);
						
						memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(header.code),INT_LEN);
						offsetpos += INT_LEN;
						memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
		
						sprintf(qury,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'台账匹配率数值为零','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
					if(fabs(tate_temp.datasuc)<=float_border)
					{
						char timenow[20];
						char path_name[100];
						memset(timenow,0x00,sizeof(timenow));
						memset(path_name,0x00,sizeof(path_name));
						time_t time_temp= time(NULL);
						struct tm *tm_temp = localtime(&time_temp);
						strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
						sprintf(path_name,"%s/%s",g_config.back_path,timenow);
						
						int offsetpos = 0;
						char qury[200];
						memset(qury,0x00,sizeof(qury));
						char error_info[200];
						memset(error_info,0x00,sizeof(error_info));
						struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
						if(attrs == NULL)
							return -1;
						attrs[0].data_type = DCI_STR;
						attrs[0].data_size = STR_LEN_NAME;
						attrs[1].data_type = DCI_STR;
						attrs[1].data_size = STR_LEN_NAME;
						attrs[2].data_type = DCI_INT;
						attrs[2].data_size = INT_LEN;
						attrs[3].data_type = DCI_STR;
						attrs[3].data_size = STR_LEN_NAME;
						char *temp_err = (char *)malloc((size_t)(1000));
						if(temp_err == NULL)
							return -1;
						memset(temp_err,0x00,1000);
						
						memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(header.code),INT_LEN);
						offsetpos += INT_LEN;
						memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
		
						sprintf(qury,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'采集成功率数值为零','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
					rate_vector.push_back(tate_temp);
				}
				break;
			case _BUS_:
				{
					bus_vector.clear();
					bus bus_temp;
					TiXmlElement *thrElement_bus;
					TiXmlAttribute *attributeOfthr_bus;
					thrElement_bus = headElement->FirstChildElement();
					for (;thrElement_bus != NULL;thrElement_bus = thrElement_bus->NextSiblingElement())
					{
						memset(&bus_temp,0,sizeof(bus));
						attributeOfthr_bus = thrElement_bus->FirstAttribute();
						for(;attributeOfthr_bus != NULL;attributeOfthr_bus = attributeOfthr_bus->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_bus->Name();
							const char *attvalue = attributeOfthr_bus->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&bus_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"bus") == 0)
								strncpy(bus_temp.busid,attvalue,sizeof(bus_temp.busid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(bus_temp.busname,attvalue,sizeof(bus_temp.busname));
							else if(strcasecmp(attname,"dvid") == 0)
								strncpy(bus_temp.dvid,attvalue,sizeof(bus_temp.dvid));
							else if(strcasecmp(attname,"dvname") == 0)
							{

							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(bus_temp.vol,attvalue,sizeof(bus_temp.vol));
							else if(strcasecmp(attname,"subsid") == 0)
							{

							}
							else if(strcasecmp(attname,"subname") == 0)
							{

							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(bus_temp.vol,"0.38") != 0)
							bus_vector.push_back(bus_temp);
					}
				}
				break;
			case _CB_:
				{
					cb_vetor.clear();
					int status = -1;
					cb cb_temp;
					TiXmlElement *thrElement_cb;
					TiXmlAttribute *attributeOfthr_cb;
					thrElement_cb = headElement->FirstChildElement();
					for (;thrElement_cb != NULL;thrElement_cb = thrElement_cb->NextSiblingElement())
					{
						memset(&cb_temp,0,sizeof(cb));
						cb_temp.type = 1;
						cb_temp.flag = 1;
						attributeOfthr_cb = thrElement_cb->FirstAttribute();
						for(;attributeOfthr_cb != NULL;attributeOfthr_cb = attributeOfthr_cb->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_cb->Name();
							const char *attvalue = attributeOfthr_cb->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&cb_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"cb") == 0)
								strncpy(cb_temp.cbid,attvalue,sizeof(cb_temp.cbid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(cb_temp.cbname,attvalue,sizeof(cb_temp.cbname));
							else if(strcasecmp(attname,"dvid") == 0)
								strncpy(cb_temp.dvid,attvalue,sizeof(cb_temp.dvid));
							else if(strcasecmp(attname,"status") == 0)
							{
								/*status = atoi(attvalue);
								if(status == 10 || status == 40 || status == 42 || status == 43 || status == 12)
									cb_temp.status = 0;
								else if(status == 11 || status == 41)
									cb_temp.status = 1;
								else*/
								status = atoi(attvalue);
								if(status == 10 || status == 50)
									cb_temp.status = 0;
								else if(status == 11 || status == 41)
									cb_temp.status = 1;
								else
									cb_temp.status = atoi(attvalue);
							}
							else if(strcasecmp(attname,"dvname") == 0)
							{

							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(cb_temp.vol,attvalue,sizeof(cb_temp.vol));
							else if(strcasecmp(attname,"subsid") == 0)
							{
								if(strncmp(attvalue+32,"5107",4) == 0)
									cb_temp.flag = 0;
								else
									cb_temp.flag = 1;
							}
							else if(strcasecmp(attname,"subname") == 0)
							{
								
							}
							else if(strcasecmp(attname,"remotetype") == 0)
							{
								cb_temp.remotetype = atoi(attvalue);
							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(cb_temp.vol,"0.38") != 0)
							cb_vetor.push_back(cb_temp);
					}
				}
				break;
			case _DSC_:
				{
					dsc_vector.clear();
					dsc dsc_temp;
					int status = -1;
					TiXmlElement *thrElement_dsc;
					TiXmlAttribute *attributeOfthr_dsc;
					thrElement_dsc = headElement->FirstChildElement();
					for (;thrElement_dsc != NULL;thrElement_dsc = thrElement_dsc->NextSiblingElement())
					{
						memset(&dsc_temp,0,sizeof(dsc));
						dsc_temp.type = 0;
						dsc_temp.flag = 1;
						attributeOfthr_dsc = thrElement_dsc->FirstAttribute();
						for(;attributeOfthr_dsc != NULL;attributeOfthr_dsc = attributeOfthr_dsc->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_dsc->Name();
							const char *attvalue = attributeOfthr_dsc->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&dsc_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"dsc") == 0)
								strncpy(dsc_temp.dscid,attvalue,sizeof(dsc_temp.dscid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(dsc_temp.dscname,attvalue,sizeof(dsc_temp.dscname));
							else if(strcasecmp(attname,"dvid") == 0)
								strncpy(dsc_temp.dvid,attvalue,sizeof(dsc_temp.dvid));
							else if(strcasecmp(attname,"status") == 0)
							{
								status = atoi(attvalue);
								if(status == 10 || status == 50)
									dsc_temp.status = 0;
								else if(status == 11)
									dsc_temp.status = 1;
								else
									dsc_temp.status = atoi(attvalue);
							}
							else if(strcasecmp(attname,"dvname") == 0)
							{

							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(dsc_temp.vol,attvalue,sizeof(dsc_temp.vol));
							else if(strcasecmp(attname,"subsid") == 0)
							{
								if(strncmp(attvalue+32,"5107",4) == 0)
									dsc_temp.flag = 0;
								else
									dsc_temp.flag = 1;
							}
							else if(strcasecmp(attname,"subname") == 0)
							{

							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(dsc_temp.vol,"0.38") != 0)
							dsc_vector.push_back(dsc_temp);
					}
				}
				break;
			case _TRANS_:
				{
					trans_vector.clear();
					trans trans_temp;
					TiXmlElement *thrElement_trans;
					TiXmlAttribute *attributeOfthr_trans;
					thrElement_trans = headElement->FirstChildElement();
					for (;thrElement_trans != NULL;thrElement_trans = thrElement_trans->NextSiblingElement())
					{
						memset(&trans_temp,0,sizeof(trans));
						attributeOfthr_trans = thrElement_trans->FirstAttribute();
						for(;attributeOfthr_trans != NULL;attributeOfthr_trans = attributeOfthr_trans->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_trans->Name();
							const char *attvalue = attributeOfthr_trans->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&trans_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"trans") == 0)
								strncpy(trans_temp.transid,attvalue,sizeof(trans_temp.transid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(trans_temp.transname,attvalue,sizeof(trans_temp.transname));
							else if(strcasecmp(attname,"dvid") == 0)
								strncpy(trans_temp.dvid,attvalue,sizeof(trans_temp.dvid));
							else if(strcasecmp(attname,"dvname") == 0)
							{

							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(trans_temp.vol,attvalue,sizeof(trans_temp.vol));
							else if(strcasecmp(attname,"subsid") == 0)
							{

							}
							else if(strcasecmp(attname,"subname") == 0)
							{

							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(trans_temp.vol,"0.38") != 0)
							trans_vector.push_back(trans_temp);
					}
				}
				break;
			case _SUBS_:
				{
					subs_vetor.clear();
					subs subs_temp;
					TiXmlElement *thrElement_subs;
					TiXmlAttribute *attributeOfthr_subs;
					thrElement_subs = headElement->FirstChildElement();
					for (;thrElement_subs != NULL;thrElement_subs = thrElement_subs->NextSiblingElement())
					{
						memset(&subs_temp,0,sizeof(subs));
						attributeOfthr_subs = thrElement_subs->FirstAttribute();
						for(;attributeOfthr_subs != NULL;attributeOfthr_subs = attributeOfthr_subs->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_subs->Name();
							const char *attvalue = attributeOfthr_subs->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&subs_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"subs") == 0)
								strncpy(subs_temp.subsid,attvalue,sizeof(subs_temp.subsid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(subs_temp.subsname,attvalue,sizeof(subs_temp.subsname));
							else if(strcasecmp(attname,"dvid") == 0)
								strncpy(subs_temp.dvid,attvalue,sizeof(subs_temp.dvid));
							else if(strcasecmp(attname,"dvname") == 0)
							{

							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(subs_temp.vol,attvalue,sizeof(subs_temp.vol));
							else if(strcasecmp(attname,"subsid") == 0)
							{

							}
							else if(strcasecmp(attname,"subname") == 0)
							{

							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(subs_temp.vol,"0.38") != 0)
							subs_vetor.push_back(subs_temp);
					}
				}
				break;
#ifdef _TDTRANS
			case _TDTRANS_:
				{
					tdtrans_vector.clear();
					tdtrans tdtrans_temp;
					TiXmlElement *thrElement_tdtrans;
					thrElement_tdtrans = headElement->FirstChildElement();
					for(;thrElement_tdtrans != NULL;thrElement_tdtrans = thrElement_tdtrans->NextSiblingElement())
					{
						const char * elvalue = thrElement_tdtrans->Value();
						const char * eltext = thrElement_tdtrans->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&tdtrans_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"tdtrans") == 0)
							tdtrans_temp.tdtrans = atoi(eltext);
						else if(strcasecmp(elvalue,"wrongtrans") == 0)
							tdtrans_temp.wrongtrans = atoi(eltext);
						else if(strcasecmp(elvalue,"reltrans") == 0)
							tdtrans_temp.realtrans = atoi(eltext);
						else
							cout << "Unknown :" << elvalue << endl;
					}
					tdtrans_vector.push_back(tdtrans_temp);
				}
				break;
#endif // _TDTRANS
			case _AUTOMAP_:
				{
					automap_vector.clear();
					automap automap_temp;
					memset(&automap_temp,0,sizeof(automap));
					TiXmlElement *thrElement_automap;
					thrElement_automap = headElement->FirstChildElement();
					for(;thrElement_automap != NULL;thrElement_automap = thrElement_automap->NextSiblingElement())
					{
						const char * elvalue = thrElement_automap->Value();
						const char * eltext = thrElement_automap->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&automap_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"alrmap") == 0)
							automap_temp.alrmap = atoi(eltext);
						else if(strcasecmp(elvalue,"shoumap") == 0)
							automap_temp.shoumap = atoi(eltext);
						else
							cout << "Unknown :" << elvalue << endl;
					}
					automap_vector.push_back(automap_temp);
				}
				break;
#ifdef _TQRELEALSE
			case _TQREALEASE_:
				{
					tqrelease_vector.clear();
					tqrelease tqrelease_temp;
					int null_flag = 0;
					TiXmlElement *thrElement_tqrelease;
					TiXmlAttribute *attributeOfthr_tqrelease;
					thrElement_tqrelease = headElement->FirstChildElement();
					for (;thrElement_tqrelease != NULL;thrElement_tqrelease = thrElement_tqrelease->NextSiblingElement())
					{
						memset(&tqrelease_temp,0,sizeof(tqrelease));
						attributeOfthr_tqrelease = thrElement_tqrelease->FirstAttribute();
						int mflag = 0;
						for(;attributeOfthr_tqrelease != NULL;attributeOfthr_tqrelease = attributeOfthr_tqrelease->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_tqrelease->Name();
							const char *attvalue = attributeOfthr_tqrelease->Value();
							
							//memcpy_struct(datt_temp.header,header);
							memcpy(&tqrelease_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"dmstd") == 0)
								strncpy(tqrelease_temp.dmstd,attvalue,sizeof(tqrelease_temp.dmstd));
							else if(strcasecmp(attname,"ddaff") == 0)
								strncpy(tqrelease_temp.ddaff,attvalue,sizeof(tqrelease_temp.ddaff));
							else if(strcasecmp(attname,"dmsaff") == 0)
								strncpy(tqrelease_temp.dmsaff,attvalue,sizeof(tqrelease_temp.dmsaff));
							else if(strcasecmp(attname,"tdnum") == 0)
								tqrelease_temp.tdnum = atoi(attvalue);
							else if(strcasecmp(attname,"tdintr") == 0)
								strncpy(tqrelease_temp.tdintnr,attvalue,sizeof(tqrelease_temp.tdintnr)-1);
							else if(strcasecmp(attname,"endtime") == 0)
								strncpy(tqrelease_temp.judged_time,attvalue,sizeof(tqrelease_temp.judged_time));
							else if(strcasecmp(attname,"type") == 0)
							{
								if(strcmp(attvalue,"0") == 0 || strcmp(attvalue,"1") == 0 || strcmp(attvalue,"2") == 0)
									tqrelease_temp.sendtype = atoi(attvalue);
								if(strcmp(attvalue,"100") == 0)
									mflag = 1;
							}
							else if(strcasecmp(attname,"yptype") == 0)
							{
								if(mflag == 1)
									tqrelease_temp.yptype = 100;
								else
									tqrelease_temp.yptype = atoi(attvalue);
							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if (tqrelease_temp.sendtype == 2)
						{
							strncpy(tqrelease_temp.ddaff,tqrelease_temp.dmstd,sizeof(tqrelease_temp.ddaff));
							strncpy(tqrelease_temp.dmsaff,tqrelease_temp.dmstd,sizeof(tqrelease_temp.dmsaff));
						}
						if(strlen(tqrelease_temp.tdintnr) == 0)
						{
							null_flag = 1;
						}
						tqrelease_vector.push_back(tqrelease_temp);
					}
					//校验dms停电信息文件是否存在停电范围为空记录 add by lcm 20150910
					if(null_flag == 1)
					{
						char timenow[20];
						char path_name[100];
						memset(timenow,0x00,sizeof(timenow));
						memset(path_name,0x00,sizeof(path_name));
						time_t time_temp= time(NULL);
						struct tm *tm_temp = localtime(&time_temp);
						strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
						sprintf(path_name,"%s/%s",g_config.back_path,timenow);
						
						int offsetpos = 0;
						char qury[200];
						memset(qury,0x00,sizeof(qury));
						char error_info[200];
						memset(error_info,0x00,sizeof(error_info));
						struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
						if(attrs == NULL)
							return -1;
						attrs[0].data_type = DCI_STR;
						attrs[0].data_size = STR_LEN_NAME;
						attrs[1].data_type = DCI_STR;
						attrs[1].data_size = STR_LEN_NAME;
						attrs[2].data_type = DCI_INT;
						attrs[2].data_size = INT_LEN;
						attrs[3].data_type = DCI_STR;
						attrs[3].data_size = STR_LEN_NAME;
						char *temp_err = (char *)malloc((size_t)(1000));
						if(temp_err == NULL)
							return -1;
						memset(temp_err,0x00,1000);
						
						memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(header.code),INT_LEN);
						offsetpos += INT_LEN;
						memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
		
						sprintf(qury,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'该文件中有停电范围为空记录','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
				}
				break;
#endif // _TQRELEALSE
#ifdef _FASTR
			case _FASTR_:
				{
					fastr_vector.clear();
					fastr fastr_temp;
					TiXmlElement *thrElement_fastr;
					TiXmlAttribute *attributeOfthr_fastr;
					thrElement_fastr = headElement->FirstChildElement();
					for (;thrElement_fastr != NULL;thrElement_fastr = thrElement_fastr->NextSiblingElement())
					{
						memset(&fastr_temp,0,sizeof(fastr));
						attributeOfthr_fastr = thrElement_fastr->FirstAttribute();
						for(;attributeOfthr_fastr != NULL;attributeOfthr_fastr = attributeOfthr_fastr->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_fastr->Name();
							const char *attvalue = attributeOfthr_fastr->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&fastr_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"num") == 0)
								fastr_temp.num = atoi(attvalue);
							else if(strcasecmp(attname,"name") == 0)
								strncpy(fastr_temp.dvid,attvalue,sizeof(fastr_temp.dvid));
							else if(strcasecmp(attname,"str") == 0)
								strncpy(fastr_temp.str,attvalue,sizeof(fastr_temp.str));
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						fastr_vector.push_back(fastr_temp);
					}
				}
				break;
#endif // _FASTR
			case _TZLIST_:
				{
					tz_vector.clear();
					tzlist_qc_vector.clear();
					tzlist tz_temp;
					TiXmlElement *thrElement_tz;
					TiXmlAttribute *attributeOfthr_tz;
					thrElement_tz = headElement->FirstChildElement();
					for (;thrElement_tz != NULL;thrElement_tz = thrElement_tz->NextSiblingElement())
					{
						memset(&tz_temp,0,sizeof(tzlist));
						attributeOfthr_tz = thrElement_tz->FirstAttribute();
						for(;attributeOfthr_tz != NULL;attributeOfthr_tz = attributeOfthr_tz->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_tz->Name();
							const char *attvalue = attributeOfthr_tz->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&tz_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"tz") == 0)
								strncpy(tz_temp.tz,attvalue,sizeof(tz_temp.tz));
							else if(strcasecmp(attname,"belong") == 0)
								strcpy(tz_temp.belong,attvalue);
							else if(strcasecmp(attname,"id") == 0)
								strncpy(tz_temp.id,attvalue,sizeof(tz_temp.id));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(tz_temp.name,attvalue,sizeof(tz_temp.name));
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if((strcmp(tz_temp.tz," ") != 0) && (strcmp(tz_temp.belong,"dms") != 0) && (find(tzlist_qc_vector.begin(),tzlist_qc_vector.end(),tz_temp.tz) == tzlist_qc_vector.end()))
						{
							tzlist_qc_vector.push_back(tz_temp.tz);
							tz_vector.push_back(tz_temp);
						}
					}
				}
				break;
			case _GPMSTD_:
				{
					gpmstd_vetor.clear();
					gpmstd temp_gpmstd;
					memset(&temp_gpmstd,0x00,sizeof(temp_gpmstd));

					TiXmlElement *thrElement_gpmstd;
					TiXmlAttribute *attributeOfthr_gpmstd;
					thrElement_gpmstd = headElement->FirstChildElement();
					for (;thrElement_gpmstd != NULL;thrElement_gpmstd = thrElement_gpmstd->NextSiblingElement())
					{
						memset(&temp_gpmstd,0,sizeof(gpmstd));
						attributeOfthr_gpmstd = thrElement_gpmstd->FirstAttribute();
						for(;attributeOfthr_gpmstd != NULL;attributeOfthr_gpmstd = attributeOfthr_gpmstd->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_gpmstd->Name();
							const char *attvalue = attributeOfthr_gpmstd->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&temp_gpmstd.header,&header,sizeof(header));
							if(strcasecmp(attname,"organ_code") == 0)
								temp_gpmstd.citycode = atoi(attvalue);
							else if(strcasecmp(attname,"organ_name") == 0)
								strncpy(temp_gpmstd.cityname,attvalue,sizeof(temp_gpmstd.cityname));
							else if(strcasecmp(attname,"num") == 0)
								temp_gpmstd.allnum = atoi(attvalue);
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						gpmstd_vetor.push_back(temp_gpmstd);
					}
				}
				break;
			case _GPMSTDDET_:
				{
					gpmstddet_vector.clear();
					gpmstddet temp_gpmstddet;
					int null_flag = 0;
					memset(&temp_gpmstddet,0x00,sizeof(temp_gpmstddet));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_gpmstddet,0,sizeof(temp_gpmstddet));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&temp_gpmstddet.header,&header,sizeof(header));
							if(strcasecmp(attname,"organ_code") == 0)
								temp_gpmstddet.citycode = atoi(attvalue);
							else if(strcasecmp(attname,"organ_name") == 0)
								strncpy(temp_gpmstddet.cityname,attvalue,sizeof(temp_gpmstddet.cityname));
							else if(strcasecmp(attname,"dmsid") == 0)
								strncpy(temp_gpmstddet.dmsid,attvalue,sizeof(temp_gpmstddet.dmsid));
							else if(strcasecmp(attname,"gpmsid") == 0)
								strncpy(temp_gpmstddet.gpmsid,attvalue,sizeof(temp_gpmstddet.gpmsid));
							else if (strcasecmp(attname,"judtime") == 0)
								strncpy(temp_gpmstddet.judged_time,attvalue,sizeof(temp_gpmstddet.judged_time));
							else if (strcasecmp(attname,"disdesk") == 0)
								strncpy(temp_gpmstddet.disdesk,attvalue,sizeof(temp_gpmstddet.disdesk));
							else if (strcasecmp(attname,"belarea") == 0)
								strncpy(temp_gpmstddet.belarea,attvalue,sizeof(temp_gpmstddet.belarea));
							else if(strcasecmp(attname,"fault_time") == 0)
								strncpy(temp_gpmstddet.fault_time,attvalue,sizeof(temp_gpmstddet.fault_time));
							else if(strcasecmp(attname,"return_time") == 0)
								strncpy(temp_gpmstddet.return_time,attvalue,sizeof(temp_gpmstddet.return_time));
							else if(strcasecmp(attname,"fault_line") == 0)
								strncpy(temp_gpmstddet.fault_line,attvalue,sizeof(temp_gpmstddet.fault_line));
							else if(strcasecmp(attname,"fault_no") == 0)
								strncpy(temp_gpmstddet.fault_no,attvalue,sizeof(temp_gpmstddet.fault_no));
							else if(strcasecmp(attname,"bills_num") == 0)
								strncpy(temp_gpmstddet.bills_num,attvalue,sizeof(temp_gpmstddet.bills_num));
							else if(strcasecmp(attname,"details") == 0)
								strncpy(temp_gpmstddet.details,attvalue,sizeof(temp_gpmstddet.details));
							else if(strcasecmp(attname,"tdintr") == 0)
								strncpy(temp_gpmstddet.tdintr,attvalue,sizeof(temp_gpmstddet.tdintr));
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if(strcasecmp(temp_gpmstddet.disdesk,"DDT_NP_NP") == 0 && strcasecmp(temp_gpmstddet.belarea,"3950A505-7FF2-4AAC-9970-C4E5D4E73796") == 0)
							strncpy(temp_gpmstddet.disdesk,"DDT_NP_YP",sizeof(temp_gpmstddet.disdesk));//延平停电故障由市调度台发出

						multimap<string,disdesk>::iterator it_dis = disdesk_map.find(temp_gpmstddet.disdesk);
						if(it_dis != disdesk_map.end())
						{
							temp_gpmstddet.citycode = it_dis->second.organ_code;
							strncpy(temp_gpmstddet.cityname,it_dis->second.area,sizeof(temp_gpmstddet.cityname));
						}
						if(strlen(temp_gpmstddet.tdintr) == 0)
						{
							null_flag = 1;
						}
						gpmstddet_vector.push_back(temp_gpmstddet);
					}
					//校验gpms停电信息文件是否存在停电范围为空记录 add by lcm 20150910
					if(null_flag == 1)
					{
						char timenow[20];
						char path_name[100];
						memset(timenow,0x00,sizeof(timenow));
						memset(path_name,0x00,sizeof(path_name));
						time_t time_temp= time(NULL);
						struct tm *tm_temp = localtime(&time_temp);
						strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
						sprintf(path_name,"%s/%s",g_config.back_path,timenow);
						
						int offsetpos = 0;
						char qury[200];
						memset(qury,0x00,sizeof(qury));
						char error_info[200];
						memset(error_info,0x00,sizeof(error_info));
						struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
						if(attrs == NULL)
							return -1;
						attrs[0].data_type = DCI_STR;
						attrs[0].data_size = STR_LEN_NAME;
						attrs[1].data_type = DCI_STR;
						attrs[1].data_size = STR_LEN_NAME;
						attrs[2].data_type = DCI_INT;
						attrs[2].data_size = INT_LEN;
						attrs[3].data_type = DCI_STR;
						attrs[3].data_size = STR_LEN_NAME;
						char *temp_err = (char *)malloc((size_t)(1000));
						if(temp_err == NULL)
							return -1;
						memset(temp_err,0x00,1000);
						
						memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
						memcpy(temp_err+offsetpos,&(header.code),INT_LEN);
						offsetpos += INT_LEN;
						memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
						offsetpos += STR_LEN_NAME;
		
						sprintf(qury,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,organ_code,count_file_path,dis,source) values "
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'该文件中有停电范围为空记录','yc');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
				}
				break;
			case _GPMSZLPDET_:
				{
					gpmszlpdet_vector.clear();
					gpmszlpdet temp_gpmszlpdet;
					
					memset(&temp_gpmszlpdet,0x00,sizeof(temp_gpmszlpdet));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_gpmszlpdet,0,sizeof(temp_gpmszlpdet));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&temp_gpmszlpdet.header,&header,sizeof(header));
							if(strcasecmp(attname,"organ_code") == 0)
								temp_gpmszlpdet.citycode = atoi(attvalue);
							else if(strcasecmp(attname,"organ_name") == 0)
								strncpy(temp_gpmszlpdet.cityname,attvalue,sizeof(temp_gpmszlpdet.cityname));
								else if(strcasecmp(attname,"dmsid") == 0)
								strncpy(temp_gpmszlpdet.dmsid,attvalue,sizeof(temp_gpmszlpdet.dmsid));
							else if(strcasecmp(attname,"gpmsid") == 0)
								strncpy(temp_gpmszlpdet.gpmsid,attvalue,sizeof(temp_gpmszlpdet.gpmsid));
							else if(strcasecmp(attname,"type") == 0)
								strncpy(temp_gpmszlpdet.type,attvalue,sizeof(temp_gpmszlpdet.type));
							else if (strcasecmp(attname,"content") == 0)
								strncpy(temp_gpmszlpdet.content,attvalue,sizeof(temp_gpmszlpdet.content));
							else if (strcasecmp(attname,"start_time") == 0)
								strncpy(temp_gpmszlpdet.start_time,attvalue,sizeof(temp_gpmszlpdet.start_time));
							else if (strcasecmp(attname,"order_time") == 0)
								strncpy(temp_gpmszlpdet.order_time,attvalue,sizeof(temp_gpmszlpdet.order_time));
							else if (strcasecmp(attname,"confirm_time") == 0)
								strncpy(temp_gpmszlpdet.confirm_time,attvalue,sizeof(temp_gpmszlpdet.confirm_time));
							else if (strcasecmp(attname,"ctl_name") == 0)
								strncpy(temp_gpmszlpdet.ctl_name,attvalue,sizeof(temp_gpmszlpdet.ctl_name));
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						multimap<string,disdesk>::iterator it_dis = disdesk_map.find(temp_gpmszlpdet.ctl_name);
						if(it_dis != disdesk_map.end())
						{
							temp_gpmszlpdet.citycode = it_dis->second.organ_code;
							strncpy(temp_gpmszlpdet.cityname,it_dis->second.area,sizeof(temp_gpmszlpdet.cityname));
						}
						gpmszlpdet_vector.push_back(temp_gpmszlpdet);
					}
				}
				break;
			case _GPMSYD_:
				{
					gpmsyd_vector.clear();
					gpmsyd temp_gpmsyd;
					
					memset(&temp_gpmsyd,0x00,sizeof(temp_gpmsyd));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_gpmsyd,0,sizeof(temp_gpmsyd));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&temp_gpmsyd.header,&header,sizeof(header));
							if(strcasecmp(attname,"organ_code") == 0)
								temp_gpmsyd.citycode = atoi(attvalue);
							else if(strcasecmp(attname,"organ_name") == 0)
								strncpy(temp_gpmsyd.cityname,attvalue,sizeof(temp_gpmsyd.cityname));
								else if(strcasecmp(attname,"ydd_id") == 0)
								strncpy(temp_gpmsyd.ydd_id,attvalue,sizeof(temp_gpmsyd.ydd_id));
							else if(strcasecmp(attname,"status") == 0)
								strncpy(temp_gpmsyd.status,attvalue,sizeof(temp_gpmsyd.status));
							else if(strcasecmp(attname,"rollback_time") == 0)
								temp_gpmsyd.rollback_time = atoi(attvalue);
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						
						gpmsyd_vector.push_back(temp_gpmsyd);
					}
				}
				break;
			case _DMSZLPDET_:
				{
					dmszlpdet_vector.clear();
					dmszlpdet temp_dmszlpdet;
					
					memset(&temp_dmszlpdet,0x00,sizeof(temp_dmszlpdet));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_dmszlpdet,0,sizeof(temp_dmszlpdet));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&temp_dmszlpdet.header,&header,sizeof(header));
							if(strcasecmp(attname,"id") == 0)
								strncpy(temp_dmszlpdet.dmsid,attvalue,sizeof(temp_dmszlpdet.dmsid));
							else if(strcasecmp(attname,"type") == 0)
								strncpy(temp_dmszlpdet.type,attvalue,sizeof(temp_dmszlpdet.type));
							else if (strcasecmp(attname,"content") == 0)
								strncpy(temp_dmszlpdet.content,attvalue,sizeof(temp_dmszlpdet.content));
							else if (strcasecmp(attname,"start_time") == 0)
								strncpy(temp_dmszlpdet.start_time,attvalue,sizeof(temp_dmszlpdet.start_time));
							else if (strcasecmp(attname,"order_time") == 0)
								strncpy(temp_dmszlpdet.plan_operate_time,attvalue,sizeof(temp_dmszlpdet.plan_operate_time));
							else if (strcasecmp(attname,"confirm_time") == 0)
								cout << "未应用域：confirm_time" << endl;
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						dmszlpdet_vector.push_back(temp_dmszlpdet);
					}
				}
				break;
			case _DMSXSD_:
				{
					dmsxsd_vector.clear();
					dmsxsd temp_dmsxsd;
					
					memset(&temp_dmsxsd,0x00,sizeof(temp_dmsxsd));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_dmsxsd,0,sizeof(temp_dmsxsd));
						memcpy(&temp_dmsxsd.header,&header,sizeof(header));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;
							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							if(strcasecmp(attname,"code") == 0)
								temp_dmsxsd.header.code=atoi(attvalue);
							else if(strcasecmp(attname,"wkwhsum") == 0)
								temp_dmsxsd.value=atof(attvalue)/10;
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						dmsxsd_vector.push_back(temp_dmsxsd);
					}
				}
				break;
			case _ZTGJ_:
				{
					ztgj_vector.clear();
					ztgj temp_ztgj;
					
					memset(&temp_ztgj,0x00,sizeof(temp_ztgj));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_ztgj,0,sizeof(temp_ztgj));
						memcpy(&temp_ztgj.header,&header,sizeof(header));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;
							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							if(strcasecmp(attname,"ztgjvalue") == 0)
								temp_ztgj.value=atof(attvalue);
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						ztgj_vector.push_back(temp_ztgj);
					}
				}
				break;
			case _DMSYD_:
				{
					dmsyd_vector.clear();
					dmsyd temp_dmsyd;
					
					memset(&temp_dmsyd,0x00,sizeof(temp_dmsyd));

					TiXmlElement *thrElement_gpmstddet;
					TiXmlAttribute *attributeOfthr_gpmstddet;
					thrElement_gpmstddet = headElement->FirstChildElement();
					for (;thrElement_gpmstddet != NULL;thrElement_gpmstddet = thrElement_gpmstddet->NextSiblingElement())
					{
						memset(&temp_dmsyd,0,sizeof(temp_dmsyd));
						memcpy(&temp_dmsyd.header,&header,sizeof(header));
						attributeOfthr_gpmstddet = thrElement_gpmstddet->FirstAttribute();
						for(;attributeOfthr_gpmstddet != NULL;attributeOfthr_gpmstddet = attributeOfthr_gpmstddet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;
							const char *attname = attributeOfthr_gpmstddet->Name();
							const char *attvalue = attributeOfthr_gpmstddet->Value();
							//memcpy_struct(datt_temp.header,header);
							if(strcasecmp(attname,"id") == 0)
								strncpy(temp_dmsyd.id,attvalue,sizeof(temp_dmsyd.id));
							else if(strcasecmp(attname,"status") == 0)
								temp_dmsyd.status = atoi(attvalue);
							else if(strcasecmp(attname,"rollback_time") == 0)
								temp_dmsyd.rollback_time = atoi(attvalue);
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						dmsyd_vector.push_back(temp_dmsyd);
					}
				}
				break;
			default:
				cout << "未定义的表信息" << endl;
				break;
			}
		}	
	}
	if (deleteDBDate(header.code,g_xmltype) == 0)
	{
		retCode = insertDB(g_xmltype,threadlog,filename);
		if(g_xmltype == _DMSGPZW_ || g_xmltype == _GPMSGPZW_ || g_xmltype == _DAGC_)
		{
			if((retCode >> 16) ==0 || (retCode << 16) == 0)
			{
				insertfile(filename);
				mvxmlfile(filename,0);
			}
			else
				mvxmlfile(filename,1);
		}
		else if(g_xmltype == _RATE_ || g_xmltype == _TZLIST_)
		{
			if(retCode == 0)
				insertfile(filename);

			mvxmlfile(filename,2);
		}
		else
		{
			if(retCode == 0)
			{
				insertfile(filename);
				mvxmlfile(filename,0);
			}
			else
				mvxmlfile(filename,1);
		}
	}
	FreeSize();
}

void resolvexmldata::mem_init()
{
	dmsop_vector.reserve(g_config.max_record);
	datt_vector.reserve(g_config.max_record);
	dagc_vector.reserve(g_config.max_record);
	dagc_info_vector.reserve(g_config.max_record);
	yxbw_vector.reserve(g_config.max_record);
	dmsbrand_vector.reserve(g_config.max_record);
	dmsstatus_vector.reserve(g_config.max_record);
	gpmsbrand_vector.reserve(g_config.max_record);
	gpmsbrand_vector.reserve(g_config.max_record);
	zdzx_vector.reserve(g_config.max_record);
	ykcz_vector.reserve(g_config.max_record);
	yxinfo_vector.reserve(g_config.max_record);
}

//数据库插入数据
int resolvexmldata::insertDB(int arg,logstr& threadlog,const char *filename)
{
	unsigned int ret = 0,mask = 0,errmask = 1;
	char error_info[100];
	memset(error_info,0x00,sizeof(error_info));
	//enum xmltype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_UnKnownFileType_,_GPZW_DMS_,_GPZW_GPMS_}g_xmltype;
	enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_RATE_,_BUS_,_CB_,_DSC_,_TRANS_,_SUBS_,_TDTRANS_,_AUTOMAP_,_TQRELEASE_,_FASTR_,_TZLIST_,_GPMSTD_,_GPMSTDDET_,_GPMSZLPDET_,_DMSZLPDET_,_DMSXSD_,_ZTGJ_,_DMSYD_,_GPMSYD_,_END_,_UnKnownFileType_}g_filetype;
	const char *g_strfiletypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","YCRATE","DMSBUS","DMSCB","DMSCB","DMSLD","DMSST","DMSTD","AUTOGRAPH","TDINFO","FACTION","TZDETAILS","GPMSTD","GPMSTDDET","GPMS_ZLP","DMS_ZLP","DMS_XSD","DMS_ZTGJ","DMS_YD","GPMS_YD","END"};
	const char *g_indexname[] = {"配电主站月平均运行率","FA投运率","遥信正确率","挂牌信息一致率","拓扑正确率","终端在线率","遥控成功率","遥控使用率","遥信正确率","DMS与用采公专变台账匹配率",
	"DMS公专变采集成功率","配电母线数完整率","配电开关数完整率","配电刀闸数完整率","设备状态一致率","配电站房数完整率","配电变压器完整率","DMS停电信息发布完整率","自动成图率","停电信息确认率","停电信息发布及时率",
	"停电信息发布时间","FA动作次数","台账不匹配明细"};

	switch(arg)
	{
	case _ZZZX_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(dmsop_vector.size() == 0)
			{
				SetGError(0,0,NULL,_ZZZX_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_INT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_DD;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_INT;
			attrs[5].data_size = INT_LEN;

			number = dmsop_vector.size();
			char *temp_dmsop = (char *)malloc((size_t)(number*100));
			if(temp_dmsop == NULL)
				break;
			memset(temp_dmsop,0x00,number*100);
			vector<dmsop>::iterator it_dmsop;
			for(it_dmsop = dmsop_vector.begin();it_dmsop != dmsop_vector.end();++it_dmsop)
			{
				memcpy(temp_dmsop+offsetpos,&(it_dmsop->header.code),INT_LEN);
				offsetpos += sizeof(int);
				memcpy(temp_dmsop+offsetpos,&(it_dmsop->online),INT_LEN);
				offsetpos += sizeof(int);
				memcpy(temp_dmsop+offsetpos,it_dmsop->header.count,strlen(it_dmsop->header.count));
				offsetpos += strlen(it_dmsop->header.count);	
				memcpy(temp_dmsop+offsetpos,it_dmsop->header.send,strlen(it_dmsop->header.send));
				offsetpos += strlen(it_dmsop->header.send);
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSOP(ORGAN_CODE,ONLINE,COUNT_TIME,SEND_TIME) VALUES(:1,:2,to_date(:3,'YYYY-MM-DD'),to_date(:4,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSOP",query,temp_dmsop,number,4,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_ZZZX_,error_info,threadlog);
				Log("Insert DMSOP fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_ZZZX_,"入库成功.",threadlog);
			}

			free(temp_dmsop);
			free(attrs);
			attrs = NULL;
			temp_dmsop = NULL;
			break;
		}
	case _DATT_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(datt_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DATT_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = datt_vector.size();
			char *temp_datt = (char *)malloc((size_t)(number*200));
			if(temp_datt == NULL)
				break;
			memset(temp_datt,0x00,number*200);
			vector<datt>::iterator it_datt;
			for(it_datt = datt_vector.begin();it_datt != datt_vector.end();++it_datt)
			{
				memcpy(temp_datt+offsetpos,&(it_datt->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_datt+offsetpos,it_datt->dv,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_datt+offsetpos,&(it_datt->type),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_datt+offsetpos,it_datt->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_datt+offsetpos,&(it_datt->src),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_datt+offsetpos,it_datt->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DATT(ORGAN_CODE,DV,TYPE,GTIME,SRC,SEND_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DATT",query,temp_datt,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DATT_,error_info,threadlog);
				Log("Insert DATT fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DATT_,"入库成功.",threadlog);
			}

			free(temp_datt);
			free(attrs);
			attrs = NULL;
			temp_datt = NULL;
			break;
		}
		/*case _DAGC_:
		{
		int offsetpos = 0,number = 0;
		char query[200];
		memset(query,0x00,sizeof(query));

		if(dagc_vector.size() != 0)
		{
		struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*8);
		attrs[0].data_type = DCI_INT;
		attrs[0].data_size = INT_LEN;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_CHAR;
		attrs[2].data_type = DCI_STR;
		attrs[2].data_size = STR_LEN_NAME;
		attrs[3].data_type = DCI_INT;
		attrs[3].data_size = INT_LEN;
		attrs[4].data_type = DCI_STR;
		attrs[4].data_size = STR_LEN_SS;
		attrs[5].data_type = DCI_INT;
		attrs[5].data_size = INT_LEN;
		attrs[6].data_type = DCI_STR;
		attrs[6].data_size = STR_LEN_NAME;
		attrs[7].data_type = DCI_STR;
		attrs[7].data_size = STR_LEN_SS;

		number = dagc_vector.size();
		char *temp_dagc = (char *)malloc((size_t)(number*400));
		memset(temp_dagc,0x00,number*400);
		vector<dagc>::iterator it_dagc;
		for(it_dagc = dagc_vector.begin();it_dagc != dagc_vector.end();++it_dagc)
		{
		memcpy(temp_dagc+offsetpos,&(it_dagc->header.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc+offsetpos,it_dagc->da,STR_LEN_CHAR);
		offsetpos += STR_LEN_CHAR;
		memcpy(temp_dagc+offsetpos,it_dagc->dv,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_dagc+offsetpos,&(it_dagc->step),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc+offsetpos,it_dagc->gtime,STR_LEN_SS);
		offsetpos += STR_LEN_SS;
		memcpy(temp_dagc+offsetpos,&(it_dagc->result),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc+offsetpos,it_dagc->cb,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_dagc+offsetpos,it_dagc->header.send,STR_LEN_SS);
		offsetpos += STR_LEN_SS;
		}
		strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DAGC(ORGAN_CODE,DA,DV,STEP,GTIME,RESULT,CB,SEND_TIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),:6,:7,to_date(:8,'YYYY-MM-DD HH24:MI:SS'))");
		if(d5000.d5000_WriteData("DAGC",query,temp_dagc,number,8,attrs,error_info) != 0)
		{
		SetGError(2,0,g_strfiletypeName[arg],NULL,error_info,threadlog);
		Log("Insert DAGC fail.\n");
		ret = ret | errmask;
		}
		else
		{
		SetGError(1,0,g_strfiletypeName[arg],NULL,"入库成功.",threadlog);
		}

		free(temp_dagc);
		free(attrs);
		attrs = NULL;
		temp_dagc = NULL;
		}

		memset(query,0x00,sizeof(query));
		offsetpos = 0;

		if(dagc_info_vector.size() != 0)
		{
		struct ColAttr* attrs2 = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
		attrs2[0].data_type = DCI_INT;
		attrs2[0].data_size = INT_LEN;
		attrs2[1].data_type = DCI_STR;
		attrs2[1].data_size = STR_LEN_CHAR;
		attrs2[2].data_type = DCI_INT;
		attrs2[2].data_size = INT_LEN;
		attrs2[3].data_type = DCI_STR;
		attrs2[3].data_size = STR_LEN_NAME;
		attrs2[4].data_type = DCI_INT;
		attrs2[4].data_size = INT_LEN;
		attrs2[5].data_type = DCI_STR;
		attrs2[5].data_size = STR_LEN_SS;

		number = dagc_info_vector.size();
		char *temp_dagc_info = (char *)malloc((size_t)(number*250));
		memset(temp_dagc_info,0x00,number*150);
		vector<dagc_info>::iterator it_dagc_info;
		for(it_dagc_info = dagc_info_vector.begin();it_dagc_info != dagc_info_vector.end();++it_dagc_info)
		{
		memcpy(temp_dagc_info+offsetpos,&(it_dagc_info->header.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc_info+offsetpos,it_dagc_info->da,STR_LEN_CHAR);
		offsetpos += STR_LEN_CHAR;
		memcpy(temp_dagc_info+offsetpos,&(it_dagc_info->step),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc_info+offsetpos,it_dagc_info->cb,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_dagc_info+offsetpos,&(it_dagc_info->src),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_dagc_info+offsetpos,it_dagc_info->header.send,STR_LEN_SS);
		offsetpos += STR_LEN_SS;
		}
		strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DAGC_INFO(ORGAN_CODE,DA,STEP,CB,SRC,SEND_TIME) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
		if(d5000.d5000_WriteData("DAGC_INFO",query,temp_dagc_info,number,6,attrs2,error_info) != 0)
		{
		SetGError(2,0,g_strfiletypeName[arg+1],NULL,error_info,threadlog);
		Log("Insert DAGC_INFO fail.\n");
		ret = ret | (errmask << 16);
		}
		else
		{
		SetGError(1,0,g_strfiletypeName[arg+1],NULL,"入库成功.",threadlog);
		}

		free(temp_dagc_info);
		free(attrs2);
		attrs2 = NULL;
		temp_dagc_info = NULL;
		}
		break;
		}*/
	case _YXBW_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(yxbw_vector.size() == 0)
			{
				SetGError(0,0,NULL,_YXBW_-1,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = yxbw_vector.size();
			char *temp_yxbw = (char *)malloc((size_t)(number*200));
			if(temp_yxbw == NULL)
				break;
			memset(temp_yxbw,0x00,number*200);
			vector<yxbw>::iterator it_yxbw;
			for(it_yxbw = yxbw_vector.begin();it_yxbw != yxbw_vector.end();++it_yxbw)
			{
				memcpy(temp_yxbw+offsetpos,&(it_yxbw->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxbw+offsetpos,it_yxbw->cb,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_yxbw+offsetpos,it_yxbw->gtime,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_yxbw+offsetpos,&(it_yxbw->src),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxbw+offsetpos,&(it_yxbw->type),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxbw+offsetpos,it_yxbw->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YXBW(ORGAN_CODE,CB,GTIME,SRC,TYPE,SEND_TIME) VALUES(:1,:2,to_date(:3,'YYYY-MM-DD HH24:MI:SS'),:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("YXBW",query,temp_yxbw,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_YXBW_-1,error_info,threadlog);
				Log("Insert YXBW fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_YXBW_-1,"入库成功.",threadlog);
			}

			free(temp_yxbw);
			free(attrs);
			attrs = NULL;
			temp_yxbw = NULL;
			break;
		}
	case _DMSGPZW_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(dmsbrand_vector.size() != 0)
			{
				struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
				if(attrs == NULL)
					break;
				attrs[0].data_type = DCI_INT;
				attrs[0].data_size = INT_LEN;
				attrs[1].data_type = DCI_STR;
				attrs[1].data_size = STR_LEN_NAME;
				attrs[2].data_type = DCI_STR;
				attrs[2].data_size = STR_LEN_TYPE;
				attrs[3].data_type = DCI_STR;
				attrs[3].data_size = STR_LEN_SS;
				attrs[4].data_type = DCI_INT;
				attrs[4].data_size = INT_LEN;
				attrs[5].data_type = DCI_INT;
				attrs[5].data_size = INT_LEN;

				number = dmsbrand_vector.size();
				char *temp_dmsbrand = (char *)malloc((size_t)(number*200));
				if(temp_dmsbrand == NULL)
					break;
				memset(temp_dmsbrand,0x00,number*200);
				vector<dmsbrand>::iterator it_dmsbrand;
				for(it_dmsbrand = dmsbrand_vector.begin();it_dmsbrand != dmsbrand_vector.end();++it_dmsbrand)
				{
					memcpy(temp_dmsbrand+offsetpos,&(it_dmsbrand->status.header.code),INT_LEN);
					offsetpos += INT_LEN;
					memcpy(temp_dmsbrand+offsetpos,it_dmsbrand->status.devid,STR_LEN_NAME);
					offsetpos += STR_LEN_NAME;
					memcpy(temp_dmsbrand+offsetpos,it_dmsbrand->status.devtype,STR_LEN_TYPE);
					offsetpos += STR_LEN_TYPE;
					memcpy(temp_dmsbrand+offsetpos,it_dmsbrand->status.gtime,STR_LEN_SS);
					offsetpos += STR_LEN_SS;
					memcpy(temp_dmsbrand+offsetpos,&(it_dmsbrand->status.src),INT_LEN);
					offsetpos += INT_LEN;
					memcpy(temp_dmsbrand+offsetpos,&(it_dmsbrand->info),INT_LEN);
					offsetpos += INT_LEN;
				}
				strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSBRAND(ORGAN_CODE,DEVID,DEVTYPE,GTIME,SRC,INFO) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5,:6)");
				if(d5000.d5000_WriteData("DMSBRAND",query,temp_dmsbrand,number,6,attrs,error_info) != 0)
				{
					SetGError(2,0,g_strfiletypeName[arg],_DMSGPZW_-2,error_info,threadlog);
					Log("Insert DMSBRAND fail.\n");
					ret = ret | errmask;
				}
				else
				{
					SetGError(1,0,g_strfiletypeName[arg],_DMSGPZW_-2,"入库成功.",threadlog);
				}

				free(temp_dmsbrand);
				free(attrs);
				attrs = NULL;
				temp_dmsbrand = NULL;
			}
			else
			{
				SetGError(0,0,NULL,_DMSGPZW_-2,"文件内容为空",threadlog);
			}

			/*memset(query,0x00,sizeof(query));
			offsetpos = 0;

			if(dmsstatus_vector.size() != 0)
			{
				struct ColAttr* attrs2 = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
				attrs2[0].data_type = DCI_INT;
				attrs2[0].data_size = INT_LEN;
				attrs2[1].data_type = DCI_STR;
				attrs2[1].data_size = STR_LEN_NAME;
				attrs2[2].data_type = DCI_STR;
				attrs2[2].data_size = STR_LEN_TYPE;
				attrs2[3].data_type = DCI_STR;
				attrs2[3].data_size = STR_LEN_SS;
				attrs2[4].data_type = DCI_INT;
				attrs2[4].data_size = INT_LEN;

				number = dmsstatus_vector.size();
				char *temp_dmsstatus = (char *)malloc((size_t)(number*200));
				memset(temp_dmsstatus,0x00,number*200);
				vector<dmsstatus>::iterator it_dmsstatus;
				for(it_dmsstatus = dmsstatus_vector.begin();it_dmsstatus != dmsstatus_vector.end();++it_dmsstatus)
				{
					memcpy(temp_dmsstatus+offsetpos,&(it_dmsstatus->header.code),INT_LEN);
					offsetpos += INT_LEN;
					memcpy(temp_dmsstatus+offsetpos,it_dmsstatus->devid,STR_LEN_NAME);
					offsetpos += STR_LEN_NAME;
					memcpy(temp_dmsstatus+offsetpos,it_dmsstatus->devtype,STR_LEN_TYPE);
					offsetpos += STR_LEN_TYPE;
					memcpy(temp_dmsstatus+offsetpos,it_dmsstatus->gtime,STR_LEN_SS);
					offsetpos += STR_LEN_SS;
					memcpy(temp_dmsstatus+offsetpos,&(it_dmsstatus->src),INT_LEN);
					offsetpos += INT_LEN;
				}
				strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSSTATUS(ORGAN_CODE,DEVID,DEVTYPE,GTIME,SRC) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5)");
				if(d5000.d5000_WriteData("DMSSTATUS",query,temp_dmsstatus,number,5,attrs2,error_info) != 0)
				{
					SetGError(2,0,g_strfiletypeName[arg+1],g_indexname[arg-2],error_info,threadlog);
					Log("Insert DMSSTATUS fail.\n");
					ret = ret | (errmask << 16);
				}
				else
				{
					SetGError(1,0,g_strfiletypeName[arg+1],g_indexname[arg-2],"入库成功.",threadlog);
				}

				free(temp_dmsstatus);
				free(attrs2);
				attrs2 = NULL;
				temp_dmsstatus = NULL;
			}*/
			break;
		}
		/*case _GPMSGPZW_:
		{
		int offsetpos = 0,number = 0;
		char query[200];
		memset(query,0x00,sizeof(query));

		if(gpmsbrand_vector.size() != 0)
		{
		struct ColAttr* attrs3 = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
		attrs3[0].data_type = DCI_INT;
		attrs3[0].data_size = INT_LEN;
		attrs3[1].data_type = DCI_STR;
		attrs3[1].data_size = STR_LEN_NAME;
		attrs3[2].data_type = DCI_STR;
		attrs3[2].data_size = STR_LEN_TYPE;
		attrs3[3].data_type = DCI_STR;
		attrs3[3].data_size = STR_LEN_SS;
		attrs3[4].data_type = DCI_INT;
		attrs3[4].data_size = INT_LEN;
		attrs3[5].data_type = DCI_INT;
		attrs3[5].data_size = INT_LEN;

		number = gpmsbrand_vector.size();
		char *temp_gpmsbrand = (char *)malloc((size_t)(number*200));
		memset(temp_gpmsbrand,0x00,number*100);
		vector<dmsbrand>::iterator it_gpmsbrand;
		for(it_gpmsbrand = gpmsbrand_vector.begin();it_gpmsbrand != gpmsbrand_vector.end();++it_gpmsbrand)
		{
		memcpy(temp_gpmsbrand+offsetpos,&(it_gpmsbrand->status.header.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_gpmsbrand+offsetpos,it_gpmsbrand->status.devid,STR_LEN_NAME);
		offsetpos += STR_LEN_NAME;
		memcpy(temp_gpmsbrand+offsetpos,it_gpmsbrand->status.devtype,STR_LEN_TYPE);
		offsetpos += STR_LEN_TYPE;
		memcpy(temp_gpmsbrand+offsetpos,it_gpmsbrand->status.gtime,STR_LEN_SS);
		offsetpos += STR_LEN_SS;
		memcpy(temp_gpmsbrand+offsetpos,&(it_gpmsbrand->status.src),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_gpmsbrand+offsetpos,&(it_gpmsbrand->info),INT_LEN);
		offsetpos += INT_LEN;
		}
		strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMSBRAND(ORGAN_CODE,DEVID,DEVTYPE,GTIME,SRC,INFO) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5,:6)");
		if(d5000.d5000_WriteData("GPMSBRAND",query,temp_gpmsbrand,number,6,attrs3,error_info) != 0)
		{
		SetGError(2,0,g_strfiletypeName[arg+1],NULL,error_info,threadlog);
		Log("Insert GPMSBRAND fail.query:%s\n",query);
		ret = ret | errmask;
		}
		else
		{
		SetGError(1,0,g_strfiletypeName[arg+1],NULL,"入库成功.",threadlog);
		}

		free(temp_gpmsbrand);
		free(attrs3);
		attrs3 = NULL;
		temp_gpmsbrand = NULL;

		}

		memset(query,0x00,sizeof(query));
		offsetpos = 0;

		if(gpmsstatus_vector.size() != 0)
		{
		struct ColAttr* attrs4 = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
		attrs4[0].data_type = DCI_INT;
		attrs4[0].data_size = INT_LEN;
		attrs4[1].data_type = DCI_STR;
		attrs4[1].data_size = STR_LEN_CHAR;
		attrs4[2].data_type = DCI_STR;
		attrs4[2].data_size = STR_LEN_TYPE;
		attrs4[3].data_type = DCI_STR;
		attrs4[3].data_size = STR_LEN_SS;
		attrs4[4].data_type = DCI_INT;
		attrs4[4].data_size = INT_LEN;

		number = gpmsstatus_vector.size();
		char *temp_gpmsstatus = (char *)malloc((size_t)(number*100));
		memset(temp_gpmsstatus,0x00,number*100);
		vector<dmsstatus>::iterator it_gpmsstatus;
		for(it_gpmsstatus = gpmsstatus_vector.begin();it_gpmsstatus != gpmsstatus_vector.end();++it_gpmsstatus)
		{
		memcpy(temp_gpmsstatus+offsetpos,&(it_gpmsstatus->header.code),INT_LEN);
		offsetpos += INT_LEN;
		memcpy(temp_gpmsstatus+offsetpos,it_gpmsstatus->devid,STR_LEN_CHAR);
		offsetpos += STR_LEN_CHAR;
		memcpy(temp_gpmsstatus+offsetpos,it_gpmsstatus->devtype,STR_LEN_TYPE);
		offsetpos += STR_LEN_CHAR;
		memcpy(temp_gpmsstatus+offsetpos,it_gpmsstatus->gtime,STR_LEN_SS);
		offsetpos += STR_LEN_SS;
		memcpy(temp_gpmsstatus+offsetpos,&(it_gpmsstatus->src),INT_LEN);
		offsetpos += INT_LEN;
		}
		strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMSSTATUS(ORGAN_CODE,DEVID,DEVTYPE,GTIME,SRC) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5)");
		if(d5000.d5000_WriteData("GPMSSTATUS",query,temp_gpmsstatus,number,5,attrs4,error_info) != 0)
		{
		SetGError(2,0,g_strfiletypeName[arg+2],NULL,error_info,threadlog);
		Log("Insert GPMSSTATUS fail.\n");
		ret = ret | (errmask << 16);
		}
		else
		{
		SetGError(1,0,g_strfiletypeName[arg+2],NULL,"入库成功.",threadlog);
		}

		free(temp_gpmsstatus);
		free(attrs4);
		attrs4 = NULL;
		temp_gpmsstatus = NULL;

		}
		break;
		}*/
	case _ZDZX_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(zdzx_vector.size() == 0)
			{
				SetGError(0,0,NULL,_ZDZX_-2,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_SS;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_CHAR;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = zdzx_vector.size();
			char *temp_zdzx = (char *)malloc((size_t)(number*400));
			if(temp_zdzx == NULL)
				break;
			memset(temp_zdzx,0x00,number*400);
			vector<zdzx>::iterator it_zdzx;
			for(it_zdzx = zdzx_vector.begin();it_zdzx != zdzx_vector.end();++it_zdzx)
			{
				memcpy(temp_zdzx+offsetpos,&(it_zdzx->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_zdzx+offsetpos,it_zdzx->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_zdzx+offsetpos,it_zdzx->link,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_zdzx+offsetpos,it_zdzx->name,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_zdzx+offsetpos,&(it_zdzx->online),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_zdzx+offsetpos,&(it_zdzx->header.send),STR_LEN_SS);
				offsetpos += STR_LEN_SS;


				/*单行入库，可检查具体哪一条数据出错*/
				/*printf("send:%s\n",it_zdzx->header.send);
				memset(temp_zdzx,0x00,sizeof(temp_zdzx));
				offsetpos = 0;
				memcpy(temp_zdzx,&(it_zdzx->header.code),INT_LEN);
				offsetpos += INT_LEN;
				printf("code:%d,",it_zdzx->header.code);
				memcpy(temp_zdzx+offsetpos,it_zdzx->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				printf("count:%s,",it_zdzx->header.count);
				memcpy(temp_zdzx+offsetpos,it_zdzx->link,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				printf("link:%s,",it_zdzx->link);
				memcpy(temp_zdzx+offsetpos,it_zdzx->name,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				printf("name:%s\n",it_zdzx->name);
				memcpy(temp_zdzx+offsetpos,&(it_zdzx->online),INT_LEN);
				offsetpos += INT_LEN;
				printf("online:%d,",it_zdzx->online);
				memcpy(temp_zdzx+offsetpos,it_zdzx->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				printf("send:%s\n",it_zdzx->header.send);
				strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.ZDZX(ORGAN_CODE,COUNT_TIME,RNUM,NAME,ONLINE,SEND_TIME) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
				if(d5000.d5000_WriteData("ZDZX",query,temp_zdzx,1,6,attrs,error_info) != 0)
				{
				printf("name:%s\n",it_zdzx->name);
				Log("Insert ZDZX fail.\n");
				ret = ret | errmask;
				}*/
			}
			//具体执行
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.ZDZX(ORGAN_CODE,COUNT_TIME,RNUM,NAME,ONLINE,SEND_TIME) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("ZDZX",query,temp_zdzx,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_ZDZX_-2,error_info,threadlog);
				Log("Insert ZDZX fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_ZDZX_-2,"入库成功.",threadlog);
			}

			free(temp_zdzx);
			free(attrs);
			attrs = NULL;
			temp_zdzx = NULL;
			break;
		}
	case _YKCZ_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(ykcz_vector.size() == 0)
			{
				SetGError(0,0,NULL,_YKCZ_-2,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = ykcz_vector.size();
			char *temp_ykcz = (char *)malloc((size_t)(number*200));
			if(temp_ykcz == NULL)
				break;
			memset(temp_ykcz,0x00,number*200);
			vector<ykcz>::iterator it_ykcz;
			for(it_ykcz = ykcz_vector.begin();it_ykcz != ykcz_vector.end();++it_ykcz)
			{
				memcpy(temp_ykcz+offsetpos,&(it_ykcz->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ykcz+offsetpos,it_ykcz->cb,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_ykcz+offsetpos,it_ykcz->gtime,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_ykcz+offsetpos,&(it_ykcz->src),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ykcz+offsetpos,&(it_ykcz->result),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ykcz+offsetpos,&(it_ykcz->header.send),STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YKCZ(ORGAN_CODE,CB,GTIME,SRC,RESULT,SEND_TIME) VALUES(:1,:2,to_date(:3,'YYYY-MM-DD HH24:MI:SS'),:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("YKCZ",query,temp_ykcz,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_YKCZ_-2,error_info,threadlog);
				Log("Insert YKCZ fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_YKCZ_-2,"入库成功.",threadlog);
			}

			free(temp_ykcz);
			free(attrs);
			attrs = NULL;
			temp_ykcz = NULL;
			break;
		}
	case _YXINFO_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(yxinfo_vector.size() == 0)
			{
				SetGError(0,0,NULL,_YXINFO_-2,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_SS;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = yxinfo_vector.size();
			char *temp_yxinfo = (char *)malloc((size_t)(number*100));
			if(temp_yxinfo == NULL)
				break;
			memset(temp_yxinfo,0x00,number*100);
			vector<yxinfo>::iterator it_yxinfo;
			for(it_yxinfo = yxinfo_vector.begin();it_yxinfo != yxinfo_vector.end();++it_yxinfo)
			{
				memcpy(temp_yxinfo+offsetpos,&(it_yxinfo->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxinfo+offsetpos,it_yxinfo->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_yxinfo+offsetpos,&(it_yxinfo->total),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxinfo+offsetpos,&(it_yxinfo->matchsoe),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxinfo+offsetpos,&(it_yxinfo->local),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_yxinfo+offsetpos,&(it_yxinfo->header.send),STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YXINFO(ORGAN_CODE,COUNT_TIME,TOTAL,MATCHSOE,LOCAL,SEND_TIME) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("YXINFO",query,temp_yxinfo,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_YXINFO_-2,error_info,threadlog);
				Log("Insert YXINFO fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_YXINFO_-2,"入库成功.",threadlog);
			}

			free(temp_yxinfo);
			free(attrs);
			attrs = NULL;
			temp_yxinfo = NULL;
			break;
		}
	case _RATE_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[ycrate]:" <<  rate_vector.size() << endl;
			if(rate_vector.size() == 0)
			{
				SetGError(0,0,NULL,_RATE_-2,"文件内容为空",threadlog);
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_FLT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_FLT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = rate_vector.size();
			char *temp_rate = (char *)malloc((size_t)(number*100));
			if(temp_rate == NULL)
				break;
			memset(temp_rate,0x00,number*100);
			vector<rate>::iterator it_rate;
			for(it_rate = rate_vector.begin();it_rate != rate_vector.end();++it_rate)
			{
				memcpy(temp_rate+offsetpos,&(it_rate->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_rate+offsetpos,&(*(int*)&it_rate->tq),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_rate+offsetpos,&(*(int*)&it_rate->datasuc),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_rate+offsetpos,&(*(int*)&it_rate->xsd_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_rate+offsetpos,it_rate->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YCRATE(ORGAN_CODE,TZ,CJ,XSD_RATE,COUNT_TIME) VALUES(:1,round(:2,4),round(:3,4),round(:4,4),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("YCRATE",query,temp_rate,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_RATE_-2,error_info,threadlog);
				Log("Insert Rate fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_RATE_-2,"入库成功.",threadlog);
			}

			free(temp_rate);
			free(attrs);
			attrs = NULL;
			temp_rate = NULL;
		}
		break;
	case _BUS_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[bus]:%d\n",filename,bus_vector.size());
			if(bus_vector.size() == 0)
			{
				SetGError(0,0,NULL,_BUS_-1,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = bus_vector.size();
			char *temp_bus = (char *)malloc((size_t)(number*500));
			if(temp_bus == NULL)
				break;
			memset(temp_bus,0x00,number*500);
			vector<bus>::iterator it_bus;
			for(it_bus = bus_vector.begin();it_bus != bus_vector.end();++it_bus)
			{
				memcpy(temp_bus+offsetpos,&(it_bus->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_bus+offsetpos,&(it_bus->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_bus+offsetpos,&(it_bus->busid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_bus+offsetpos,&(it_bus->busname),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_bus+offsetpos,it_bus->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSBUS(ORGAN_CODE,DV,DEVID,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSBUS_IMAGE",query,temp_bus,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_BUS_-1,error_info,threadlog);
				Log("Insert bus fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_BUS_-1,"入库成功.",threadlog);
			}

			free(temp_bus);
			free(attrs);
			attrs = NULL;
			temp_bus = NULL;
		}
		break;
	case _CB_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[cb]:%d\n",filename,cb_vetor.size());

			if(cb_vetor.size() == 0)
			{
				SetGError(0,0,NULL,_CB_-1,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*9);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_INT;
			attrs[6].data_size = INT_LEN;
			attrs[7].data_type = DCI_INT;
			attrs[7].data_size = INT_LEN;
			attrs[8].data_type = DCI_INT;
			attrs[8].data_size = INT_LEN;
			
			number = cb_vetor.size();
			char *temp_cb = (char *)malloc((size_t)(number*500));
			if(temp_cb == NULL)
				break;
			memset(temp_cb,0x00,number*500);
			vector<cb>::iterator it_cb;
			for(it_cb = cb_vetor.begin();it_cb != cb_vetor.end();++it_cb)
			{
				memcpy(temp_cb+offsetpos,&(it_cb->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_cb+offsetpos,&(it_cb->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_cb+offsetpos,&(it_cb->cbid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_cb+offsetpos,&(it_cb->cbname),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_cb+offsetpos,&(it_cb->type),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_cb+offsetpos,it_cb->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_cb+offsetpos,&(it_cb->status),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_cb+offsetpos,&(it_cb->flag),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_cb+offsetpos,&(it_cb->remotetype),INT_LEN);
				offsetpos += INT_LEN;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSCB(ORGAN_CODE,DV,CB,NAME,TYPE,GTIME,STATUS,FLAG,REMOTETYPE) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7,:8,:9)");
			if(d5000.d5000_WriteData("DMSCBL_IMAGE",query,temp_cb,number,9,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_CB_-1,error_info,threadlog);
				Log("Insert cb fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_CB_-1,"入库成功.",threadlog);
			}

			free(temp_cb);
			free(attrs);
			attrs = NULL;
			temp_cb = NULL;
		}
		break;
	case _DSC_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[dsc]:%d\n",filename,dsc_vector.size());

			if(dsc_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DSC_-1,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*8);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_INT;
			attrs[6].data_size = INT_LEN;
			attrs[7].data_type = DCI_INT;
			attrs[7].data_size = INT_LEN;

			number = dsc_vector.size();
			char *temp_dsc = (char *)malloc((size_t)(number*500));
			if(temp_dsc == NULL)
				break;
			memset(temp_dsc,0x00,number*500);
			vector<dsc>::iterator it_dsc;
			for(it_dsc = dsc_vector.begin();it_dsc != dsc_vector.end();++it_dsc)
			{
				memcpy(temp_dsc+offsetpos,&(it_dsc->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dsc+offsetpos,&(it_dsc->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_dsc+offsetpos,&(it_dsc->dscid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_dsc+offsetpos,&(it_dsc->dscname),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_dsc+offsetpos,&(it_dsc->type),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dsc+offsetpos,it_dsc->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dsc+offsetpos,&(it_dsc->status),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dsc+offsetpos,&(it_dsc->flag),INT_LEN);
				offsetpos += INT_LEN;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSCB(ORGAN_CODE,DV,CB,NAME,TYPE,GTIME,STATUS,FLAG) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7,:8)");
			if(d5000.d5000_WriteData("DMSCBL",query,temp_dsc,number,8,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DSC_-1,error_info,threadlog);
				Log("Insert cb fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DSC_-1,"入库成功.",threadlog);
			}

			free(temp_dsc);
			free(attrs);
			attrs = NULL;
			temp_dsc = NULL;
		}
		break;
	case _TRANS_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[trans]:%d\n",filename,trans_vector.size());

			if(trans_vector.size() == 0)
			{
				SetGError(0,0,NULL,_TRANS_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = trans_vector.size();
			char *temp_trans = (char *)malloc((size_t)(number*500));
			if(temp_trans == NULL)
				break;
			memset(temp_trans,0x00,number*500);
			vector<trans>::iterator it_trans;
			for(it_trans = trans_vector.begin();it_trans != trans_vector.end();++it_trans)
			{
				memcpy(temp_trans+offsetpos,&(it_trans->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_trans+offsetpos,&(it_trans->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_trans+offsetpos,&(it_trans->transid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_trans+offsetpos,&(it_trans->transname),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_trans+offsetpos,it_trans->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSLD(ORGAN_CODE,DV,LD,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSLDL",query,temp_trans,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TRANS_,error_info,threadlog);
				Log("Insert ld fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TRANS_,"入库成功.",threadlog);
			}

			free(temp_trans);
			free(attrs);
			attrs = NULL;
			temp_trans = NULL;
		}
		break;
	case _SUBS_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[subs]:%d\n",filename,subs_vetor.size());

			if(subs_vetor.size() == 0)
			{
				SetGError(0,0,NULL,_SUBS_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = subs_vetor.size();
			char *temp_subs = (char *)malloc((size_t)(number*500));
			if(temp_subs == NULL)
				break;
			memset(temp_subs,0x00,number*500);
			vector<subs>::iterator it_subs;
			for(it_subs = subs_vetor.begin();it_subs != subs_vetor.end();++it_subs)
			{
				memcpy(temp_subs+offsetpos,&(it_subs->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_subs+offsetpos,&(it_subs->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_subs+offsetpos,&(it_subs->subsid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_subs+offsetpos,&(it_subs->subsname),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_subs+offsetpos,it_subs->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSST(ORGAN_CODE,DV,DEVID,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSST",query,temp_subs,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_SUBS_,error_info,threadlog);
				Log("Insert st fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_SUBS_,"入库成功.",threadlog);
			}

			free(temp_subs);
			free(attrs);
			attrs = NULL;
			temp_subs = NULL;
		}
		break;
#ifdef _TDTRANS
	case _TDTRANS_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(tdtrans_vector.size() == 0)
			{
				SetGError(0,0,NULL,_TDTRANS_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_INT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = tdtrans_vector.size();
			char *temp_tdtrans = (char *)malloc((size_t)(number*100));
			if(temp_tdtrans == NULL)
				break;
			memset(temp_tdtrans,0x00,number*100);
			vector<tdtrans>::iterator it_tdtrans;
			for(it_tdtrans = tdtrans_vector.begin();it_tdtrans != tdtrans_vector.end();++it_tdtrans)
			{
				memcpy(temp_tdtrans+offsetpos,&(it_tdtrans->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tdtrans+offsetpos,&(it_tdtrans->tdtrans),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tdtrans+offsetpos,&(it_tdtrans->wrongtrans),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tdtrans+offsetpos,&(it_tdtrans->realtrans),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tdtrans+offsetpos,it_tdtrans->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tdtrans+offsetpos,it_tdtrans->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSTD(ORGAN_CODE,TDTRANS,WRONGTRANS,RELTRANS,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSTD",query,temp_tdtrans,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TDTRANS_,error_info,threadlog);
				Log("Insert dmstd fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TDTRANS_,"入库成功.",threadlog);
			}

			free(temp_tdtrans);
			free(attrs);
			attrs = NULL;
			temp_tdtrans = NULL;
		}
		break;
#endif // _TDTRANS
	case _AUTOMAP_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(automap_vector.size() == 0)
			{
				SetGError(0,0,NULL,_AUTOMAP_,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_INT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = automap_vector.size();
			char *temp_automap = (char *)malloc((size_t)(number*100));
			if(temp_automap == NULL)
				break;
			memset(temp_automap,0x00,number*100);
			vector<automap>::iterator it_automap;
			for(it_automap = automap_vector.begin();it_automap != automap_vector.end();++it_automap)
			{
				memcpy(temp_automap+offsetpos,&(it_automap->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_automap+offsetpos,&(it_automap->shoumap),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_automap+offsetpos,&(it_automap->alrmap),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_automap+offsetpos,it_automap->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_automap+offsetpos,it_automap->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.AUTOGRAPH(ORGAN_CODE,RNUM,SNUM,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			//strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.AUTOGRAPH(ORGAN_CODE,COUNT_TIME,SEND_TIME) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),to_date(:3,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("AUTOGRAPH",query,temp_automap,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_AUTOMAP_,error_info,threadlog);
				Log("Insert automap fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_AUTOMAP_,"入库成功.",threadlog);
			}

			free(temp_automap);
			free(attrs);
			attrs = NULL;
			temp_automap = NULL;
		}
		break;
#ifdef _TQRELEALSE
	case _TQRELEASE_:
		{
			int offsetpos = 0,number = 0;
			char query[20600];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[td]:%d\n",filename,tqrelease_vector.size());

			if(tqrelease_vector.size() == 0)
			{
				SetGError(0,0,NULL,_TQRELEASE_+1,"文件内容为空",threadlog);
				break;
			}
			/*sprintf(sql,"DELETE FROM DETAIL.TDINFO WHERE organ_code = %d and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')",threadlog.code);
			
			if(d5000.DeleteDBDate(sql) != 0)
			{
				Log("delete TDINFO fail.\n");
				ret = ret | errmask;
			}
			else
			{
				Log("delete TDINFO success.\n");
			}
			printf("%s\n",sql);*/

			/*struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*8);
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_INT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_LONGNAME;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;
			attrs[7].data_type = DCI_STR;
			attrs[7].data_size = STR_LEN_SS;

			number = tqrelease_vector.size();
			char *temp_tqrelease = (char *)malloc((size_t)(number*10600));
			memset(temp_tqrelease,0x00,number*10600);*/
			vector<tqrelease>::iterator it_tqrelease;
			pthread_mutex_lock(&text_tqrelease_mutex);
			for(it_tqrelease = tqrelease_vector.begin();it_tqrelease != tqrelease_vector.end();++it_tqrelease)
			{
				/*memcpy(temp_tqrelease+offsetpos,&(it_tqrelease->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tqrelease+offsetpos,&(it_tqrelease->tdnum),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->dmstd,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->ddaff,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->dmsaff,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->tdintnr,STR_LEN_LONGNAME);
				offsetpos += STR_LEN_LONGNAME;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tqrelease+offsetpos,it_tqrelease->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;*/
				sprintf(query,"INSERT INTO EVALUSYSTEM.DETAIL.TDINFO(ORGAN_CODE,TDNUM,DMSTD,DDAFF,DMSFF,TDINTR,COUNT_TIME,SEND_TIME,SENDTYPE,JUDGED_TIME,JUDGED_TYPE) VALUES(%d,%d,to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),'%s',to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),%d,to_date('%s','YYYY-MM-DD HH24:MI:SS'),%d)",it_tqrelease->header.code,it_tqrelease->tdnum,it_tqrelease->dmstd,it_tqrelease->ddaff,it_tqrelease->dmsaff,it_tqrelease->tdintnr,it_tqrelease->header.count,it_tqrelease->header.send,it_tqrelease->sendtype,it_tqrelease->judged_time,it_tqrelease->yptype);

				if(d5000.d5000_ExecSingle(query) != 0)
				{
					//SetGError(2,0,g_strfiletypeName[arg],_TQRELEASE_+1,error_info,threadlog);
					Log("Insert tdinfo fail.\n");
					ret = ret | errmask;
				}
			}
			//strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.TDINFO(ORGAN_CODE,TDNUM,DMSTD,DDAFF,DMSFF,TDINFO,COUNT_TIME,SEND_TIME) VALUES(:1,:2,to_date(:3,'YYYY-MM-DD HH24:MI:SS'),to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'),:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'))");
			//pthread_mutex_lock(&text_tqrelease_mutex);
			/*if(d5000.d5000_WriteData("TDINFO",query,temp_tqrelease,number,8,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TQRELEASE_+1,error_info,threadlog);
				Log("Insert tdinfo fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TQRELEASE_+1,"入库成功.",threadlog);
			}*/
			pthread_mutex_unlock(&text_tqrelease_mutex);
			if(ret != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TQRELEASE_+1,error_info,threadlog);
				Log("Insert tdinfo fail.\n");
				//ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TQRELEASE_+1,"入库成功.",threadlog);
			}
			/*free(temp_tqrelease);
			free(attrs);
			attrs = NULL;
			temp_tqrelease = NULL;*/
		}
		break;
#endif // _TQRELEALSE
#ifdef _FASTR
	case _FASTR_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(fastr_vector.size() == 0)
			{
				SetGError(0,0,NULL,_FASTR_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_LONGNAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = fastr_vector.size();
			char *temp_fastr = (char *)malloc((size_t)(number*10500));
			if(temp_fastr == NULL)
				break;
			memset(temp_fastr,0x00,number*10500);
			vector<fastr>::iterator it_fastr;
			for(it_fastr = fastr_vector.begin();it_fastr != fastr_vector.end();++it_fastr)
			{
				memcpy(temp_fastr+offsetpos,&(it_fastr->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_fastr+offsetpos,&(it_fastr->dvid),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_fastr+offsetpos,&(it_fastr->num),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_fastr+offsetpos,&(it_fastr->str),STR_LEN_LONGNAME);
				offsetpos += STR_LEN_LONGNAME;
				memcpy(temp_fastr+offsetpos,&(it_fastr->header.count),STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_fastr+offsetpos,it_fastr->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.FACTION(ORGAN_CODE,DVNAME,FATIMES,INFO,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");
			pthread_mutex_lock(&text_FA_mutex);
			if(d5000.d5000_WriteData("FACTION",query,temp_fastr,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_FASTR_+3,error_info,threadlog);
				Log("Insert fades fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_FASTR_+3,"入库成功.",threadlog);
			}
			pthread_mutex_unlock(&text_FA_mutex);
			free(temp_fastr);
			free(attrs);
			attrs = NULL;
			temp_fastr = NULL;
		}
		break;
#endif // _FASTR
	case _TZLIST_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(tz_vector.size() == 0)
			{
				SetGError(0,0,NULL,_TZLIST_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*7);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_NAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_ZDNAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;
	
			number = tz_vector.size();
			char *temp_tz = (char *)malloc((size_t)(number*700));
			if(temp_tz == NULL)
				break;
			memset(temp_tz,0x00,number*700);
			vector<tzlist>::iterator it_tz;
			for(it_tz = tz_vector.begin();it_tz != tz_vector.end();++it_tz)
			{
				memcpy(temp_tz+offsetpos,&(it_tz->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_tz+offsetpos,&(it_tz->tz),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_tz+offsetpos,it_tz->id,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_tz+offsetpos,it_tz->name,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_tz+offsetpos,it_tz->belong,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tz+offsetpos,&(it_tz->header.count),STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_tz+offsetpos,it_tz->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.TZDETAILS(ORGAN_CODE,UNMATCHEDID,TRANSID,TRANSNAME,INFO,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),to_date(:7,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("TZDETAILS",query,temp_tz,number,7,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TZLIST_+3,error_info,threadlog);
				Log("Insert fades fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TZLIST_+3,"入库成功.",threadlog);
			}

			free(temp_tz);
			free(attrs);
			attrs = NULL;
			temp_tz = NULL;
		}
		break;
	case _GPMSTD_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			printf("%s:[gpmstd]:%d\n",filename,gpmstd_vetor.size());

			if(gpmstd_vetor.size() == 0)
			{
				SetGError(0,0,NULL,_GPMSTD_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_INT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;


			number = gpmstd_vetor.size();
			char *temp_gpmstd = (char *)malloc((size_t)(number*300));
			if(temp_gpmstd == NULL)
				break;
			memset(temp_gpmstd,0x00,number*300);
			vector<gpmstd>::iterator it_gpmstd;
			for(it_gpmstd = gpmstd_vetor.begin();it_gpmstd != gpmstd_vetor.end();++it_gpmstd)
			{
				memcpy(temp_gpmstd+offsetpos,&(it_gpmstd->citycode),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmstd+offsetpos,&(it_gpmstd->allnum),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmstd+offsetpos,it_gpmstd->cityname,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_gpmstd+offsetpos,it_gpmstd->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstd+offsetpos,it_gpmstd->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_POWERCUT(ORGAN_CODE,NUM,ORGAN_NAME,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("GPMS_POWERCUT",query,temp_gpmstd,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSTD_+3,error_info,threadlog);
				Log("Insert fades fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSTD_+3,"入库成功.",threadlog);
			}

			free(temp_gpmstd);
			free(attrs);
			attrs = NULL;
			temp_gpmstd = NULL;
		}
		break;
	case _GPMSTDDET_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[gpmstddet]:%d\n",filename,gpmstddet_vector.size());

			if(gpmstddet_vector.size() == 0)
			{
				SetGError(0,0,NULL,_GPMSTDDET_+3,"文件内容为空",threadlog);
				
				char timenow[20];
				char path_name[100];
				memset(timenow,0x00,sizeof(timenow));
				memset(path_name,0x00,sizeof(path_name));
				time_t time_temp= time(NULL);
				struct tm *tm_temp = localtime(&time_temp);
				strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
				sprintf(path_name,"%s/%s",g_config.back_path,timenow);
				
				int offsetpos = 0;
				char qury[200];
				memset(qury,0x00,sizeof(qury));
				char error_info[200];
				memset(error_info,0x00,sizeof(error_info));
				struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*3);
				if(attrs == NULL)
					return -1;
				attrs[0].data_type = DCI_STR;
				attrs[0].data_size = STR_LEN_NAME;
				attrs[1].data_type = DCI_STR;
				attrs[1].data_size = STR_LEN_NAME;
				attrs[2].data_type = DCI_STR;
				attrs[2].data_size = STR_LEN_NAME;
				char *temp_err = (char *)malloc((size_t)(1000));
				if(temp_err == NULL)
					return -1;
				memset(temp_err,0x00,1000);
						
				memcpy(temp_err+offsetpos,filename,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_err+offsetpos,&(path_name),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_err+offsetpos,&(g_config.path_name),STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
		
				sprintf(qury,"insert into evalusystem.manage.err_file(file_name,err_file_path,date,count_file_path,dis,source) values "
					"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,'该文件内容为空','yc');");
				if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,3,attrs,error_info) != 0)
				{
					return -1;
				}

				free(temp_err);
				free(attrs);
				attrs = NULL;
				temp_err = NULL;
				
				break;
			}
			sprintf(sql,"DELETE FROM DETAIL.GPMS_POWERCUT_DET WHERE COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')");
			
			if(d5000.DeleteDBDate(sql) != 0)
			{
				Log("delete GPMS_POWERCUT_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				Log("delete GPMS_POWERCUT_DET success.\n");
			}
			printf("%s\n",sql);
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*16);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_ZDNAME;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_CHAR;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_CHAR;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_ZDNAME;
			attrs[7].data_type = DCI_STR;
			attrs[7].data_size = STR_LEN_SS;
			attrs[8].data_type = DCI_STR;
			attrs[8].data_size = STR_LEN_SS;
			attrs[9].data_type = DCI_STR;
			attrs[9].data_size = STR_LEN_CHAR;
			attrs[10].data_type = DCI_STR;
			attrs[10].data_size = STR_LEN_CHAR;
			attrs[11].data_type = DCI_STR;
			attrs[11].data_size = STR_LEN_ZDNAME;
			/*attrs[12].data_type = DCI_STR;
			attrs[12].data_size = STR_LEN_SS;
			attrs[13].data_type = DCI_STR;
			attrs[13].data_size = STR_LEN_CHAR;*/
			attrs[12].data_type = DCI_STR;
			attrs[12].data_size = STR_LEN_DETAILS;
			attrs[13].data_type = DCI_STR;
			attrs[13].data_size = STR_LEN_CHAR;
			attrs[14].data_type = DCI_STR;
			attrs[14].data_size = STR_LEN_SS;
			attrs[15].data_type = DCI_STR;
			attrs[15].data_size = STR_LEN_CHAR;

			/*attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_SS;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_CHAR;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_DETAILS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_CHAR;*/


			number = gpmstddet_vector.size();
			char *temp_gpmstddet = (char *)malloc((size_t)(number*10000));
			if(temp_gpmstddet == NULL)
				break;
			memset(temp_gpmstddet,0x00,number*10000);
			vector<gpmstddet>::iterator it_gpmstddet;
			for(it_gpmstddet = gpmstddet_vector.begin();it_gpmstddet != gpmstddet_vector.end();++it_gpmstddet)
			{
				memcpy(temp_gpmstddet+offsetpos,&(it_gpmstddet->citycode),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->cityname,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->dmsid,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->gpmsid,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->fault_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->return_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->fault_line,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->fault_no,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->bills_num,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->details,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				/*memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->disdesk,STR_LEN_CHAR);
				cout << "disdesk:" << it_gpmstddet->disdesk << endl;
				offsetpos += STR_LEN_CHAR;*/
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->tdintr,STR_LEN_DETAILS);
				offsetpos += STR_LEN_DETAILS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->belarea,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->judged_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmstddet+offsetpos,it_gpmstddet->disdesk,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;

			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_POWERCUT_DET(ORGAN_CODE,ORGAN_NAME,DMSID,GPMSID,FAULT_TIME,RETURN_TIME,FAULT_LINE,COUNT_TIME,SEND_TIME,FAULT_NO,BILLS_NUM,DETAILS,TDINTR,BELAREA,JUDGED_TIME,DISDESK) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7,to_date(:8,'YYYY-MM-DD HH24:MI:SS'),to_date(:9,'YYYY-MM-DD HH24:MI:SS'),:10,:11,:12,:13,:14,to_date(:15,'YYYY-MM-DD HH24:MI:SS'),:16);");
			//strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_POWERCUT_DET(ORGAN_CODE,JUDGED_TIME,DISDESK,TDINTR,BELAREA) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5);");

			if(d5000.d5000_WriteData("GPMS_POWERCUT_DET",query,temp_gpmstddet,number,16,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSTDDET_+3,error_info,threadlog);
				Log("Insert GPMS_POWERCUT_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSTDDET_+3,"入库成功.",threadlog);
			}

			free(temp_gpmstddet);
			free(attrs);
			attrs = NULL;
			temp_gpmstddet = NULL;
		}
		break;
	case _GPMSZLPDET_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[128];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[gpmszlpdet]:%d\n",filename,gpmszlpdet_vector.size());
			if(gpmszlpdet_vector.size() == 0)
			{
				SetGError(0,0,NULL,_GPMSZLPDET_+3,"文件内容为空",threadlog);
				break;
			}
			
			sprintf(sql,"DELETE FROM DETAIL.GPMS_ZLP WHERE COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')");
			
			if(d5000.DeleteDBDate(sql) != 0)
			{
				Log("delete GPMS_ZLP fail.\n");
				ret = ret | errmask;
			}
			else
			{
				Log("delete GPMS_ZLP success.\n");
			}
			printf("%s\n",sql);
			
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*12);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_CHAR;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_CHAR;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_CHAR;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_ZLCOT;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;
			attrs[7].data_type = DCI_STR;
			attrs[7].data_size = STR_LEN_SS;
			attrs[8].data_type = DCI_STR;
			attrs[8].data_size = STR_LEN_SS;
			attrs[9].data_type = DCI_STR;
			attrs[9].data_size = STR_LEN_SS;
			attrs[10].data_type = DCI_STR;
			attrs[10].data_size = STR_LEN_SS;
			attrs[11].data_type = DCI_STR;
			attrs[11].data_size = STR_LEN_SS;

			number = gpmszlpdet_vector.size();
			char *temp_gpmszlpdet = (char *)malloc((size_t)(number*4000));
			if(temp_gpmszlpdet == NULL)
				break;
			memset(temp_gpmszlpdet,0x00,number*4000);
			vector<gpmszlpdet>::iterator it_gpmszlpdet;
			for(it_gpmszlpdet = gpmszlpdet_vector.begin();it_gpmszlpdet != gpmszlpdet_vector.end();++it_gpmszlpdet)
			{
				memcpy(temp_gpmszlpdet+offsetpos,&(it_gpmszlpdet->citycode),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->cityname,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->dmsid,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->gpmsid,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->type,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->content,STR_LEN_ZLCOT);
				offsetpos += STR_LEN_ZLCOT;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->start_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->order_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->confirm_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->ctl_name,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_ZLP(ORGAN_CODE,ORGAN_NAME,DMSID,GPMSID,TYPE,CONTENT,START_TIME,ORDER_TIME,CONFIRM_TIME,COUNT_TIME,SEND_TIME,CTL_NAME) VALUES(:1,:2,:3,:4,:5,:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'),to_date(:9,'YYYY-MM-DD HH24:MI:SS'),to_date(:10,'YYYY-MM-DD HH24:MI:SS'),to_date(:11,'YYYY-MM-DD HH24:MI:SS'),:12);");
		
			if(d5000.d5000_WriteData("GPMS_ZLP",query,temp_gpmszlpdet,number,12,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSZLPDET_+3,error_info,threadlog);
				Log("Insert GPMS_ZLP_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSZLPDET_+3,"入库成功.",threadlog);
			}

			free(temp_gpmszlpdet);
			free(attrs);
			attrs = NULL;
			temp_gpmszlpdet = NULL;
		}
		break;
	case _DMSZLPDET_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			memset(query,0x00,sizeof(query));
			printf("%s:[dmszlpdet]:%d\n",filename,dmszlpdet_vector.size());
			if(dmszlpdet_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSZLPDET_+3,"文件内容为空",threadlog);
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*8);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_CHAR;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_ZLCOT;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;
			attrs[7].data_type = DCI_STR;
			attrs[7].data_size = STR_LEN_SS;

			number = dmszlpdet_vector.size();
			char *temp_dmszlpdet = (char *)malloc((size_t)(number*4000));
			if(temp_dmszlpdet == NULL)
				break;
			memset(temp_dmszlpdet,0x00,number*4000);
			vector<dmszlpdet>::iterator it_dmszlpdet;
			for(it_dmszlpdet = dmszlpdet_vector.begin();it_dmszlpdet != dmszlpdet_vector.end();++it_dmszlpdet)
			{
				memcpy(temp_dmszlpdet+offsetpos,&(it_dmszlpdet->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->dmsid,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->type,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->content,STR_LEN_ZLCOT);
				offsetpos += STR_LEN_ZLCOT;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->start_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->plan_operate_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;

			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_ZLP(ORGAN_CODE,DMSID,TYPE,CONTENT,START_TIME,PLAN_OPERATE_TIME,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'),to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_ZLP",query,temp_dmszlpdet,number,8,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSZLPDET_+3,error_info,threadlog);
				Log("Insert DMS_ZLP_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSZLPDET_+3,"入库成功.",threadlog);
			}

			free(temp_dmszlpdet);
			free(attrs);
			attrs = NULL;
			temp_dmszlpdet = NULL;
		}
		break;
	case _DMSXSD_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[dmsxsd]:%d\n",filename,dmsxsd_vector.size());
			if(dmsxsd_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSXSD_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = FLOAT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;

			number = dmsxsd_vector.size();
			char *temp_dmsxsd = (char *)malloc((size_t)(number*100));
			if(temp_dmsxsd == NULL)
				break;
			memset(temp_dmsxsd,0x00,number*100);
			vector<dmsxsd>::iterator it_dmsxsd;
			for(it_dmsxsd = dmsxsd_vector.begin();it_dmsxsd != dmsxsd_vector.end();++it_dmsxsd)
			{
				memcpy(temp_dmsxsd+offsetpos,&(it_dmsxsd->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsxsd+offsetpos,&(it_dmsxsd->value),FLOAT_LEN);
				offsetpos += FLOAT_LEN;
				memcpy(temp_dmsxsd+offsetpos,it_dmsxsd->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmsxsd+offsetpos,it_dmsxsd->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;

			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_XSD(ORGAN_CODE,VALUE,COUNT_TIME,SEND_TIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'),to_date(:4,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_XSD",query,temp_dmsxsd,number,4,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSXSD_+3,error_info,threadlog);
				Log("Insert DMS_XSD fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSXSD_+3,"入库成功.",threadlog);
			}

			free(temp_dmsxsd);
			free(attrs);
			attrs = NULL;
			temp_dmsxsd = NULL;
		}
		break;
	case _ZTGJ_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[ztgj]:%d\n",filename,ztgj_vector.size());
			if(ztgj_vector.size() == 0)
			{
				SetGError(0,0,NULL,_ZTGJ_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = FLOAT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;

			number = ztgj_vector.size();
			char *temp_ztgj = (char *)malloc((size_t)(number*100));
			if(temp_ztgj == NULL)
				break;
			memset(temp_ztgj,0x00,number*100);
			vector<ztgj>::iterator it_ztgj;
			for(it_ztgj = ztgj_vector.begin();it_ztgj != ztgj_vector.end();++it_ztgj)
			{
				memcpy(temp_ztgj+offsetpos,&(it_ztgj->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ztgj+offsetpos,&(it_ztgj->value),FLOAT_LEN);
				offsetpos += FLOAT_LEN;
				memcpy(temp_ztgj+offsetpos,it_ztgj->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_ztgj+offsetpos,it_ztgj->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;

			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_ZTGJ(ORGAN_CODE,VALUE,COUNT_TIME,SEND_TIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'),to_date(:4,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_ZTGJ",query,temp_ztgj,number,4,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_ZTGJ_+3,error_info,threadlog);
				Log("Insert DMS_ZTGJ fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_ZTGJ_+3,"入库成功.",threadlog);
			}

			free(temp_ztgj);
			free(attrs);
			attrs = NULL;
			temp_ztgj = NULL;
		}
		break;
	case _DMSYD_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[dmsyd]:%d\n",filename,dmsyd_vector.size());
			if(dmsyd_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSYD_+3,"文件内容为空",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_ID;
			attrs[2].data_type = DCI_INT;
			attrs[2].data_size = INT_LEN;
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

			number = dmsyd_vector.size();
			char *temp_dmsyd = (char *)malloc((size_t)(number*200));
			if(temp_dmsyd == NULL)
				break;
			memset(temp_dmsyd,0x00,number*200);
			vector<dmsyd>::iterator it_dmsyd;
			for(it_dmsyd = dmsyd_vector.begin();it_dmsyd != dmsyd_vector.end();++it_dmsyd)
			{
				memcpy(temp_dmsyd+offsetpos,&(it_dmsyd->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsyd+offsetpos,it_dmsyd->id,STR_LEN_ID);
				offsetpos += STR_LEN_ID;
				memcpy(temp_dmsyd+offsetpos,&(it_dmsyd->status),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsyd+offsetpos,&(it_dmsyd->rollback_time),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsyd+offsetpos,it_dmsyd->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmsyd+offsetpos,it_dmsyd->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;

			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_YD(ORGAN_CODE,YDD_ID,STATUS,ROLLBACK_TIME,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_YD",query,temp_dmsyd,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSYD_+3,error_info,threadlog);
				Log("Insert DMS_YD fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSYD_+3,"入库成功.",threadlog);
			}

			free(temp_dmsyd);
			free(attrs);
			attrs = NULL;
			temp_dmsyd = NULL;
		}
		break;
	case _GPMSYD_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[128];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[gpmsydddet]:%d\n",filename,gpmsyd_vector.size());
			if(gpmsyd_vector.size() == 0)
			{
				SetGError(0,0,NULL,_GPMSZLPDET_+3,"文件内容为空",threadlog);
				break;
			}
			
			sprintf(sql,"DELETE FROM DETAIL.GPMS_YD WHERE COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')");
			
			if(d5000.DeleteDBDate(sql) != 0)
			{
				Log("delete GPMS_YD fail.\n");
				ret = ret | errmask;
			}
			else
			{
				Log("delete GPMS_YD success.\n");
			}
			printf("%s\n",sql);
			
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*7);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_ID;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_CHAR;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;

			number = gpmsyd_vector.size();
			char *temp_gpmsyd = (char *)malloc((size_t)(number*500));
			if(temp_gpmsyd == NULL)
				break;
			memset(temp_gpmsyd,0x00,number*500);
			vector<gpmsyd>::iterator it_gpmsyd;
			for(it_gpmsyd = gpmsyd_vector.begin();it_gpmsyd != gpmsyd_vector.end();++it_gpmsyd)
			{
				memcpy(temp_gpmsyd+offsetpos,&(it_gpmsyd->citycode),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->cityname,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->ydd_id,STR_LEN_ID);
				offsetpos += STR_LEN_ID;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->status,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_gpmsyd+offsetpos,&(it_gpmsyd->rollback_time),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_YD(ORGAN_CODE,ORGAN_NAME,YDD_ID,STATUS,ROLLBACK_TIME,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),to_date(:7,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("GPMS_YD",query,temp_gpmsyd,number,7,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSYD_+3,error_info,threadlog);
				Log("Insert GPMS_YD fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSYD_+3,"入库成功.",threadlog);
			}

			free(temp_gpmsyd);
			free(attrs);
			attrs = NULL;
			temp_gpmsyd = NULL;
		}
		break;
	default:
		ret = ret|errmask;
		break;
	}
	//printf("insert end\n");
	return ret;
}

void  resolvexmldata::FreeSize()
{
	dmsop_vector.clear();
	datt_vector.clear();
	dagc_vector.clear();
	dagc_info_vector.clear();
	yxbw_vector.clear();
	dmsbrand_vector.clear();
	dmsstatus_vector.clear();
	gpmsbrand_vector.clear();
	gpmsstatus_vector.clear();
	zdzx_vector.clear();
	ykcz_vector.clear();
	yxinfo_vector.clear();
	tqnum_vector.clear();
	rate_vector.clear();

	zdzx_name_vector.clear();

	bus_vector.clear();
	cb_vetor.clear();
	dsc_vector.clear();
	trans_vector.clear();
	subs_vetor.clear();

	tdtrans_vector.clear();
	automap_vector.clear();
	tqrelease_vector.clear();
	fastr_vector.clear();
	tz_vector.clear();

	gpmstd_vetor.clear();
	gpmstddet_vector.clear();
	tzlist_qc_vector.clear();
	gpmszlpdet_vector.clear();
	dmszlpdet_vector.clear();
	dmsxsd_vector.clear();
	ztgj_vector.clear();
	dmsyd_vector.clear();
	gpmsyd_vector.clear();
}

#ifdef _USE_
//将入库记录存入log表
int resolvexmldata::insertDBLOG(int arg,int code,int retcode,const char *filename)
{
	//enum xmltype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_DMS_,_GPZW_GPMS_,_ZDZX_,_YKCZ_,_YXINFO_,_UnKnownFileType_}g_xmltype;
	//const char *g_xmltypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW_DMS","GPZW_GPMS","ZDZX","YKCZ","YXINFO"};
	enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_DMSCIME_,_GPMSCIME_,_DMSDATA_,_GPMSDATA_,_END_,_UnKnownFileType_}g_filetype;
	const char *g_xmltypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","DMSCIME","GPMSCIME","DMSDATA","GPMSDATA","END"};
	char query[100],temp[150],time_now[25],content[100];
	int offsetpos = 0,i = 0;
	time_t time_temp;
	struct tm *tm_temp;

	time_temp = time(NULL);

	memset(query,0x00,sizeof(query));
	memset(temp,0x00,sizeof(temp));
	memset(time_now,0x00,sizeof(time_now));
	memset(content,0x00,sizeof(content));

	tm_temp = localtime(&time_temp);
	sprintf(time_now,"%d-%d-%d %d-%d-%d",1900+tm_temp->tm_year,1+tm_temp->tm_mon,tm_temp->tm_mday,tm_temp->tm_hour,tm_temp->tm_min,tm_temp->tm_sec);

	switch(arg)
	{
	case _ZZZX_:
		sprintf(content,"DMS配电主站运行情况%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	case _DATT_:
		sprintf(content,"DMS馈线自动化投退记录%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	case _DAGC_:
		sprintf(content,"DMS馈线自动化过程记录%s入库%s,DMS馈线自动化过程记录%s_info入库%s",g_xmltypeName[arg],(retcode << 16)?"失败":"成功",g_xmltypeName[arg],(retcode >> 16)?"失败":"成功");
		break;
	case _YXBW_:
		sprintf(content,"DMS馈线自动化的开关遥信记录%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	case _DMSGPZW_:
		sprintf(content,"DMSDMS挂牌信息%s_brand入库%s,DMSDMS置位信息%s_status入库%s",g_xmltypeName[arg],(retcode << 16)?"失败":"成功",g_xmltypeName[arg],(retcode >> 16)?"失败":"成功");
		break;
	case _GPMSGPZW_:
		sprintf(content,"DMSGPMS挂牌信息%s_brand入库%s,DMSGPMS置位信息%s_status入库",g_xmltypeName[arg],(retcode << 16)?"失败":"成功",g_xmltypeName[arg],(retcode >> 16)?"失败":"成功");
		break;
	case _ZDZX_:
		sprintf(content,"DMS终端在线情况%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	case _YKCZ_:
		sprintf(content,"DMS遥控操作情况%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	case _YXINFO_:
		sprintf(content,"DMS遥信正确情况%s入库%s",g_xmltypeName[arg],retcode?"失败":"成功");
		break;
	default:
		break;
	}
	struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
	if(attrs == NULL)
		break;
	attrs[0].data_type = DCI_INT;
	attrs[0].data_size = 4;
	attrs[1].data_type = DCI_INT;
	attrs[1].data_size = 4;
	attrs[2].data_type = DCI_STR;
	attrs[2].data_size = 100;
	attrs[3].data_type = DCI_INT;
	attrs[3].data_size = 4;
	attrs[4].data_type = DCI_STR;
	attrs[4].data_size = 25;

	int type = 1,level = 1;

	memcpy(temp+offsetpos,&type,4);
	offsetpos += 4;
	memcpy(temp+offsetpos,&code,4);
	offsetpos += 4;
	memcpy(temp+offsetpos,content,sizeof(content));
	offsetpos += sizeof(content);
	memcpy(temp+offsetpos,&level,4);
	offsetpos += 4;
	memcpy(temp+offsetpos,time_now,sizeof(time_now));
	offsetpos += sizeof(time_now);

	strcpy(query,"INSERT INTO EVALUSYSTEM.MANAGE.LOG(TYPE,ORGAN_CODE,CONTENT,LEVEL,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
	d5000.d5000_WriteData("LOG",query,temp,1,5,attrs);
	
	free(attrs);
	attrs = NULL;


	if(arg == _DMSGPZW_ || arg == _GPMSGPZW_ || arg == _DAGC_)
	{
		if((retcode >> 16) ==0 || (retcode << 16) == 0)
		{
			insertfile(filename);
			mvxmlfile(filename,0);
		}
		else
			mvxmlfile(filename,1);
	}
	else
	{
		if(retcode == 0)
		{
			insertfile(filename);
			mvxmlfile(filename,0);
		}
		else
			mvxmlfile(filename,1);
	}

	return 0;
}
#endif

//删除指定日期记录
int resolvexmldata::deleteDBDate(int code,int arg)
{
	unsigned int mask = 1,retcode = 0;
	char sql[250];
	memset(sql,0x00,sizeof(sql));

	//enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_DMSCIME_,_GPMSCIME_,_DMSDATA_,_GPMSDATA_,_END_,_UnKnownFileType_}g_filetype;
	enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_RATE_,_BUS_,_CB_,_DSC_,_TRANS_,_SUBS_,_TDTRANS_,_AUTOMAP_,_TQRELEASE_,_FASTR_,_TZLIST_,_GPMSTD_,_GPMSTDDET_,_GPMSZLPDET_,_DMSZLPDET_,_DMSXSD_,_DMSZTGJ_,_DMSYD_,_END_,_UnKnownFileType_}g_filetype;

	//const char *g_xmltypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","DMSCIME","GPMSCIME","DMSDATA","GPMSDATA","END"};
	const char *g_xmltypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW","DMSBRAND","GPMSGPZW","ZDZX","YKCZ","YXINFO","YCRATE","DMSBUS","DMSCB","DMSCB","DMSLD","DMSST","DMSTD","AUTOGRAPH","TDINFO","FACTION","TZDETAILS","GPMSTD","GPMSTDDET","GPMS_ZLP","DMS_ZLP","DMS_XSD","DMS_ZTGJ","DMS_YD","END"};
	
		switch(arg)
		{
		case _ZZZX_:
		case _DATT_:
		case _YXBW_:
		case _ZDZX_:
		case _YKCZ_:
		case _YXINFO_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and SEND_TIME > to_date(sysdate,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			break;
		case _DAGC_:
			//sprintf(sql,"DELETE FROM DETAIL.dagc WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.dagc WHERE COUNT_TIME = '%s')",intime);
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;

			/*sprintf(sql,"DELETE FROM DETAIL.dagc_info WHERE COUNT_TIME '%s'",intime);
			retcode = d5000_insert.DeleteDBDate(sql);
			if (retcode != 0)
			retcode = retcode | (mask << 16);*/
			break;
		case _DMSGPZW_:
			//sprintf(sql,"DELETE FROM DETAIL.dmsbrand WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.dmsbrand WHERE COUNT_TIME = '%s')",intime);
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			//sprintf(sql,"DELETE FROM DETAIL.dmsstatus WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.dmsstatus WHERE COUNT_TIME = '%s')",intime);
			/*sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | (mask << 16);*/
			break;
		case _GPMSGPZW_:
			//sprintf(sql,"DELETE FROM DETAIL.gpmsbrand WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.gpmsbrand WHERE COUNT_TIME = '%s')",intime);
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			//sprintf(sql,"DELETE FROM DETAIL.gpmsstatus WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.gpmsstatus WHERE COUNT_TIME = '%s')",intime);
			/*sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | (mask << 16);*/
			break;
		case _BUS_:
		case _TRANS_:
		case _SUBS_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
				
			break;
		case _CB_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD') and type = 1",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			break;
		case _DSC_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD') and type = 0",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			break;
		case _TDTRANS_:
		case _AUTOMAP_:
		case _TQRELEASE_:
		case _FASTR_:
		case _TZLIST_:
		case _RATE_:
		case _DMSZLPDET_:
		case _DMSZTGJ_:
		case _DMSYD_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code = %d and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			
			break;
		case _DMSXSD_:
			sprintf(sql,"DELETE FROM DETAIL.%s WHERE organ_code like '%d%%' and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')",g_xmltypeName[arg],code);
			retcode = d5000.DeleteDBDate(sql);
			printf("%s\n",sql);
			if (retcode != 0)
				retcode = retcode | mask;
			break;
		default:
			break;
		}
	return retcode;
}

void* threadlogwrite(void *param)
{
	const int buf_size = 1024 * 1024; //1M
	int pipefd[2];
	vector<char> 	vBuf;
	tm last = { 0 };
	FILE *fp = NULL;
	char szLogPath[260];

	sprintf(szLogPath,g_config.log_path);

	struct stat st;
	if (stat(szLogPath, &st) == -1)
	{
		if (mkdir(szLogPath, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		{
			exit(0);
		}
	}

	cout << "注意：屏幕输出已写入日志文件，请用cat命令查看" << szLogPath << "目录下相应文件。" << endl;

	pipe(pipefd);

	//将输出缓存设置为行缓存模式
	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);

	dup2(pipefd[1], STDOUT_FILENO);
	dup2(pipefd[1], STDERR_FILENO);

	while (true)
	{
		char ch = 0;
		int ret = read(pipefd[0], &ch, 1);
		vBuf.push_back(ch);

		if (ch == '\n')
		{
			time_t now = time(NULL);
			tm *ptm = localtime(&now);

			//如果是新的一天，则重新建立新的日志文件
			if (ptm->tm_mday != last.tm_mday)
			{
				memcpy(&last, ptm, sizeof(tm));

				if (fp != NULL)
				{
					fclose(fp);
				}

				char szFile[260] = { 0 };

				sprintf(szFile, "%s/tResolve_%04d%02d%02d.log", szLogPath, ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

				fp = fopen(szFile, "a+");

				if (fp == NULL)
				{
					exit(0);
				}
			}

			char szTime[50] = { 0 };

			sprintf(szTime, "[%04d/%02d/%02d %02d:%02d:%02d]:", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
			fwrite(szTime, 1, strlen(szTime), fp);

			fwrite(&vBuf.front(), 1, vBuf.size(), fp);
			fflush(fp);

			vBuf.clear();
		}
	}

	return NULL;
}

int getorgan()
{
	if(d5000.d5000_Connect() != 0)
		return -1;
	char query[200],err_info[200],tempstr[100],tmp[20];
	char *buf = NULL,*m_presult = NULL;
	struct ColAttr *attrs = NULL;
	memset(query,0x00,sizeof(query));
	memset(err_info,0x00,sizeof(err_info));
	//strcpy(query,"select organ_code,organ_name from evalusystem.config.organ;");
	strcpy(query,"select organ_code from evalusystem.config.organ;");
	int rec_num = 0,attr_num = 0,rec = 0,col = 0,offset = 0;
	if(d5000.d5000_ReadData(query,&rec_num,&attr_num,&attrs,&buf,err_info) == 0)
	{
		m_presult = buf;
		int m_organ_code;
		for (rec = 0; rec < rec_num; rec++)
		{
			for(col = 0; col < attr_num; col++)
			{
				memset(tempstr, 0x00 , 100);
				memcpy(tempstr, m_presult, attrs[col].data_size);
				switch(col)
				{
				case 0:
					m_organ_code = *(int*)(tempstr);
					break;
				default:
					break;
				}
				m_presult += attrs[col].data_size;
			}
			vec_organ.push_back(m_organ_code);
		}
	}
	else
	{
		d5000.d5000_DisConnect();
		return -1;
	}
	if(rec_num > 0)
		d5000.g_CDci.FreeReadData(attrs,attr_num,buf);
	else
		d5000.g_CDci.FreeColAttrData(attrs, attr_num);
	d5000.d5000_DisConnect();
	return 0;
}

//校验模型文件是否发送，add by lcm 20150910
int checkmodel()
{
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];
	char* pstr=NULL;
	char* pdstr=NULL;
	char my_organ_code[10];
	int organ_code;
	char sql[200];

	memset(path_name, 0x00, sizeof(path_name));

	sprintf(path_name, "%s", g_config.cime_path);

	if ((dirp = opendir(path_name)) == NULL)
	{
		printf("open dir %s error!", path_name);
		return -1;
	}

	while ((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
			continue;
		else
		{
			memset(my_organ_code, 0x00, sizeof(my_organ_code));
			pstr=dirt->d_name;
			pdstr=strstr(dirt->d_name,"_");
			strncpy(my_organ_code,pstr,pdstr-pstr);
			organ_code = atoi(my_organ_code);
			//如果存在该organ_code的模型文件，则删除数据库中该地区未提供模型记录 add by lcm 20150910
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from evalusystem.manage.err_file where organ_code =%d and dis = '模型文件未提供';",organ_code);
			if(d5000.d5000_ExecSingle(sql)!=0)
			{
				printf("sql语句执行失败");
				return -1;
			}
		}
	}
	closedir(dirp);
	return 0;
}

int init_modelcheck()
{
	//添加模型校验初始化数据
	char error_info[256];
	for(int i=0;i<vec_organ.size();i++)
	{
		int offsetpos_model = 0;
		char qury[200];
		memset(qury,0x00,sizeof(qury));
		memset(error_info,0x00,sizeof(error_info));
		struct ColAttr* attrs_model = (ColAttr* )malloc(sizeof(ColAttr_t)*2);
		if(attrs_model == NULL)
			return;
		attrs_model[0].data_type = DCI_INT;
		attrs_model[0].data_size = INT_LEN;
		attrs_model[1].data_type = DCI_STR;
		attrs_model[1].data_size = STR_LEN_NAME;
		char *temp_err = (char *)malloc((size_t)(1000));
		if(temp_err == NULL)
			return;
		memset(temp_err,0x00,1000);
		
		memcpy(temp_err+offsetpos_model,&vec_organ[i],INT_LEN);
		offsetpos_model += INT_LEN;
		memcpy(temp_err+offsetpos_model,&(g_config.cime_path),STR_LEN_NAME);
		offsetpos_model += STR_LEN_NAME;
		
		sprintf(qury,"insert into evalusystem.manage.err_file(err_file_path,date,organ_code,count_file_path,dis,source) values "
			"(null,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:1,:2,'模型文件未提供','dms');");
		if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,2,attrs_model,error_info) != 0)
		{
			return -1;
		}

		free(temp_err);
		free(attrs_model);
		attrs_model = NULL;
		temp_err = NULL;
	}
	return 0;
}

