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
#define DC_SIZE 9 //�ؼ�������

void* threadlogwrite(void *param);

/// @param filename->�ļ�����·��
int mvxmlfile(const char *filename,int type);
/// @brief �Ӿ���·����ȡ�ļ���
/// @param _ap->����·��
/// @param _bn->�����ļ���
char *GetFileBaseName(const char *_ap,char *_bn);

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
	if(month_num % 2 != 0 )	//�ж��Ƿ���
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

//�Ƴ�dmsxml�����ļ�����ҹ�Ƴ�����dmsxml�ļ�����������賿ָ����㣬add by lcm 20150727
int RemoveFile()
{
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256];
	char path_full[256]; //�����Ҫɾ���ļ�����·��

	memset(path_name, 0x00, sizeof(path_name));

	sprintf(path_name, "%s", g_config.path_name);

	if ((dirp = opendir(path_name)) == NULL)
	{
		printf("open dir %s error!", path_name);
		return -1;
	}
	else {
		printf("open dir %s Success!\r\n", path_name);
	}

	while ((dirt = readdir(dirp)) != NULL)
	{
		if (strcmp(dirt->d_name, ".") == 0 || strcmp(dirt->d_name, "..") == 0)
			continue;
		else {
			memset(path_full, 0x00, sizeof(path_full));
			sprintf(path_full, "%s/%s", path_name, dirt->d_name);
			mvxmlfile(path_full, 0);
			//remove(dirt->d_name);
			//printf("Mv File %s/%s\r\n",g_config.path_name, dirt->d_name);
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
	if(month_num % 2 != 0 && tm_temp->tm_wday == 1)	//�ж��Ƿ�����һ
	{
		sprintf(qury,"TRUNCATE TABLE EVALUSYSTEM.CONFIG.HISRECORD;");
		if(d5000.d5000_ExecSingle(qury)!=0)
		{
			return -1;
		}
	}
	return 0;
}

int DeleteHisrecordForDays()
{
	char qury[200];
	int year_temp, month_temp, day_temp, w;
	time_t time_temp;
	struct tm *tm_temp;

	time_temp = time(NULL);
	/* // @detail ��ȡ14��ǰ������Ϣ */
	time_temp -= 1209600;
	tm_temp = localtime(&time_temp);
	month_temp = tm_temp->tm_mon + 1;
	year_temp = tm_temp->tm_year + 1900;
	day_temp = tm_temp->tm_mday;

	sprintf(qury, "DELETE FROM EVALUSYSTEM.CONFIG.HISRECORD WHERE GTIME LIKE '%04d-%02d-%02d%';", year_temp, month_temp, day_temp);

	if (d5000.d5000_ExecSingle(qury) != 0) {
		return -1;
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
IN arg:�ɹ�ʧ�ܣ�1�ɹ� 2ʧ��
IN code:���繫˾����
IN str:�����ƻ��ļ���
IN info:��Ϣ
OUT threadlog:�洢
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
			if(nCount == 0 && strBuffer[i] == '_')//����_��ID����һ��Ĳ������ļ���ʽ
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
	
	const char *g_indexname[] = {"�����վ��ƽ��������","FAͶ����","ң����ȷ��","������Ϣһ����","������ȷ��","�ն�������","ң�سɹ���","ң��ʹ����",
		"DMS���òɹ�ר��̨��ƥ����","DMS��ר��ɼ��ɹ���","���ĸ����������","��翪����������","��絶բ��������","�豸״̬һ����","����ѹ��������","���վ����������",
		"DMSͣ����Ϣ����������","�Զ���ͼ��","�Զ���ͼ����","ͣ����Ϣȷ����","ͣ����Ϣ������ʱ��","ͣ����Ϣ����ʱ��","FA��������","��ר����ϸ","ͣ�������","ͣ�������ϸ","GPMSָ��Ʊ��ϸ","DMSָ��Ʊ��ϸ","Сˮ����ϸ"};

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
			threadlog.arg,"ָ���ļ�����Ϊ��",threadlog.code,g_indexname[threadlog.name]);
	}
	else if(threadlog.arg == 1)
	{
		sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"ָ��������ȷ",threadlog.code,g_indexname[threadlog.name]);
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
	
	if (threadlog.name == 8 || threadlog.name == 12 || threadlog.name == 17 || threadlog.name == 19)//"DMS���òɹ�ר��̨��ƥ����","��翪����������","�Զ���ͼ��","ͣ����Ϣȷ����"
	{
		if(threadlog.arg == 0)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,"�����ļ�����Ϊ��",threadlog.code,g_indexname[threadlog.name+1]);
		else if(threadlog.arg == 1)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,"ָ��������ȷ",threadlog.code,g_indexname[threadlog.name+1]);
		else
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
				threadlog.arg,threadlog.log,threadlog.code,g_indexname[threadlog.name+1]);
				
		if(d5000.d5000_ExecSingle(temp)!=0)
		{
			return;
		}
	}
	
	if(threadlog.name == 19)//"ͣ����Ϣȷ����"
	{
		if(threadlog.arg == 0)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"�����ļ�����Ϊ��",threadlog.code,g_indexname[threadlog.name+2]);
		else if(threadlog.arg == 1)
			sprintf(temp,"update result.datarec set value = %d,reason = '%s' where organ_code = %d and name = '%s' and cur_time = to_date(sysdate,'YYYY-MM-DD')",
			threadlog.arg,"ָ��������ȷ",threadlog.code,g_indexname[threadlog.name+2]);
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
	
	//����У������д�����ݿ� add by lcm 20150917
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
			"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'ָ���ļ�����Ϊ��','dms');");
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

//�ļ��������ļ��������Ѵ���
int insertfile(const char *filename)
{
	if(filename == NULL)
		return -1;

	int ret = 0;

	pthread_rwlock_rdlock(&lockfile);
	FILE *fp = fopen("traversalfile.txt","a+");
	if(fp == NULL) {
		pthread_rwlock_unlock(&lockfile);
		Log("fopen fail,%s can't write in traversalfile.txt\n",filename);
		return -1;
	}

	fprintf(fp,"%s\n",filename);

	fclose(fp);

	pthread_rwlock_unlock(&lockfile);

	return 0;
}

int inserthisrec(const char *filename,int citycode, Cim_Type _type)
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
	sprintf(qury,"INSERT INTO EVALUSYSTEM.CONFIG.HISRECORD(ORGAN_CODE,NAME,CIM_TYPE,GTIME) values(%d,'%s',%d,"
		"to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'));",citycode,filename,_type);

	if ( d5000.d5000_ExecSingle(qury)!=0) {
		return -1;
	}

	return 0;
}

/*****************************
type:0:�ɹ� 1:ʧ��
*****************************/
int mvxmlfile(const char *filename,int type)
{
	if(filename == NULL)
		return -1;

	int ret = 0;
	char newfile[256];
	char base_name[256];
	memset(newfile,0x00,sizeof(newfile));
	memset(base_name,0x00,sizeof(base_name));
	char timenow[100];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);

	GetFileBaseName(filename, base_name);

	//���ļ��Ƴ�������Ŀ¼
	if(type == 0)
		sprintf(newfile,"%s/%s/%s",g_config.back_path,timenow, base_name);
	else if(type == 1)
		sprintf(newfile,"%s/%s/err/%s",g_config.back_path,timenow, base_name);
	else if(type == 2)
		sprintf(newfile,"%s/%s",g_config.oracle_path, base_name);

	/*if(strstr(filename,"TZ") != NULL || strstr(filename,"tzlist") != NULL)
		sprintf(newfile,"%s/%s/TZ/%s",g_config.back_path,timenow,filename);*/

	ret = rename(filename,newfile);
	if(ret == -1) {
		Log("%s mv fail.errno:%s.newfile:%s.\n",filename,strerror(errno),newfile);
		return -1;
	}
	else {
		Log("%s mv to %s success.\n", filename, newfile);
	}

	return 0;
}

char * GetFileBaseName(const char *_ap, char *_bn)
{
	const char *base_pos_ = NULL;

	base_pos_ = strrchr(_ap,'/');
	if (base_pos_ == NULL) {
		strcpy(_bn, _ap);
	} // 
	else {
		strcpy(_bn, base_pos_ + 1);
	}

	return _bn;
}

int InitXmlMutex()
{
	int ret = pthread_mutex_init(&thr_xml_mutex.queue_mutex_file, NULL);//��ʹ���߳���
	if(ret < 0)
	{
		Log("----- ��ʹ������xml�ļ��߳���ʧ�� ----------\n");
	}

	ret = pthread_cond_init(&thr_xml_mutex.queue_cond_file, NULL);   //��ʹ����������
	if(ret < 0)
	{
		Log("----- ��ʹ������xml�ļ��߳���������ʧ��  ----------\n");
	}
	ret = pthread_rwlock_init(&lockfile,NULL);
	if(ret < 0)
	{
		Log("----- ��ʹ���ļ���ʧ��  ----------\n");
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

	//����д��־�߳�
	pthread_t pt;
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	if (pthread_create(&pt, &attr, threadlogwrite, NULL) != 0) {
		printf("����д��־�߳�ʧ�ܣ�\r\n");
		return -1;
	}

	pthread_detach(pt);

	for (;i<maxthr;i++) {

		ret=pthread_create(&thr_xml[i],NULL,(void*(*)(void*))process_queue_xml,NULL);
		if(ret<0) {
			Log("------- ����xml�����߳�%dʧ�� : %s ---------------------\n",i,strerror(ret));
		}
	}

	while(1) {

		if((ret = pthread_create(&cimthr,NULL,(void*(*)(void*))process_cim,NULL))<0) {
			Log("------- �����ļ��б�cim���߳�ʧ�� : %s ---------------------\n",strerror(ret));
		}
		pthread_join(cimthr,&trathrresult);

		if((ret = pthread_create(&trathr,NULL,(void*(*)(void*))process_traversal,NULL))<0) {
			Log("------- �����ļ��б����߳�ʧ�� : %s ---------------------\n",strerror(ret));
		}

		pthread_join(trathr,&trathrresult);

		Log("thread quit.\n");

		while(1) {
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
			if (tm_temp->tm_hour == 0) {
				if(d5000.d5000_Connect() != 0)
					return -1;
				remove("traversalfile.txt");
				//TruncateHisrecord();
				DeleteHisrecordForDays();
				break;
			}
			else if(tm_temp->tm_hour == 3) {
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (90-tm_temp->tm_min)*60;
				continue;
			}
			else if(tm_temp->tm_hour == 4) {
				del = 0;
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if(tm_temp->tm_hour == 5) { //Ϊ����gpmsͣ����Ϣ��������
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (100-tm_temp->tm_min)*60;
				break;
			}
			else if(tm_temp->tm_hour == 6) {
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			else if(tm_temp->tm_hour == 7) { //Ϊ����Сˮ�絥������
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}
			/*else if (tm_temp->tm_hour == 11) {
				time_temp = time(NULL);
				tm_temp = localtime(&time_temp);
				g_config.poll_interval = (70 - tm_temp->tm_min) * 60;
				d5000.d5000_DisConnect();
				continue;
			}
			else if (tm_temp->tm_hour == 12) {

				if(d5000.d5000_Connect() != 0)
					return -1;

				del = 1;
				remove("traversalfile.txt");
				g_config.poll_interval = 3600;
				checkmodel();
				break;
			}*/
			else if (tm_temp->tm_hour == 23) { //ɾ��dmsxml�����ļ�����ҹɾ������dmsxml�ļ�����������賿ָ����㣬add by lcm 20150727
			
				printf("Begin Delete Old File\r\n");
				if(RemoveFile()!=0) {
					printf("ɾ���ļ�����!\r\n");
				}
				else {
					printf("�Ƴ��ļ��ɹ�!\r\n");
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
	for (i=0;i<maxthr;i++) {
		pthread_join(thr_xml[i],&thr_result[i]);
	}
}

//�ļ���������
int dequeue_xml_recv(char *filename)
{
	pthread_mutex_lock(&thr_xml_mutex.queue_mutex_file);

	while (thr_xml_mutex.queue_file.size()== 0 ) {
		pthread_cond_wait(&thr_xml_mutex.queue_cond_file,&thr_xml_mutex.queue_mutex_file);
	}

	strcpy(filename,thr_xml_mutex.queue_file.front().c_str());

	if (thr_xml_mutex.queue_file.size() != 0) {
		thr_xml_mutex.queue_file.pop();
	}

	pthread_mutex_unlock(&thr_xml_mutex.queue_mutex_file);

	return 0;
}

//�����ļ��̻߳ص�����
void *process_queue_xml()
{
	char full_src_path[250];
	char full_des_path[250];
	char tempname[256];

	while(1) {
		memset(tempname,0x00,sizeof(tempname));
		memset(full_src_path,0x00,sizeof(full_src_path));
		memset(full_des_path,0x00,sizeof(full_des_path));

		dequeue_xml_recv(tempname);

		if(tempname == NULL)
			break;

		mylog threadlog;
		memset(&threadlog,0x00,sizeof(mylog));
		class resolvexmldata reslov_file;
		reslov_file.readdata_xml(tempname,threadlog);
	}
}

int set_default_conf(config &Pa_config)
{
	//strcpy(Pa_config.path_name,"/home/d5000/dpkf/Server/doc");
	strcpy(Pa_config.cime_path,"/home/d5000/dpkf/Server/doc");
	Pa_config.max_thread = 100;
	Pa_config.poll_interval = 5;
	Pa_config.max_record =  5000;
	return true;
}

//��ȡ�����ļ�
int read_config(char* filename, config& Pa_config)
{
	if( NULL == filename)
		return -1;

	char temp_[1024];
	char sys_home_[256];

	memset(sys_home_, 0x00, sizeof(sys_home_));
	strncpy(sys_home_, getenv("HOME"), sizeof(sys_home_) - 1); /*��ȡ$Home*/

	bool is_exist = access(filename, F_OK) < 0 ? false : true;
	if(!is_exist) {
		Log("config file %s don't exist!!\n", filename);
		return -1;
	}

	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) {
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

	for(; pElem != NULL; pElem=pElem->NextSiblingElement()) {

		pattrname = pElem->Attribute("paramname");
		if(!pattrname)
			return false;
		//paraname = std::string(pattr);
		pattrvalue = pElem->Attribute("paramvalue");
		if(!pattrvalue)
			return false;

		if(strcmp(pattrname,"max_thread") == 0)
			Pa_config.max_thread = atoi(pattrvalue);
		else if(strcmp(pattrname,"poll_interval") == 0)
			Pa_config.poll_interval = atoi(pattrvalue);
		else if (strcmp(pattrname, "path_name") == 0) { /*������$HOME*/
			memset(temp_, 0x00, sizeof(temp_));
			strncpy(temp_, pattrvalue, sizeof(temp_) - 1);
			SplitStrByCh(temp_, sys_home_, ':', Pa_config.path_name);
		}
		else if(strcmp(pattrname,"oracle_path") == 0)
			strcpy(Pa_config.oracle_path,pattrvalue);
		else if (strcmp(pattrname, "cime_path") == 0) { 
			strncpy(Pa_config.cime_path, pattrvalue, sizeof(Pa_config.cime_path) - 1);
		}
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
		else if (strcmp(pattrname,"db_server") == 0) {

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

//�����༶Ŀ¼
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

	//Ĭ�ϸ�ʽ"/"��ʼ��������Ե���ʼ"/"
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

//�����洢�ļ���
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
	//���������ļ���
	if(CreateFolder() !=0)
		return false;
	return true;
}

//�ļ��������
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
//�ж��ļ��Ƿ��Ѿ������
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
//�������ļ���
**************************/
int TraversalFolder()
{
	int ret = -1;
	DIR *dirp;
	struct dirent *dirt;
	struct stat st;
	char path_name[256], absolute_path[256];

	for (int i = 0; i < 2;++i) {
		memset(path_name, 0x00, sizeof(path_name));

		strcpy(path_name, g_config.path_name[i]);

		if ((dirp = opendir(path_name)) == NULL) {
			perror("open dir error!");
			return -1;
		}

		while ((dirt = readdir(dirp)) != NULL) {

			if (strcmp(dirt->d_name, ".") == 0 
				|| 
				strcmp(dirt->d_name, "..") == 0)
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
			if (ret == 1) {
				memset(absolute_path, 0x00, sizeof(absolute_path));
				sprintf(absolute_path, "%s/%s", path_name, dirt->d_name);

				enqueue_xml_recv(absolute_path);
			}
		}

		closedir(dirp);
	} // 

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

	if((dirp = opendir(path_name)) == NULL) {
		perror("open dir error!");
		return -1;
	}

	while((dirt = readdir(dirp)) != NULL) {

		if (strcmp(dirt->d_name,".")==0 || strcmp(dirt->d_name,"..") == 0)
			continue; 

		len = strlen(dirt->d_name);

		/* // @detail Ϊxml��׺�ļ����Ҵ���"����ͼ"������(�Ǹ�ѹ�û�) */
		if ((strcmp(dirt->d_name + len - 3, "xml") == 0)
			&&
			(strstr(dirt->d_name,"����ͼ") != NULL)) { 
			xml_vector.push_back(dirt->d_name);
		}
	}

	closedir(dirp);

	return 0;
}

int mvcim::mvtocim(int organ_code, Cim_Type _type)
{
	int num = 0,seq = 0,type = 0,cityhavesize = 0,citycode = 0;
	int find_count_ = 0,find_flag_ = 0;
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
	
	/*if(strstr(code,"35401") != NULL) {
		cityhavesize = fz_map.count(code);
		citycode = 0;
	}
	else if(strstr(code,"35402") != NULL) {
		cityhavesize = xm_map.count(code);
		citycode = 1;
	}
	else if(strstr(code,"35403") != NULL) {
		cityhavesize = nd_map.count(code);
		citycode = 2;
	}
	else if(strstr(code,"35404") != NULL) {
		cityhavesize = pt_map.count(code);
		citycode = 3;
	}
	else if(strstr(code,"35405") != NULL) {
		cityhavesize = qz_map.count(code);
		citycode = 4;
	}
	else if(strstr(code,"35406") != NULL) {
		cityhavesize = zz_map.count(code);
		citycode = 5;
	}
	else if(strstr(code,"35407") != NULL) {
		cityhavesize = ly_map.count(code);
		citycode = 6;
	}
	else if(strstr(code,"35408") != NULL) {
		cityhavesize = sm_map.count(code);
		citycode = 7;
	}
	else if(strstr(code,"35409") != NULL) {
		cityhavesize = np_map.count(code);
		citycode = 8;
	}*/
	
	int count_have_ = m_mOrganCode2Num.count(organ_code);
	multimap<int, CimHaveNum>::iterator itr_ = m_mOrganCode2Num.find(organ_code);
	for (int i = 0; i < count_have_;++i) {

		if (itr_->second.s_type == _type) {
			cityhavesize = itr_->second.s_num;
			break;
		} // 

		++itr_;
	} // 

	sprintf(trapath,"%s/All_Model_%d",g_config.cime_path,organ_code);

	if(getfilenum(trapath) == 0) {

		/* // @detail ʵ��δɸѡģ��ȫ����� ȥ���������� 2017-04-09 */
		/*if (xml_vector.size() >= cityhavesize) {
			num = (xml_vector.size()-cityhavesize)/g_config.day;
		}
		else { //�����м任ר��ͼ��������ĿС���ѷ���������Ŀ
			num = xml_vector.size()/14;
		}*/
		num = xml_vector.size() - cityhavesize;
		cout << "code:" << organ_code << "\tcim_num:" << xml_vector.size() << "\tcityhave_num:" << cityhavesize << "\tnum:" << num << endl;

#if 0
		/* // @detail ���ظ�,ȥ��zgs */
		if (xml_vector.size() <= cityhavesize) {
			cityhavesize == 0;
		} // 
#endif
		/*if(xml_vector.size() == cityhavesize) {

			if(citycode == 0) {
				fz_map.clear();
				fz_vector.clear();
			}
			else if(citycode == 1) {
				xm_map.clear();
				xm_vector.clear();
			}
			else if(citycode == 2) {
				nd_map.clear();
				nd_vector.clear();
			}
			else if(citycode == 3) {
				pt_map.clear();
				pt_vector.clear();
			}
			else if(citycode == 4) {
				qz_map.clear();
				qz_vector.clear();
			}
			else if(citycode == 5) {
				zz_map.clear();
				zz_vector.clear();
			}
			else if(citycode == 6) {
				ly_map.clear();
				ly_vector.clear();
			}
			else if(citycode == 7) {
				sm_map.clear();
				sm_vector.clear();
			}
			else if(citycode == 8) {
				np_map.clear();
				np_vector.clear();
			}
		}*/

		vector<string>::iterator it = xml_vector.begin();
		for (;it != xml_vector.end();it++) {
			find_flag_ = 0;

			sprintf(oldname,"%s/%s",trapath,it->c_str());
			memset(messSplit,0,sizeof(messSplit));
			memset(temp_filename,0,sizeof(temp_filename));
			strncpy(temp_filename,it->c_str(),sizeof(temp_filename));
			SeperateStringsFromOneLine(temp_filename,messSplit[0]);
			memset(temp_file,0,sizeof(temp_file));
			strncpy(temp_file,messSplit[0][0],sizeof(temp_file));
			//temp_file[strslen(temp_file)-1] = 0;
			temp_file[strlen(temp_file)] = 0;
			/* // @detail ʵ��δɸѡģ��ȫ����� ȥ���������� 2017-04-09 */
			//if(seq < num) { /*δ�ﵽָ������*/

				find_count_ = m_multiFeederId2ModelInfo.count(temp_file);
				if (find_count_ != 0) {
					
					multimap<string, modelinfo>::iterator itr_feeder = m_multiFeederId2ModelInfo.find(temp_file);
					for (int i = 0; i < find_count_; ++i) {

						if ((itr_feeder->second.organ_code == organ_code)
							&&
							(itr_feeder->second.type == _type)) { /*������ͬ��������ͬһ���������¾�ģ��һ��*/
							/*sprintf(newname, "%s/%s", g_config.mx_path, it->c_str());
							rename(oldname, newname);
							seq++;
							inserthisrec(it->c_str(), organ_code, _type);*/

							find_flag_ = 1;
							break;
						} // 

						++itr_feeder;
					} // 
				}

				if (find_count_ == 0
					||
					find_flag_ == 0) {
					sprintf(newname, "%s/%s", g_config.mx_path, it->c_str());
					rename(oldname, newname);
					seq++;
					inserthisrec(it->c_str(), organ_code, _type);
					/* // @detail �洢��ɸѡ�ļ� */
					modelinfo modelinfo_intemp_;
					memset(&modelinfo_intemp_, 0x00, sizeof(modelinfo));
					strncpy(modelinfo_intemp_.feederid, temp_file, sizeof(modelinfo_intemp_.feederid) - 1);
					modelinfo_intemp_.organ_code = organ_code;
					modelinfo_intemp_.type = _type;
					m_multiFeederId2ModelInfo.insert(make_pair(modelinfo_intemp_.feederid, modelinfo_intemp_));
				} // 

				remove(oldname);

				/*if(citycode == 0) {

					vector<string>::iterator it_find = find(fz_vector.begin(),fz_vector.end(),temp_file);
					if(it_find == fz_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),organ_code);
					}

					remove(oldname);
				}
				else if(citycode == 1) {

					vector<string>::iterator it_find = find(xm_vector.begin(),xm_vector.end(),temp_file);
					if(it_find == xm_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 2) {

					vector<string>::iterator it_find = find(nd_vector.begin(),nd_vector.end(),temp_file);
					if(it_find == nd_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 3) {

					vector<string>::iterator it_find = find(pt_vector.begin(),pt_vector.end(),temp_file);
					if(it_find == pt_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 4) {

					vector<string>::iterator it_find = find(qz_vector.begin(),qz_vector.end(),temp_file);
					if(it_find == qz_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 5) {

					vector<string>::iterator it_find = find(zz_vector.begin(),zz_vector.end(),temp_file);
					if(it_find == zz_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 6) {

					vector<string>::iterator it_find = find(ly_vector.begin(),ly_vector.end(),temp_file);
					if(it_find == ly_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 7) {

					vector<string>::iterator it_find = find(sm_vector.begin(),sm_vector.end(),temp_file);
					if(it_find == sm_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					}

					remove(oldname);
				}
				else if(citycode == 8) {

					vector<string>::iterator it_find = find(np_vector.begin(),np_vector.end(),temp_file);
					if(it_find == np_vector.end()) {
						sprintf(newname,"%s/%s",g_config.mx_path,it->c_str());
						rename(oldname,newname);
						seq++;
						inserthisrec(it->c_str(),atoi(organ_code));
					 }

					remove(oldname);
				}*/
			/*}
			else {
				sprintf(oldname,"%s/%s",trapath,it->c_str());
				remove(oldname);
			}*/
		}
	}

	memset(temp,0,sizeof(temp));
	sprintf(temp,"update result.datarec set value = 1,reason = 'ָ��������ȷ' where organ_code = %d and name = '������ȷ��' and cur_time = to_date(sysdate,'YYYY-MM-DD')",citytype[citycode]);
	
	d5000.d5000_ExecSingle(temp);
	
	if(rmdir(trapath) == -1)
		perror("rmdir:");

	return 0;
}

int mvcim::unzip_fun(char *filename,int _code, Cim_Type _type)
{
	char query[200];
	memset(query,0,sizeof(query));
	sprintf(query,"unzip -o %s/%s -d %s/All_Model_%d",g_config.cime_path,filename,g_config.cime_path, _code);
	system(query);

	mvtocim(_code, _type);
	return 0;
}

//�����̻߳ص�����
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

	if(tm_temp->tm_hour == 0 && tm_temp->tm_min <= 30) //һСʱ����һ�Σ���0���ڼ�����ʱ���������Ϣ
		sign = 0;

#ifdef _DATAREC
	else
		sign = 1;
#endif // _DATAREC

	if(sign == 0) {
		int offsetpos = 0,number = 0;
		char query[200];
		memset(query,0x00,sizeof(query));

		//ɾ���������б�
		sprintf(query,"delete from EVALUSYSTEM.RESULT.DATAREC where cur_time = to_date(sysdate,'YYYY-MM-DD')");
		d5000.DeleteDBDate(query);

		//int code[] = {35401,35402,35403,35404,35405,35406,35407,35408,35409};
		int acvalue[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		char *index[] = {"�����վ��ƽ��������","FAͶ����","ң����ȷ��","������Ϣһ����","������ȷ��","�ն�������","ң�سɹ���","ң��ʹ����",
			"DMS���òɹ�ר��̨��ƥ����","DMS��ר��ɼ��ɹ���","���ĸ����������","��翪����������","��絶բ��������","�豸״̬һ����","���վ����������",
			"����ѹ��������","DMSͣ����Ϣ����������","�Զ���ͼ��","�Զ���ͼ����","ͣ����Ϣȷ����","ͣ����Ϣ������ʱ��","ͣ����Ϣ����ʱ��","FA��������","��ר����ϸ","ͣ�������","ͣ�������ϸ"};

		datarec temp_datarec;
		vector<datarec> vec_datarec;
		vec_datarec.clear();

		for (i = 0;i < vec_organ.size();i++) {

			for (j = 0;j < 26;j++) {
				memset(&temp_datarec,0,sizeof(datarec));
				temp_datarec.organ_code = vec_organ[i];
				temp_datarec.value = acvalue[j];
				strcpy(temp_datarec.name,index[j]);
				strcpy(temp_datarec.reason,"δ�ṩ����");
				vec_datarec.push_back(temp_datarec);
			}
		}
		//���ģ��У���ʼ������
		if(init_modelcheck() != 0) {
			printf("ģ��У�����ݳ�ʼ��ʧ�ܣ�");
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

		for(it_datarec = vec_datarec.begin();it_datarec != vec_datarec.end();++it_datarec) {
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

		if(d5000.d5000_WriteData("DATAREC",query,tmp_datarec,number,4,attrs,error_info)!= 0) {
			return NULL;
		}
		
		free(tmp_datarec);
		free(attrs);
		attrs = NULL;
		tmp_datarec = NULL;
		sign = 1;
	}

	if(TraversalFolder() != 0) {
		//ʧ�ܷ���"INVA",����ļ����к�
		//enqueue_xml_recv("INVA");
		return NULL;
	}
	//����͡�END������
	//enqueue_xml_recv("END");

	printf("insert end\n");

	return NULL;
}

int mvcim::gethavexml()
{
	int i = 0;
	int find_count_ = 0, find_flag_ = 0;
	char temp_line[300];
	char temp_name[300];
	char messSplit[2][3][300];
	FILE * filefp = NULL;
	const char *g_logname[] = {"fz.txt","xm.txt","nd.txt","pt.txt","qz.txt","zz.txt","ly.txt","sm.txt","np.txt"};

	//ȥ���ļ��洢��ʽ
#if 0
	for (i = 0;i < 9;i++) {

		filefp = fopen(g_logname[i],"r");
		if (filefp == NULL) {

			if(errno == 2)
				continue;

			printf("%s:%s\n",g_logname[i],strerror(errno));
			return -1;
		}

		while(!feof(filefp)) {

			memset(temp_line,0x00,sizeof(temp_line));
			if(fgets(temp_line,sizeof(temp_line),filefp) == NULL)
				break;

			memset(messSplit,0,sizeof(messSplit));
			SeperateStringsFromOneLine(temp_line,messSplit[0]);
			memset(temp_name,0x00,sizeof(temp_name));
			strncpy(temp_name,messSplit[0][0],sizeof(temp_name));
			temp_name[strlen(temp_name)] = 0;

			if(i == 0) {
				fz_map.insert(make_pair("35401",temp_name));
				fz_vector.push_back(temp_name);
			}
			else if (i == 1) {
				xm_map.insert(make_pair("35402",temp_name));
				xm_vector.push_back(temp_name);
			}
			else if (i == 2) {
				nd_map.insert(make_pair("35403",temp_name));
				nd_vector.push_back(temp_name);
			}
			else if (i == 3) {
				pt_map.insert(make_pair("35404",temp_name));
				pt_vector.push_back(temp_name);
			}
			else if (i == 4) {
				qz_map.insert(make_pair("35405",temp_name));
				qz_vector.push_back(temp_name);
			}
			else if (i == 5) {
				zz_map.insert(make_pair("35406",temp_name));
				zz_vector.push_back(temp_name);
			}
			else if (i == 6) {
				ly_map.insert(make_pair("35407",temp_name));
				ly_vector.push_back(temp_name);
			}
			else if (i == 7) {
				sm_map.insert(make_pair("35408",temp_name));
				sm_vector.push_back(temp_name);
			}
			else if (i == 8) {
				np_map.insert(make_pair("35409",temp_name));
				np_vector.push_back(temp_name);
			}
		}

		if(ferror(filefp)!=0) {
			fclose(filefp);
			perror("read or write");
			return -1;
		}

		fclose(filefp);
	}
#endif

	char ccitycode[20];
	char qury[100];
	time_t time_temp;
	struct ColAttr *attrs = NULL;
	struct tm *tm_temp;
	char *buf = NULL,*m_presult = NULL;
	char err_info[200],tempstr[300];
	modelinfo modelinfo_temp_;

	memset(err_info,0,sizeof(err_info));
	memset(qury,0,sizeof(qury));
	time_temp = time(NULL);
	tm_temp = localtime(&time_temp);
	sprintf(qury,"SELECT ORGAN_CODE,NAME,CIM_TYPE FROM EVALUSYSTEM.CONFIG.HISRECORD WHERE 1=1;");

	int rec_num = 0,attr_num = 0,rec = 0,col = 0,offset = 0;
	if(d5000.d5000_ReadData(qury,&rec_num,&attr_num,&attrs,&buf,err_info) == 0) {
		m_presult = buf;

		for (rec = 0; rec < rec_num; rec++) {
			memset(&modelinfo_temp_, 0x00, sizeof(modelinfo));

			for(col = 0; col < attr_num; col++) {
				memset(tempstr, 0x00 , 300);
				memcpy(tempstr, m_presult, attrs[col].data_size);

				switch(col) {
				case 0:
					modelinfo_temp_.organ_code = *(int*)(tempstr);
					break;
				case 1:
					memset(messSplit,0,sizeof(messSplit));
					SeperateStringsFromOneLine(tempstr,messSplit[0]);
					memset(temp_name,0x00,sizeof(temp_name));
					strncpy(temp_name,messSplit[0][0],sizeof(temp_name));
					temp_name[strlen(temp_name)] = 0;

					strncpy(modelinfo_temp_.feederid, temp_name, sizeof(modelinfo_temp_.feederid) - 1);
					
					/*sprintf(ccitycode,"%d",citycode);

					if(strstr(ccitycode,"35401") != NULL) {
						fz_map.insert(make_pair(ccitycode,temp_name));
						fz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35402") != NULL) {
						xm_map.insert(make_pair(ccitycode,temp_name));
						xm_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35403") != NULL) {
						nd_map.insert(make_pair(ccitycode,temp_name));
						nd_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35404") != NULL) {
						pt_map.insert(make_pair(ccitycode,temp_name));
						pt_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35405") != NULL) {
						qz_map.insert(make_pair(ccitycode,temp_name));
						qz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35406") != NULL) {
						zz_map.insert(make_pair(ccitycode,temp_name));
						zz_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35407") != NULL) {
						ly_map.insert(make_pair(ccitycode,temp_name));
						ly_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35408") != NULL) {
						sm_map.insert(make_pair(ccitycode,temp_name));
						sm_vector.push_back(temp_name);
					}
					else if(strstr(ccitycode,"35409") != NULL) {
						np_map.insert(make_pair(ccitycode,temp_name));
						np_vector.push_back(temp_name);
					}*/
					break;
				case 2:
					modelinfo_temp_.type = static_cast<Cim_Type>(*(int*)tempstr);
					break;
				default:
					break;
				}

				m_presult += attrs[col].data_size;
			}

			m_multiFeederId2ModelInfo.insert(make_pair(modelinfo_temp_.feederid, modelinfo_temp_));

			find_count_ = 0;
			find_flag_ = 0;
			find_count_ = m_mOrganCode2Num.count(modelinfo_temp_.organ_code);
			if (find_count_ != 0) {

				multimap<int, CimHaveNum>::iterator itr_ = m_mOrganCode2Num.find(modelinfo_temp_.organ_code);
				for (int i = 0; i < find_count_; ++i) {

					if (itr_->second.s_type == modelinfo_temp_.type) {
						++itr_->second.s_num;
						find_flag_ = 1;
						break;
					} // 

					++itr_;
				} // 
			} // 

			if ((find_flag_ == 0)
				||
				(find_count_ == 0) ) {
				CimHaveNum CimHaveNum_temp;
				memset(&CimHaveNum_temp, 0x00, sizeof(CimHaveNum));
				CimHaveNum_temp.organ_code = modelinfo_temp_.organ_code;
				CimHaveNum_temp.s_type = modelinfo_temp_.type;
				CimHaveNum_temp.s_num = 1;
				m_mOrganCode2Num.insert(make_pair(modelinfo_temp_.organ_code, CimHaveNum_temp));
			} // 
		}
	}
	else {
		return -1;
	}

	if(rec_num > 0)
		d5000.g_CDci.FreeReadData(attrs,attr_num,buf);
	else
		d5000.g_CDci.FreeColAttrData(attrs, attr_num);

	//Printm_mOrganCode2Num();

#if 0
	/*cout << "fz:" << fz_vector.size() << "\txm:" << xm_vector.size() << "\tnd:" << nd_vector.size() << "\npt:" << pt_vector.size()
		<< "\tqz:" << qz_vector.size() << "\tzz:" << zz_vector.size() << "\tly:" << ly_vector.size() << "\tsm:" << sm_vector.size()
		<< "\tnp:" << np_vector.size() << endl;*/
	/// @detail ����֮�� */
	cout << "fz:" << m_mOrganCode2Num[35401]+ m_mOrganCode2Num[3540107] + m_mOrganCode2Num[3540108] + m_mOrganCode2Num[3540109] 
		+ m_mOrganCode2Num[3540110] + m_mOrganCode2Num[3540111] + m_mOrganCode2Num[3540112] + m_mOrganCode2Num[3540113] 
		+ m_mOrganCode2Num[3540114] 
		<< "\txm:" << m_mOrganCode2Num[35402] 
		<< "\tnd:" << m_mOrganCode2Num[35403] + m_mOrganCode2Num[3540302] + m_mOrganCode2Num[3540303] + m_mOrganCode2Num[3540304] 
		+ m_mOrganCode2Num[3540305] + m_mOrganCode2Num[3540306] + m_mOrganCode2Num[3540307] + m_mOrganCode2Num[3540308] 
		+ m_mOrganCode2Num[3540309]
		<< "\tpt:" << m_mOrganCode2Num[35404] + m_mOrganCode2Num[3540404]
		<< "\tqz:" << m_mOrganCode2Num[35405] + m_mOrganCode2Num[3540506] + m_mOrganCode2Num[3540507] 
		+ m_mOrganCode2Num[3540508] + m_mOrganCode2Num[3540509] + m_mOrganCode2Num[3540510] + m_mOrganCode2Num[3540511] 
		+ m_mOrganCode2Num[3540512]
		<< "\tzz:" << m_mOrganCode2Num[35406] + m_mOrganCode2Num[3540603] + m_mOrganCode2Num[3540604] + m_mOrganCode2Num[3540605]
		+ m_mOrganCode2Num[3540606] + m_mOrganCode2Num[3540608] + m_mOrganCode2Num[3540609] + m_mOrganCode2Num[3540610]
		+ m_mOrganCode2Num[3540611] + m_mOrganCode2Num[3540612] 
		<< "\tly:" << m_mOrganCode2Num[35407] + m_mOrganCode2Num[3540704] + m_mOrganCode2Num[3540705] 
		+ m_mOrganCode2Num[3540706] + m_mOrganCode2Num[3540707] + m_mOrganCode2Num[3540708] 
		+ m_mOrganCode2Num[3540709] + m_mOrganCode2Num[3540710]
		<< "\tsm:" << m_mOrganCode2Num[35408] + m_mOrganCode2Num[3540802] + m_mOrganCode2Num[3540805] 
		+ m_mOrganCode2Num[3540806] + m_mOrganCode2Num[3540807] + m_mOrganCode2Num[3540808] + m_mOrganCode2Num[3540809] 
		+ m_mOrganCode2Num[3540810] + m_mOrganCode2Num[3540811] + m_mOrganCode2Num[3540812] + m_mOrganCode2Num[3540813]
		<< "\tnp:" << m_mOrganCode2Num[35409] + m_mOrganCode2Num[3540903] + m_mOrganCode2Num[3540905] 
		+ m_mOrganCode2Num[3540906] + m_mOrganCode2Num[3540907] + m_mOrganCode2Num[3540908] + m_mOrganCode2Num[3540909] 
		+ m_mOrganCode2Num[3540910] + m_mOrganCode2Num[3540911] + m_mOrganCode2Num[3540912] + m_mOrganCode2Num[3540913]
		<< endl;
#endif

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
	char *startpos = NULL,*Cimendpos = NULL,*typepos = NULL,*endpos = NULL;

	memset(path_name,0x00,sizeof(path_name));
	char timenow[100];
	char monthday[36];
	time_t time_temp= time(NULL);
	struct tm *tm_temp = localtime(&time_temp);
	memset(timenow,0x00,sizeof(timenow));
	strftime(timenow,sizeof(timenow),"%Y-%m-%d",tm_temp);
	memset(monthday, 0x00, sizeof(monthday));
	strftime(monthday, sizeof(monthday),"%m%d", tm_temp);

	strcpy(path_name,g_config.cime_path);

	if((dirp = opendir(path_name)) == NULL) {
		perror("open dir error!");
		return -1;
	}

	while((dirt = readdir(dirp)) != NULL) {

		if (strcmp(dirt->d_name,".")==0 || strcmp(dirt->d_name,"..") == 0)
			continue; 

		strncpy(file_name,dirt->d_name,sizeof(file_name));
		len = strlen(file_name);
		/* // @detail DMS�ع�˾ģ��ͨ���о�ת�������ڲ��԰��������оַ��������ڶ����賿�������ղ��԰�������ϵͳ����������հ���ֻ��������zip�� */
		if((strcmp(file_name+len-3,"zip") == 0)
		&&
		(strstr(file_name,monthday) != NULL) ) {

			if(strstr(file_name,"3540") != NULL ) { /*ֻ��������3540ѹ����*/
				startpos = strstr(file_name,"AllModel"); /*��ģ��*/
				Cimendpos = strstr(file_name, "CIM"); /*��ģ��*/
				typepos = strstr(file_name, "up");

				if ((typepos == NULL)
					&&
					startpos != NULL) { /*��ģ��*/
					startpos = strchr(file_name, '_'); 
					memset(code, 0, sizeof(code));
					strncpy(code, file_name, startpos - file_name);
					unzip_fun(file_name, atoi(code), _OLD_);
				} // 
				else if ((typepos != NULL)
					&&
					(Cimendpos != NULL)) { /*��ģ��*/

					startpos = strchr(file_name, '_');
					if (startpos != NULL) {
						
						endpos = strchr(startpos+1, '_');
						if (endpos != NULL) {
							memset(code, 0, sizeof(code));
							strncpy(code, startpos + 1, endpos - startpos - 1);
							unzip_fun(file_name, atoi(code), _NEW_);
						} // 
					} // 
				}
			}
		}

		sprintf(oldname,"%s/%s",g_config.cime_path,file_name);
		sprintf(newname,"%s/%s/%s",g_config.back_path,timenow,file_name);
		rename(oldname,newname);
		i++;
	}

	closedir(dirp);

	return 0;
}

void mvcim::Printm_mOrganCode2Num()
{
	multimap<int, CimHaveNum>::iterator itr_ = m_mOrganCode2Num.begin();
	for (; itr_ != m_mOrganCode2Num.end();++itr_) {
		printf("Organ_Code:%d,_type:%d,_num:%d\n", itr_->first, itr_->second.s_type, itr_->second.s_num);
	} // 
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

//��ʼ����xml�ļ�
int resolvexmldata::readdata_xml(const char* file,logstr& threadlog)
{
	const char *xmlfile = file;
	/*char tempname[250];

	memset(tempname,0x00,sizeof(tempname));

	sprintf(tempname,"%s/%s",g_config.path_name,xmlfile);*/
	TiXmlDocument doc;

#if 0
#ifndef _DMSZTGJDET
	/* // @detail Сˮ����ϸ����û���ô������ļ��ϴ󣬼��ء�����ռ����Դ���ʲ����������� */
	if (strstr(file,"xsdcltratedet_ztgjdet") != NULL) {
		mvxmlfile(xmlfile, 1);
		return 0;
	} // 
#endif
#endif
	/* // @detail ���������ļ����������� */
	if (IsFilterFile(file) != 0) {
		mvxmlfile(xmlfile, 1);
		return 0;
	} // 

	//tinyxml�����ļ�
	if (doc.LoadFile(xmlfile,TIXML_ENCODING_LEGACY)) {
		SetGError(1,0,xmlfile,-1,"���سɹ�.",threadlog);
		//SetGError(1,0,tempname,"���سɹ�.",threadlog);
		Log("Load file %s success.\n" ,xmlfile);
		import_xml(doc,file,threadlog);
	}
	else {
		SetGError(2,0,xmlfile,-1,"�����ļ�ʧ��,�ļ���ʽ����ȷ.",threadlog);
		Log("Load file %s fail.%d rec %d col\n" ,xmlfile,doc.ErrorRow(),doc.ErrorCol());
		mvxmlfile(xmlfile,1);
	}

	GetGError(xmlfile,threadlog);

	return 0;
}

//����cime���ͣ���ԭ�ļ����������ƶ���back�ļ���
int mvcimefile(char * filename,int type,int sys)
{
	if(filename == NULL)
		return -1;

	int ret = 0;
	char newfile[256];
	char oldfile[256];
	memset(newfile,0x00,sizeof(newfile));
	memset(oldfile,0x00,sizeof(oldfile));

	//���ļ��Ƴ�������Ŀ¼
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

//��ʼ����xml/cime��data�ļ�
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

	for (enumFlag=_ZDZX_;enumFlag<_UnKnownFileType_;++enumFlag) {

		if(strstr(tempname,g_strfiletypeName[enumFlag])!=NULL) {
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
	case _DMSCIME_://�洢������кż���С���к�
		i = atoi(strstr(tempname, "-") + 1);
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
		
		//�ж��Ƿ�����cime�ļ�
		/*if(min_dmscime != -1)
			remvcime(1);
		if(min_gpmscime != -1)
			remvcime(2);*/

		for (i=min_dmscime;i<=max_dmscime;++i) {
			sprintf(full_des_path,"%d%d%d.DMSCIME",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);
			sprintf(full_src_path,"%d%d%d.DMSCIME-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);

			if(Combination_cime(full_src_path,full_des_path,1) == 0) {
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,0);
			}
			else {
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,0);
			}
		}

		for (i=min_dmsdata;i<=max_dmsdata;++i) {
			sprintf(full_src_path,"%d%d%d.DMSDATA-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.DMSDATA",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);

			if(Combination_cime(full_src_path,full_des_path,2) == 0) {
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,0);
			}
			else {
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,0);
			}
		}

		for (i=min_gpmscime;i<=max_gpmscime;++i) {
			sprintf(full_src_path,"%d%d%d.GPMSCIME-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.GPMSCIME",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);

			if(Combination_cime(full_src_path,full_des_path,3) == 0) {
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,1);
			}
			else {
				SetGError(2,-1,full_src_path,-1,"Read Fail.",threadlog);
				mvcimefile(full_src_path,1,1);
			}
		}
		for (i=min_gpmsdata;i<=max_gpmsdata;++i) {
			sprintf(full_src_path,"%d%d%d.GPMSDATA-%d",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday,i);
			sprintf(full_des_path,"%d%d%d.GPMSDATA",1900+tm_now->tm_year,1+tm_now->tm_mon,tm_now->tm_mday);

			if(Combination_cime(full_src_path,full_des_path,4) == 0) {
				SetGError(1,-1,full_src_path,-1,"Read Success.",threadlog);
				insertfile(full_src_path);
				mvcimefile(full_src_path,0,1);
			}
			else {
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

//�ϲ�CIME�ļ�
int resolvexmldata::Combination_cime(const char *src_file,const char *des_file,int flag)
{
	long int read_len = 0,all_read_len = 0,one_read_len = 0,write_len = 0,all_write_len = 0,all_len = 0,lef_len = 0;
	struct stat src_file_stat;
	FILE *fp_read = NULL,*fp_write = NULL;
	char *temp_buf;
	char temp_des_file[250];
	char temp_src_file[250];

	switch(flag) {
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
	if (temp_buf == NULL) {
		perror("malloc");
		return -1;
	}

	fp_read = fopen(temp_src_file,"r");
	fp_write = fopen(temp_des_file,"a+");
	if (fp_read == NULL || fp_write == NULL) {
		free(temp_buf);
		temp_buf = NULL;
		perror("open");
		return -1;
	}

	if(stat(temp_src_file,&src_file_stat) == -1) {
		free(temp_buf);
		temp_buf = NULL;
		perror("stat");
		return -1;
	}

	all_len = src_file_stat.st_size;
	while(write_len < all_len) {
		memset(temp_buf,0x00,_2M_);
		lef_len = all_len - write_len;
		read_len = (_2M_ < lef_len)?_2M_:lef_len;

		one_read_len = fread(temp_buf,sizeof(char),read_len,fp_read);
		if (one_read_len > 0) {
			fflush(fp_read);
			all_read_len += one_read_len;
		}
		else {
			free(temp_buf);
			temp_buf = NULL;
			perror("read");
			return -1;
		}

		write_len = fwrite(temp_buf,sizeof(char),one_read_len,fp_write);
		if (write_len > 0) {
			fflush(fp_write);
			all_write_len += write_len;
		}
		else {
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

	if(temp_bom[0] == (-17) && temp_bom[1] == (-69) && temp_bom[2] == (-65))//ȥbom
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

int resolvexmldata::ConvertGDCode2OrganCode(int _src, int *_des)
{
	if (_src == 1000001) {
		*_des = 35401;
	} // 
	else if (_src == 1000002) {
		*_des = 35404;
	} // 
	else if (_src == 1000003) {
		*_des = 35405;
	} // 
	else if (_src == 1000004) {
		*_des = 35402;
	} // 
	else if (_src == 1000005) {
		*_des = 35406;
	} // 
	else if (_src == 1000006) {
		*_des = 35407;
	} // 
	else if (_src == 1000007) {
		*_des = 35408;
	} // 
	else if (_src == 1000008) {
		*_des = 35409;
	} // 
	else if (_src == 1000009) {
		*_des = 35403;
	} // 

	return 0;
}

int resolvexmldata::convertUTF8toANSI(const char *inbuf, char *outbuf, int outlen)
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

//��xml���ݴ洢���ڴ�
int resolvexmldata::import_xml(TiXmlDocument &doc,const char *filename,logstr& threadlog)
{
	int enumFlag,retCode;
	int step_temp = 0;
	int stadagc = 0;// Ϊdagc����������ñ�����ֻ�е�һ�����
	char da_temp[30];
	char gtime_temp[30];
	mes_header header;
	header_sign s_header;

	TiXmlElement *requestElement,*headElement;
	TiXmlAttribute *attributeOfhead;

	//enum xmltype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_UnKnownFileType_,_GPZW_DMS_,_GPZW_GPMS_,_LOG_}g_xmltype;
	//const char *g_xmltypeName[] = {"ZZZX","DATT","DAGC","YXBW","GPZW","ZDZX","YKCZ","YXINFO"};
	enum xmltype {
		_ZZZX_,
		_DATT_,
		_DAGC_,
		_YXBW_,
		_GPZW_,
		_DMSGPZW_,
		_GPMSGPZW_,
		_ZDZX_,
		_YKCZ_,
		_YXINFO_,
		_RATE_,
		_BUS_,
		_CB_,
		_DSC_,
		_TRANS_,
		_SUBS_,
		_TDTRANS_,
		_AUTOMAP_,
		_TQREALEASE_,
		_FASTR_,
		_TZLIST_,
		_GPMSTD_,
		_GPMSTDDET_,
		_GPMSZLPDET_,
		_DMSZLPDET_,
		_DMSXSD_,
		_DMSYD_,
		_GPMSYD_,
		_GZZSQ_,
		_DMSYCRT_,
		_DMSEMSRT_,
		_DMSYXRT_,
		_DMSLINE_,
		_DMSXSDET_,
		_DMSZTGJDET_,
		_DMSTRANSUP_, /*������(GIS1.6)���*/
		_DMSCBUP_,	/*������(GIS1.6)����*/
		_DMSDSCUP_,	/*������(GIS1.6)��բ*/
		_DMSUBSUP_,	/*������(GIS1.6)վ��*/
		_DMSBUSUP_,	/*������(GIS1.6)ĸ��*/
		_DMSDEVCORR_,	/*����ǰ(GIS1.5)��������(GIS1.6)�豸ID��Ӧ̨��*/
		_GOMSEND_, /*GOMS���͹����������ݼ�¼*/
		_SMNDATA_, /*�ܿ�ƽ̨�������ݼ�¼*/
		_SMNDATADET_, /*�ܿ�ƽ̨δ�������ݴ�����ϸ*/
		_YXBS_, /*ң�ű�ʶ�����豸*/
		_END_,
		_LOG_,
		_UnKnownFileType_
	}g_xmltype;

	const char *g_xmltypeName[] = 
	{
		"ZZZX",
		"DATT",
		"DAGC",
		"YXBW",
		"GPZW",
		"DMSGPZW",
		"GPMSGPZW",
		"ZDZX",
		"YKCZ",
		"YXINFO",
		"dmsindex",
		"buslist",
		"cblist",
		"dsclist",
		"translist",
		"subslist",
		"tdtrans",
		"automap",
		"tdrelease",
		"fastr",
		"tzlist",
		"gpmstd",
		"gpmstddet",
		"gpmszlpdet",
		"zlp",
		"wkwh",
		"dmsyd",
		"gpmsydddet",
		"GZZSQ",
		"YCRHT",
		"EMSRHT",
		"ztgj",
		"line",
		"xsdcltratedet",
		"ztgjdet",
		"translistup",
		"cblistup",
		"dsclistup",
		"subslistup",
		"buslistup",
		"devlistcorr",
		"gomsend",
		"smnpush",
		"smnpushdet",
		"yxbs",
		"END",
		"LOG"
	}; /*�ļ���type�ֶ�*/

	requestElement = doc.RootElement(); //request

	headElement = requestElement->FirstChildElement();//head

	for (;headElement != NULL;headElement = headElement->NextSiblingElement()) {

		if (strcasecmp(headElement->Value(),"head") == 0) {
			memset(&header,0,sizeof(header));
			memset(&s_header,0,sizeof(s_header));
			/*time_t temp_time;
			struct tm *temp_tm;
			temp_time = time(NULL)-86400;
			temp_tm = localtime(&temp_time);
			sprintf(header.count,"%d-%d-%d",1900+temp_tm->tm_year,1+temp_tm->tm_mon,temp_tm->tm_mday);*/

			attributeOfhead = headElement->FirstAttribute();
			for(;attributeOfhead != NULL;attributeOfhead = attributeOfhead->Next()) {
				const char *attname = attributeOfhead->Name();
				const char *attvalue = attributeOfhead->Value();

				if(strcasecmp(attname,"code") == 0) {
					header.code = atoi(attvalue);
					SetGError(1,header.code,NULL,-1,NULL,threadlog);
				}
				else if(strcasecmp(attname,"source") == 0)
					strcpy(s_header.source,attvalue);
				else if(strcasecmp(attname,"type") == 0) {
					strcpy(s_header.type,attvalue);
				}
				else if(strcasecmp(attname,"sequence") == 0) {
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
		else if (strcasecmp(headElement->Value(),"body") == 0) { //body  

			for (enumFlag=_ZZZX_;enumFlag<_UnKnownFileType_;enumFlag++) {
				//printf("enumFlag:%s\n",g_xmltypeName[enumFlag]);
				//printf("type:%s\n",s_header.type);
				if(strcmp(g_xmltypeName[enumFlag],s_header.type) == 0) {
					g_xmltype = (enum xmltype)enumFlag;
					break;
				}
			}

			if(enumFlag == _UnKnownFileType_)
				g_xmltype = (enum xmltype)enumFlag;

			switch(g_xmltype) {
			case _ZZZX_:
				//if(strcasecmp(s_header.type,"ZZZX") == 0)
				//{
				dmsop_vector.clear();
				dmsop dmsop_temp;
				TiXmlElement *thrElement_zzzx;

				thrElement_zzzx = headElement->FirstChildElement();
				for (;thrElement_zzzx != NULL;thrElement_zzzx = thrElement_zzzx->NextSiblingElement()) {
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
				for (;thrElement_datt != NULL;thrElement_datt = thrElement_datt->NextSiblingElement()) {

					attributeOfthr_datt = thrElement_datt->FirstAttribute();
					for(;attributeOfthr_datt != NULL;attributeOfthr_datt = attributeOfthr_datt->Next()) {
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
				if(stadagc == 0) {
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

				for (;thrElement_dagc != NULL;thrElement_dagc = thrElement_dagc->NextSiblingElement()) {
					memset(&dagc_temp,0x00,sizeof(dagc_temp));
					attributeOfthr_dagc = thrElement_dagc->FirstAttribute();

					for(;attributeOfthr_dagc != NULL;attributeOfthr_dagc = attributeOfthr_dagc->Next()) {
						//cout << attributeOfthr_dagc->Name() << ":" << attributeOfthr_dagc->Value() << endl;
						const char *attname = attributeOfthr_dagc->Name();
						const char *attvalue = attributeOfthr_dagc->Value();
						//memcpy_struct(header,dagc_temp.header);
						memcpy(&dagc_temp.header,&header,sizeof(header));

						if(strcasecmp(attname,"fault") == 0) {
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
							else if (strcasecmp(attname,"type") == 0) { //zgs20160319

								zdzx_temp.type = atoi(attvalue);
							}
							else if (strcasecmp(attname,"producer") == 0) { //zgs20160319

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
					
					//У��̨��ƥ���ʡ��ɼ��ɹ�����ֵ�Ƿ�Ϊ�� add by lcm 20150910
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'̨��ƥ������ֵΪ��','dms');");
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'�ɼ��ɹ�����ֵΪ��','dms');");
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
							else if (strcasecmp(attname, "tqh") == 0) {
								/* // @detail ������Ϊ�ո����� */
								if (strcmp(attvalue," ") != 0) { 
									strncpy(trans_temp.tqh, attvalue, sizeof(trans_temp.tqh));
								} // 
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
					subs_vector.clear();
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
							subs_vector.push_back(subs_temp);
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
							else if (strcasecmp(attname, "ypid") == 0)
								strncpy(tqrelease_temp.ypid, attvalue, sizeof(tqrelease_temp.ypid));
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

						if (tqrelease_temp.sendtype == 2) {
							strncpy(tqrelease_temp.ddaff,tqrelease_temp.dmstd,sizeof(tqrelease_temp.ddaff));
							strncpy(tqrelease_temp.dmsaff,tqrelease_temp.dmstd,sizeof(tqrelease_temp.dmsaff));
						}

						if(strlen(tqrelease_temp.tdintnr) == 0) {
							null_flag = 1;
						}
#if 0
						/* // @detail ȡ���ϲ�������Ϊ���GPMS���� */
						/* // @detail GPMS�������������������繫˾��������ҹ��繫˾ͣ����Ϣ�ϲ���
							DMS���⴦�����������������������繫˾�ϲ����������ҹ��繫˾*/
						if (tqrelease_temp.header.code == 3540704) {
							tqrelease_temp.header.code = 35407;
						} // 
#endif

						tqrelease_vector.push_back(tqrelease_temp);
					}
					//У��dmsͣ����Ϣ�ļ��Ƿ����ͣ�緶ΧΪ�ռ�¼ add by lcm 20150910
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'���ļ�����ͣ�緶ΧΪ�ռ�¼','dms');");
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
							else if (strcasecmp(attname, "dmsid") == 0) {
								strncpy(temp_gpmstddet.dmsid, attvalue, sizeof(temp_gpmstddet.dmsid));
							}
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
						/*if(strcasecmp(temp_gpmstddet.disdesk,"DDT_NP_NP") == 0 && strcasecmp(temp_gpmstddet.belarea,"3950A505-7FF2-4AAC-9970-C4E5D4E73796") == 0)
							strncpy(temp_gpmstddet.disdesk,"DDT_NP_YP",sizeof(temp_gpmstddet.disdesk));*///��ƽͣ��������е���̨����
						//����Ͻ���ø�ֵ����CODE��NAME
						/* // @detail GPMS���������������������繫˾��������ҹ��繫˾���ϵ��ϲ���DMSδ�ϲ�����Ҫ
						����DMSID�ӹ������ҹ��繫˾���ϵ��в�ֳ��������������������繫˾���ϵ�������DMSID��
						Ϊ"����_"��ʼ*/
						if ((temp_gpmstddet.citycode == 35407)
							&&
							(strstr(temp_gpmstddet.dmsid, "����_") != NULL)) {
							/* // @detail ����Ͻ���� */
							strncpy(temp_gpmstddet.disdesk, "DDT_LY_XL", sizeof(temp_gpmstddet.disdesk) - 1);
						} //
						else if ((temp_gpmstddet.citycode == 35409)
							&&
							(strstr(temp_gpmstddet.dmsid,"��ƽ_") != NULL) ) {
							/* // @detail ����Ͻ���� */
							strncpy(temp_gpmstddet.disdesk, "DDT_NP_YP", sizeof(temp_gpmstddet.disdesk) - 1);
						}
						/* // @detail ���µ���������CODE ΪϽ�����ñ������� */
						multimap<string,disdesk>::iterator it_dis = disdesk_map.find(temp_gpmstddet.disdesk);
						if(it_dis != disdesk_map.end()) {
							temp_gpmstddet.citycode = it_dis->second.organ_code;
							strncpy(temp_gpmstddet.cityname, it_dis->second.area, sizeof(temp_gpmstddet.cityname));
						}

						if(strlen(temp_gpmstddet.tdintr) == 0) {
							null_flag = 1;
						}

						gpmstddet_vector.push_back(temp_gpmstddet);
					}
					//У��gpmsͣ����Ϣ�ļ��Ƿ����ͣ�緶ΧΪ�ռ�¼ add by lcm 20150910
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'���ļ�����ͣ�緶ΧΪ�ռ�¼','yc');");
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
							else if (strcasecmp(attname,"notify_time") == 0)
								strncpy(temp_gpmszlpdet.notify_time,attvalue,sizeof(temp_gpmszlpdet.notify_time));
							else if (strcasecmp(attname,"oprtfinish_time") == 0)
								strncpy(temp_gpmszlpdet.oprtfinish_time,attvalue,sizeof(temp_gpmszlpdet.oprtfinish_time));
							else if (strcasecmp(attname,"check_time") == 0)
								strncpy(temp_gpmszlpdet.check_time,attvalue,sizeof(temp_gpmszlpdet.check_time));
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
							else if(strcasecmp(attname,"send_time") == 0)
								strncpy(temp_gpmsyd.send_time,attvalue,sizeof(temp_gpmsyd.send_time));
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
								strncpy(temp_dmszlpdet.order_time,attvalue,sizeof(temp_dmszlpdet.order_time));
							else if (strcasecmp(attname,"confirm_time") == 0)
								strncpy(temp_dmszlpdet.confirm_time,attvalue,sizeof(temp_dmszlpdet.confirm_time));
							else if (strcasecmp(attname,"check_time") == 0)
								strncpy(temp_dmszlpdet.check_time,attvalue,sizeof(temp_dmszlpdet.check_time));
							else if (strcasecmp(attname,"notify_time") == 0)
								strncpy(temp_dmszlpdet.notify_time,attvalue,sizeof(temp_dmszlpdet.notify_time));
							else if (strcasecmp(attname,"oprtfinish_time") == 0)
								strncpy(temp_dmszlpdet.oprtfinish_time,attvalue,sizeof(temp_dmszlpdet.oprtfinish_time));
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
							else if (strcasecmp(attname,"xsd_num") == 0) {

								temp_dmsxsd.xsd_num = atoi(attvalue);
							}
							else if (strcasecmp(attname,"sendtime") == 0) {

								strncpy(temp_dmsxsd.sendtime,attvalue,sizeof(temp_dmsxsd.sendtime)-1);
							}
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						dmsxsd_vector.push_back(temp_dmsxsd);
					}
				}
				break;
			/*case _ZTGJ_:
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
				break;*/
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
							{
								strncpy(temp_dmsyd.id,attvalue,sizeof(temp_dmsyd.id));
							}
							else if(strcasecmp(attname,"status") == 0)
								temp_dmsyd.status = atoi(attvalue);
							else if(strcasecmp(attname,"rollback_time") == 0)
								temp_dmsyd.rollback_time = atoi(attvalue);
 							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						if (strstr(temp_dmsyd.id,"LS") != NULL) { //�޳������춯

							dmsyd_vector.push_back(temp_dmsyd);
						}
					}
				}
				break;
			case _GZZSQ_:
				{
					gzzsq_vector.clear();
					gzzsq gzzsq_temp;
					float x = 0.0;
					const float float_border = 0.000001;
					//gzzsq_temp.xsd_rate = 0.0;
					TiXmlElement *thrElement_rate;
					thrElement_rate = headElement->FirstChildElement();
					for(;thrElement_rate != NULL;thrElement_rate = thrElement_rate->NextSiblingElement())
					{
						const char * elvalue = thrElement_rate->Value();
						const char * eltext = thrElement_rate->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&gzzsq_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"cover_rate") == 0)
							gzzsq_temp.cover_rate = atof(eltext);
						else if(strcasecmp(elvalue,"shake_rate") == 0)
							gzzsq_temp.shake_rate = atof(eltext);
						else if(strcasecmp(elvalue,"misoperation_rate") == 0)
							gzzsq_temp.misoperation_rate = atof(eltext);
						else if(strcasecmp(elvalue,"missreport_rate") == 0)
							gzzsq_temp.missreport_rate = atof(eltext);
						else if (strcasecmp(elvalue,"tworatiocover_rate") == 0) {

							gzzsq_temp.tworatiocover_rate = atof(eltext);
						}
						else if (strcasecmp(elvalue,"threeratiocover_rate") == 0) {

							gzzsq_temp.threeratiocover_rate = atof(eltext);
						}

						else
							cout << "Unknown :" << elvalue << endl;
					}

					//У��̨��ƥ���ʡ��ɼ��ɹ�����ֵ�Ƿ�Ϊ�� add by lcm 20150910
					if(fabs(gzzsq_temp.cover_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'����ָʾ����������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
					if(fabs(gzzsq_temp.shake_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'����ָʾ����������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
					if(fabs(gzzsq_temp.misoperation_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'����ָʾ��������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}
					if(fabs(gzzsq_temp.missreport_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'����ָʾ��©������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					if(fabs(gzzsq_temp.tworatiocover_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'��ң�ն˸�������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					if(fabs(gzzsq_temp.threeratiocover_rate)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'��ң�ն˸�������ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					gzzsq_vector.push_back(gzzsq_temp);
				}
				break;
			case _DMSYCRT_:
				{
					ycrt_vector.clear();
					ycrt ycrt_temp;
					float x = 0.0;
					const float float_border = 0.000001;
					//gzzsq_temp.xsd_rate = 0.0;
					TiXmlElement *thrElement_rate;
					thrElement_rate = headElement->FirstChildElement();
					for(;thrElement_rate != NULL;thrElement_rate = thrElement_rate->NextSiblingElement())
					{
						const char * elvalue = thrElement_rate->Value();
						const char * eltext = thrElement_rate->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&ycrt_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"yc_right") == 0)
							ycrt_temp.yc_right = atof(eltext);
						else
							cout << "Unknown :" << elvalue << endl;
					}

					//У��̨��ƥ���ʡ��ɼ��ɹ�����ֵ�Ƿ�Ϊ�� add by lcm 20150910
					if(fabs(ycrt_temp.yc_right)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'ң����ȷ����ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					ycrt_vector.push_back(ycrt_temp);
				}
				break;
			case _DMSEMSRT_:
				{
					emsrt_vector.clear();
					emsrt emsrt_temp;
					float x = 0.0;
					const float float_border = 0.000001;
					//gzzsq_temp.xsd_rate = 0.0;
					TiXmlElement *thrElement_rate;
					thrElement_rate = headElement->FirstChildElement();
					for(;thrElement_rate != NULL;thrElement_rate = thrElement_rate->NextSiblingElement())
					{
						const char * elvalue = thrElement_rate->Value();
						const char * eltext = thrElement_rate->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&emsrt_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"ems_zf_right") == 0)
							emsrt_temp.emszfrt = atof(eltext);
						else
							cout << "Unknown :" << elvalue << endl;
					}

					//У��̨��ƥ���ʡ��ɼ��ɹ�����ֵ�Ƿ�Ϊ�� add by lcm 20150910
					if(fabs(emsrt_temp.emszfrt)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'EMSת����ȷ����ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					emsrt_vector.push_back(emsrt_temp);
				}
				break;
			case _DMSYXRT_:
				{
				/* // @detail ɾ��ԭ��ң��״̬��ʶ�ʽ���ָ�����ݣ�����Ϊһ����������״̬��ʶ������ϸ���� */
#if 0
					yxrt_vector.clear();
					yxrt yxrt_temp;
					float x = 0.0;
					const float float_border = 0.000001;
					//gzzsq_temp.xsd_rate = 0.0;
					TiXmlElement *thrElement_rate;
					thrElement_rate = headElement->FirstChildElement();
					for(;thrElement_rate != NULL;thrElement_rate = thrElement_rate->NextSiblingElement())
					{
						const char * elvalue = thrElement_rate->Value();
						const char * eltext = thrElement_rate->GetText();

						//memcpy_struct(yxinfo_temp.header,header);
						memcpy(&yxrt_temp.header,&header,sizeof(header));
						if(strcasecmp(elvalue,"ztgjvalue") == 0)
							yxrt_temp.yx_right = atof(eltext);
						else
							cout << "Unknown :" << elvalue << endl;
					}

					//У��̨��ƥ���ʡ��ɼ��ɹ�����ֵ�Ƿ�Ϊ�� add by lcm 20150910
					if(fabs(yxrt_temp.yx_right)<=float_border)
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
							"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,:4,'ң��״̬��ʶ��ȷ����ֵΪ��','dms');");
						if(d5000.d5000_WriteData("DMSCBL_IMAGE",qury,temp_err,1,4,attrs,error_info) != 0)
						{
							return -1;
						}

						free(temp_err);
						free(attrs);
						attrs = NULL;
						temp_err = NULL;
					}

					yxrt_vector.push_back(yxrt_temp);
#endif
					ztgjfromone_vector.clear();
					ztgjfromone ztgjfromone_temp;
					TiXmlElement *thrElement_ztgjfromone;
					TiXmlAttribute *attributeOfthr_ztgjfromone;
					thrElement_ztgjfromone = headElement->FirstChildElement(); /*body*/
					for (; thrElement_ztgjfromone != NULL; 
						thrElement_ztgjfromone = thrElement_ztgjfromone->NextSiblingElement()) {
						const char * elvalue = thrElement_ztgjfromone->Value();
						const char * eltext = thrElement_ztgjfromone->GetText();

						if ( (elvalue == NULL)
						||
						( (strcasecmp(elvalue, "detail") != 0)
						&&
						(eltext == NULL) ) ) {
							continue;
						} // 

						if (strcasecmp(elvalue,"cbid") == 0) {
							memset(&ztgjfromone_temp, 0x00, sizeof(ztgjfromone));
							memcpy(&ztgjfromone_temp.header, &header, sizeof(header));

							strncpy(ztgjfromone_temp.devid, eltext, sizeof(ztgjfromone_temp.devid) - 1);
						} // 
						else if (strcasecmp(elvalue, "cbname") == 0) {
							strncpy(ztgjfromone_temp.devname, eltext, sizeof(ztgjfromone_temp.devname) - 1);
						} // 
						else if (strcasecmp(elvalue, "feederid") == 0) {
							strncpy(ztgjfromone_temp.feederid, eltext, sizeof(ztgjfromone_temp.feederid) - 1);
						} // 
						else if (strcasecmp(elvalue, "feedername") == 0) {
							strncpy(ztgjfromone_temp.feedername, eltext, sizeof(ztgjfromone_temp.feedername) - 1);
						} // 
						else if (strcasecmp(elvalue, "type") == 0) {
							ztgjfromone_temp.type = atoi(eltext);
						} // 
						else if (strcasecmp(elvalue, "detail") == 0) {
							ztgjfromone_vector.push_back(ztgjfromone_temp);
						} // 
						else if (strcasecmp(elvalue, "resultime") == 0) {

							vector<ztgjfromone>::iterator itr_ = ztgjfromone_vector.begin();
							for (; itr_ != ztgjfromone_vector.end();++itr_) {
								strncpy(itr_->result_time, eltext, sizeof(itr_->result_time) - 1);
							} // 
						} // 
					}
				}
				break;
			case _DMSLINE_:
				{
					lines_vector.clear();
					lines lines_temp;
					TiXmlElement *thrElement_lines;
					TiXmlAttribute *attributeOfthr_lines;
					thrElement_lines = headElement->FirstChildElement();
					for (;thrElement_lines != NULL;thrElement_lines = thrElement_lines->NextSiblingElement())
					{
						memset(&lines_temp,0,sizeof(lines));
						attributeOfthr_lines = thrElement_lines->FirstAttribute();
						for(;attributeOfthr_lines != NULL;attributeOfthr_lines = attributeOfthr_lines->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_lines->Name();
							const char *attvalue = attributeOfthr_lines->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&lines_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"line") == 0)
								strncpy(lines_temp.lineid,attvalue,sizeof(lines_temp.lineid));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(lines_temp.linename,attvalue,sizeof(lines_temp.linename));
							else if(strcasecmp(attname,"subsid") == 0) {
							}
							else if(strcasecmp(attname,"subname") == 0) {
							}
							else if(strcasecmp(attname,"vol") == 0)
								strncpy(lines_temp.vol,attvalue,sizeof(lines_temp.vol));
							else if (strcasecmp(attname,"indicator") == 0) {
	
								lines_temp.indicator = atoi(attvalue);
							}
							else if (strcasecmp(attname,"tworatio") == 0) {

								lines_temp.tworatio = atoi(attvalue);
							}
							else if (strcasecmp(attname,"threeratio") == 0) {

								lines_temp.threeratio = atoi(attvalue);
							}
							else
								cout << "Unknown attribute:" << attvalue << endl;
						}
						lines_vector.push_back(lines_temp);
					}
				}
				break;
			case _DMSXSDET_:
				{
					dmsxsdet_vector.clear();
					dmsxsdet dmsxsdet_temp;
					TiXmlElement *thrElement_dmsxsdet;
					TiXmlAttribute *attributeOfthr_dmsxsdet;
					thrElement_dmsxsdet = headElement->FirstChildElement();
					for (;thrElement_dmsxsdet != NULL;thrElement_dmsxsdet = thrElement_dmsxsdet->NextSiblingElement())
					{
						memset(&dmsxsdet_temp,0,sizeof(dmsxsdet));
						attributeOfthr_dmsxsdet = thrElement_dmsxsdet->FirstAttribute();
						for(;attributeOfthr_dmsxsdet != NULL;attributeOfthr_dmsxsdet = attributeOfthr_dmsxsdet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_dmsxsdet->Name();
							const char *attvalue = attributeOfthr_dmsxsdet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&dmsxsdet_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"id") == 0)
								strncpy(dmsxsdet_temp.id,attvalue,sizeof(dmsxsdet_temp.id));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(dmsxsdet_temp.name,attvalue,sizeof(dmsxsdet_temp.name));
							else {}
						}
						dmsxsdet_vector.push_back(dmsxsdet_temp);
					}
				}
				break;
			case _DMSZTGJDET_:
				{
#ifdef _DMSZTGJDET
					dmsztgjdet_vector.clear();
					ztgjdet dmsztgjdet_temp;
					TiXmlElement *thrElement_dmsztgjdet;
					TiXmlAttribute *attributeOfthr_dmsztgjdet;
					thrElement_dmsztgjdet = headElement->FirstChildElement();
					for (;thrElement_dmsztgjdet != NULL;thrElement_dmsztgjdet = thrElement_dmsztgjdet->NextSiblingElement())
					{
						memset(&dmsztgjdet_temp,0,sizeof(dmsxsdet));
						attributeOfthr_dmsztgjdet = thrElement_dmsztgjdet->FirstAttribute();
						for(;attributeOfthr_dmsztgjdet != NULL;attributeOfthr_dmsztgjdet = attributeOfthr_dmsztgjdet->Next())
						{
							//cout << attributeOfthr->Name() << ":" << attributeOfthr->Value() << endl;

							const char *attname = attributeOfthr_dmsztgjdet->Name();
							const char *attvalue = attributeOfthr_dmsztgjdet->Value();
							//memcpy_struct(datt_temp.header,header);
							memcpy(&dmsztgjdet_temp.header,&header,sizeof(header));
							if(strcasecmp(attname,"id") == 0)
								strncpy(dmsztgjdet_temp.id,attvalue,sizeof(dmsztgjdet_temp.id));
							else if(strcasecmp(attname,"name") == 0)
								strncpy(dmsztgjdet_temp.name,attvalue,sizeof(dmsztgjdet_temp.name));
							else if(strcasecmp(attname,"code") == 0)
								dmsztgjdet_temp.organ_code = atoi(attvalue);
							else {}
						}
						dmsztgjdet_vector.push_back(dmsztgjdet_temp);
					}
#endif
				}
				break;
			case _DMSTRANSUP_:
			{
				m_mNewId2Trans.clear();
				trans trans_temp;
				TiXmlElement *thrElement_trans;
				TiXmlAttribute *attributeOfthr_trans;

				thrElement_trans = headElement->FirstChildElement();
				for (; thrElement_trans != NULL; thrElement_trans = thrElement_trans->NextSiblingElement()) {
					memset(&trans_temp, 0, sizeof(trans));

					attributeOfthr_trans = thrElement_trans->FirstAttribute();
					for (; attributeOfthr_trans != NULL; attributeOfthr_trans = attributeOfthr_trans->Next()) {
						const char *attname = attributeOfthr_trans->Name();
						const char *attvalue = attributeOfthr_trans->Value();

						memcpy(&trans_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "trans") == 0)
							strncpy(trans_temp.transid, attvalue, sizeof(trans_temp.transid));
						else if (strcasecmp(attname, "name") == 0)
							strncpy(trans_temp.transname, attvalue, sizeof(trans_temp.transname));
						else if (strcasecmp(attname, "dvid") == 0)
							strncpy(trans_temp.dvid, attvalue, sizeof(trans_temp.dvid));
						else if (strcasecmp(attname, "dvname") == 0) {
						}
						else if (strcasecmp(attname, "vol") == 0)
							strncpy(trans_temp.vol, attvalue, sizeof(trans_temp.vol));
						else if (strcasecmp(attname, "subsid") == 0) {
						}
						else if (strcasecmp(attname, "subname") == 0) {
						}
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					if (strcasecmp(trans_temp.vol, "0.38") != 0)
						m_mNewId2Trans.insert(make_pair(trans_temp.transid,trans_temp));
				}
			}
				break;
			case _DMSCBUP_:
			{
				m_mNewId2Cb.clear();
				int status = -1;
				cb cb_temp;
				TiXmlElement *thrElement_cb;
				TiXmlAttribute *attributeOfthr_cb;

				thrElement_cb = headElement->FirstChildElement();
				for (; thrElement_cb != NULL; thrElement_cb = thrElement_cb->NextSiblingElement()) {
					memset(&cb_temp, 0, sizeof(cb));
					cb_temp.type = 1;
					cb_temp.flag = 1;

					attributeOfthr_cb = thrElement_cb->FirstAttribute();
					for (; attributeOfthr_cb != NULL; attributeOfthr_cb = attributeOfthr_cb->Next()) {
						const char *attname = attributeOfthr_cb->Name();
						const char *attvalue = attributeOfthr_cb->Value();

						memcpy(&cb_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "cb") == 0)
							strncpy(cb_temp.cbid, attvalue, sizeof(cb_temp.cbid));
						else if (strcasecmp(attname, "name") == 0)
							strncpy(cb_temp.cbname, attvalue, sizeof(cb_temp.cbname));
						else if (strcasecmp(attname, "dvid") == 0)
							strncpy(cb_temp.dvid, attvalue, sizeof(cb_temp.dvid));
						else if (strcasecmp(attname, "status") == 0) {
							status = atoi(attvalue);

							if (status == 10 || status == 50)
								cb_temp.status = 0;
							else if (status == 11 || status == 41)
								cb_temp.status = 1;
							else
								cb_temp.status = atoi(attvalue);
						}
						else if (strcasecmp(attname, "dvname") == 0) {
						}
						else if (strcasecmp(attname, "vol") == 0)
							strncpy(cb_temp.vol, attvalue, sizeof(cb_temp.vol));
						else if (strcasecmp(attname, "subsid") == 0) {

							/*if (strncmp(attvalue + 32, "5107", 4) == 0)
								cb_temp.flag = 0;
							else
							cb_temp.flag = 1;*/
							cb_temp.flag = 1;
						}
						else if (strcasecmp(attname, "subname") == 0) {
						}
						else if (strcasecmp(attname, "remotetype") == 0) {
							cb_temp.remotetype = atoi(attvalue);
						}
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					if (strcasecmp(cb_temp.vol, "0.38") != 0)
						m_mNewId2Cb.insert(make_pair(cb_temp.cbid,cb_temp));
				}
			}
				break;
			case _DMSDSCUP_:
			{
				m_mNewId2Dsc.clear();
				dsc dsc_temp;
				int status = -1;
				TiXmlElement *thrElement_dsc;
				TiXmlAttribute *attributeOfthr_dsc;

				thrElement_dsc = headElement->FirstChildElement();
				for (; thrElement_dsc != NULL; thrElement_dsc = thrElement_dsc->NextSiblingElement()) {
					memset(&dsc_temp, 0, sizeof(dsc));
					dsc_temp.type = 0;
					dsc_temp.flag = 1;

					attributeOfthr_dsc = thrElement_dsc->FirstAttribute();
					for (; attributeOfthr_dsc != NULL; attributeOfthr_dsc = attributeOfthr_dsc->Next()) {
						const char *attname = attributeOfthr_dsc->Name();
						const char *attvalue = attributeOfthr_dsc->Value();

						memcpy(&dsc_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "dsc") == 0)
							strncpy(dsc_temp.dscid, attvalue, sizeof(dsc_temp.dscid));
						else if (strcasecmp(attname, "name") == 0)
							strncpy(dsc_temp.dscname, attvalue, sizeof(dsc_temp.dscname));
						else if (strcasecmp(attname, "dvid") == 0)
							strncpy(dsc_temp.dvid, attvalue, sizeof(dsc_temp.dvid));
						else if (strcasecmp(attname, "status") == 0) {
							status = atoi(attvalue);

							if (status == 10 || status == 50)
								dsc_temp.status = 0;
							else if (status == 11)
								dsc_temp.status = 1;
							else
								dsc_temp.status = atoi(attvalue);
						}
						else if (strcasecmp(attname, "dvname") == 0) {
						}
						else if (strcasecmp(attname, "vol") == 0)
							strncpy(dsc_temp.vol, attvalue, sizeof(dsc_temp.vol));
						else if (strcasecmp(attname, "subsid") == 0) {

							/*if (strncmp(attvalue + 32, "5107", 4) == 0)
								dsc_temp.flag = 0;
							else
								dsc_temp.flag = 1;*/
							dsc_temp.flag = 1;
						}
						else if (strcasecmp(attname, "subname") == 0) {
						}
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					if (strcasecmp(dsc_temp.vol, "0.38") != 0)
						m_mNewId2Dsc.insert(make_pair(dsc_temp.dscid,dsc_temp));
				}
			}
				break;
			case _DMSUBSUP_:
			{
				m_mNewId2Bus.clear();
				subs subs_temp;
				TiXmlElement *thrElement_subs;
				TiXmlAttribute *attributeOfthr_subs;

				thrElement_subs = headElement->FirstChildElement();
				for (; thrElement_subs != NULL; thrElement_subs = thrElement_subs->NextSiblingElement()) {
					memset(&subs_temp, 0, sizeof(subs));

					attributeOfthr_subs = thrElement_subs->FirstAttribute();
					for (; attributeOfthr_subs != NULL; attributeOfthr_subs = attributeOfthr_subs->Next()) {
						const char *attname = attributeOfthr_subs->Name();
						const char *attvalue = attributeOfthr_subs->Value();
						memcpy(&subs_temp.header, &header, sizeof(header));
						if (strcasecmp(attname, "subs") == 0)
							strncpy(subs_temp.subsid, attvalue, sizeof(subs_temp.subsid));
						else if (strcasecmp(attname, "name") == 0)
							strncpy(subs_temp.subsname, attvalue, sizeof(subs_temp.subsname));
						else if (strcasecmp(attname, "dvid") == 0)
							strncpy(subs_temp.dvid, attvalue, sizeof(subs_temp.dvid));
						else if (strcasecmp(attname, "dvname") == 0) {
						}
						else if (strcasecmp(attname, "vol") == 0)
							strncpy(subs_temp.vol, attvalue, sizeof(subs_temp.vol));
						else if (strcasecmp(attname, "subsid") == 0) {
						}
						else if (strcasecmp(attname, "subname") == 0) {
						}
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					if (strcasecmp(subs_temp.vol, "0.38") != 0)
						m_mNewId2Subs.insert(make_pair(subs_temp.subsid,subs_temp));
				}
			}
				break;
			case _DMSBUSUP_:
			{
				m_mNewId2Bus.clear();
				bus bus_temp;
				TiXmlElement *thrElement_bus;
				TiXmlAttribute *attributeOfthr_bus;

				thrElement_bus = headElement->FirstChildElement();
				for (; thrElement_bus != NULL; thrElement_bus = thrElement_bus->NextSiblingElement()) {
					memset(&bus_temp, 0, sizeof(bus));

					attributeOfthr_bus = thrElement_bus->FirstAttribute();
					for (; attributeOfthr_bus != NULL; attributeOfthr_bus = attributeOfthr_bus->Next()) {
						const char *attname = attributeOfthr_bus->Name();
						const char *attvalue = attributeOfthr_bus->Value();

						memcpy(&bus_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "bus") == 0)
							strncpy(bus_temp.busid, attvalue, sizeof(bus_temp.busid));
						else if (strcasecmp(attname, "name") == 0)
							strncpy(bus_temp.busname, attvalue, sizeof(bus_temp.busname));
						else if (strcasecmp(attname, "dvid") == 0)
							strncpy(bus_temp.dvid, attvalue, sizeof(bus_temp.dvid));
						else if (strcasecmp(attname, "dvname") == 0) {
						}
						else if (strcasecmp(attname, "vol") == 0)
							strncpy(bus_temp.vol, attvalue, sizeof(bus_temp.vol));
						else if (strcasecmp(attname, "subsid") == 0) {
						}
						else if (strcasecmp(attname, "subname") == 0) {
						}
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					if (strcasecmp(bus_temp.vol, "0.38") != 0)
						m_mNewId2Bus.insert(make_pair(bus_temp.busid,bus_temp));
				}
			}
				break;
			case _DMSDEVCORR_:
			{
				m_mNewId2CorrInfo.clear();
				CorrInfo corrinfo_temp;
				TiXmlElement *thrElement_corr;
				TiXmlAttribute *attributeOfthr_corr;

				thrElement_corr = headElement->FirstChildElement();
				for (; thrElement_corr != NULL; thrElement_corr = thrElement_corr->NextSiblingElement()) {
					memset(&corrinfo_temp, 0, sizeof(CorrInfo));

					attributeOfthr_corr = thrElement_corr->FirstAttribute();
					for (; attributeOfthr_corr != NULL; attributeOfthr_corr = attributeOfthr_corr->Next()) {
						const char *attname = attributeOfthr_corr->Name();
						const char *attvalue = attributeOfthr_corr->Value();

						memcpy(&corrinfo_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "devid") == 0)
							strncpy(corrinfo_temp.dev_id, attvalue, sizeof(corrinfo_temp.dev_id));
						else if (strcasecmp(attname, "pre_devid") == 0)
							strncpy(corrinfo_temp.dev_up, attvalue, sizeof(corrinfo_temp.dev_up));
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					m_mNewId2CorrInfo.insert(make_pair(corrinfo_temp.dev_id,corrinfo_temp)); /*ͨ����ID�Ҿ�ID,������IDֻ��һ����Ӧ��ϵ*/
				}
			}
				break;
			case _GOMSEND_:
			{
				gomsend_vector.clear();
				gomsend gomsend_temp;
				TiXmlElement *thrElement_corr;
				TiXmlAttribute *attributeOfthr_corr;

				thrElement_corr = headElement->FirstChildElement();
				for (; thrElement_corr != NULL; thrElement_corr = thrElement_corr->NextSiblingElement()) {
					memset(&gomsend_temp, 0, sizeof(gomsend));

					attributeOfthr_corr = thrElement_corr->FirstAttribute();
					for (; attributeOfthr_corr != NULL; attributeOfthr_corr = attributeOfthr_corr->Next()) {
						const char *attname = attributeOfthr_corr->Name();
						const char *attvalue = attributeOfthr_corr->Value();

						memcpy(&gomsend_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "organ_code") == 0)
							ConvertGDCode2OrganCode(atoi(attvalue), &gomsend_temp.organ_code);
						else if (strcasecmp(attname, "type") == 0)
							gomsend_temp.type = atoi(attvalue);
						else if (strcasecmp(attname, "recnum") == 0)
							gomsend_temp.recnum = atoi(attvalue);
						else if (strcasecmp(attname, "suctype") == 0)
							gomsend_temp.suctype = atoi(attvalue);
						else if (strcasecmp(attname, "send_time") == 0)
							strncpy(gomsend_temp.send_time, attvalue, sizeof(gomsend_temp.send_time) - 1);
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					int find_flag = 0;
					/* // @detail ����organ_code��typeȥ�ظ� */
					for (vector<gomsend>::iterator itr = gomsend_vector.begin(); itr != gomsend_vector.end();++ itr) {

						if ((itr->organ_code == gomsend_temp.organ_code)
						&& (itr->type == gomsend_temp.type)) {
							find_flag = 1;
							break;
						} // 
					} // 

					if (find_flag == 0) {
						gomsend_vector.push_back(gomsend_temp);
					} // 
				}
			}
				break;
			case _SMNDATA_:
			{
				smndata_vector.clear();
				smndata smndata_temp;
				TiXmlElement *thrElement_corr;
				TiXmlAttribute *attributeOfthr_corr;

				thrElement_corr = headElement->FirstChildElement();
				for (; thrElement_corr != NULL; thrElement_corr = thrElement_corr->NextSiblingElement()) {
					memset(&smndata_temp, 0, sizeof(smndata));

					attributeOfthr_corr = thrElement_corr->FirstAttribute();
					for (; attributeOfthr_corr != NULL; attributeOfthr_corr = attributeOfthr_corr->Next()) {
						const char *attname = attributeOfthr_corr->Name();
						const char *attvalue = attributeOfthr_corr->Value();

						memcpy(&smndata_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "organ_code") == 0)
							ConvertGDCode2OrganCode(atoi(attvalue), &smndata_temp.organ_code);
						else if (strcasecmp(attname, "dt") == 0)
							smndata_temp.dtnum = atoi(attvalue);
						else if (strcasecmp(attname, "message") == 0)
							smndata_temp.messagenum = atoi(attvalue);
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					smndata_vector.push_back(smndata_temp);
				}
			}
				break;
			case _SMNDATADET_:
			{
				smndatadet_vector.clear();
				smndatadet smndatadet_temp;
				TiXmlElement *thrElement_corr;
				TiXmlAttribute *attributeOfthr_corr;

				thrElement_corr = headElement->FirstChildElement();
				for (; thrElement_corr != NULL; thrElement_corr = thrElement_corr->NextSiblingElement()) {
					memset(&smndatadet_temp, 0, sizeof(smndatadet));

					attributeOfthr_corr = thrElement_corr->FirstAttribute();
					for (; attributeOfthr_corr != NULL; attributeOfthr_corr = attributeOfthr_corr->Next()) {
						const char *attname = attributeOfthr_corr->Name();
						const char *attvalue = attributeOfthr_corr->Value();

						memcpy(&smndatadet_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "organ_code") == 0)
							ConvertGDCode2OrganCode(atoi(attvalue), &smndatadet_temp.organ_code);
						else if (strcasecmp(attname, "type") == 0)
							smndatadet_temp.type = atoi(attvalue);
						else if (strcasecmp(attname, "reason") == 0)
							strncpy(smndatadet_temp.reason, attvalue, sizeof(smndatadet_temp.reason) - 1);
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					smndatadet_vector.push_back(smndatadet_temp);
				}
			}
				break;
			case _YXBS_:
			{
				yxbsdet_vector.clear();
				yxbsdet yxbsdet_temp;
				TiXmlElement *thrElement_corr;
				TiXmlAttribute *attributeOfthr_corr;

				thrElement_corr = headElement->FirstChildElement();
				for (; thrElement_corr != NULL; thrElement_corr = thrElement_corr->NextSiblingElement()) {
					memset(&yxbsdet_temp, 0, sizeof(yxbsdet));

					attributeOfthr_corr = thrElement_corr->FirstAttribute();
					for (; attributeOfthr_corr != NULL; attributeOfthr_corr = attributeOfthr_corr->Next()) {
						const char *attname = attributeOfthr_corr->Name();
						const char *attvalue = attributeOfthr_corr->Value();

						memcpy(&yxbsdet_temp.header, &header, sizeof(header));

						if (strcasecmp(attname, "devid") == 0)
							strncpy(yxbsdet_temp.devid, attvalue, sizeof(yxbsdet_temp.devid) - 1);
						else if (strcasecmp(attname, "devname") == 0)
							strncpy(yxbsdet_temp.devname, attvalue, sizeof(yxbsdet_temp.devname) - 1);
						else if (strcasecmp(attname, "feederid") == 0)
							strncpy(yxbsdet_temp.feederid, attvalue, sizeof(yxbsdet_temp.feederid) - 1);
						else if (strcasecmp(attname, "feedername") == 0)
							strncpy(yxbsdet_temp.feedername, attvalue, sizeof(yxbsdet_temp.feedername) - 1);
						else if (strcasecmp(attname, "type") == 0)
							yxbsdet_temp.type = atoi(attvalue);
						else if (strcasecmp(attname, "actime") == 0)
							strncpy(yxbsdet_temp.actime, attvalue, sizeof(yxbsdet_temp.actime) - 1);
						else
							cout << "Unknown attribute:" << attvalue << endl;
					}

					yxbsdet_vector.push_back(yxbsdet_temp);
				}
			}
				break;
				default:
				cout << "δ����ı���Ϣ" << endl;
				break;
			}
		}	
	}

	if (deleteDBDate(header.code,g_xmltype) == 0) {

		retCode = insertDB(g_xmltype,threadlog,filename);
		if(g_xmltype == _DMSGPZW_ || g_xmltype == _GPMSGPZW_ || g_xmltype == _DAGC_) {

			if((retCode >> 16) ==0 || (retCode << 16) == 0) {
				insertfile(filename);
				mvxmlfile(filename,0);
			}
			else
				mvxmlfile(filename,1);
		}
		else if(g_xmltype == _RATE_ || g_xmltype == _TZLIST_) {

			if(retCode == 0)
				insertfile(filename);

			mvxmlfile(filename,2);
		}
		else {

			if(retCode == 0) {
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

//���ݿ��������
int resolvexmldata::insertDB(int arg,logstr& threadlog,const char *filename)
{
	unsigned int ret = 0,mask = 0,errmask = 1;
	char error_info[100];
	memset(error_info,0x00,sizeof(error_info));

	enum filetype {
		_ZZZX_,
		_DATT_,
		_DAGC_,
		_YXBW_,
		_GPZW_,
		_DMSGPZW_,
		_GPMSGPZW_,
		_ZDZX_,
		_YKCZ_,
		_YXINFO_,
		_RATE_,
		_BUS_,
		_CB_,
		_DSC_,
		_TRANS_,
		_SUBS_,
		_TDTRANS_,
		_AUTOMAP_,
		_TQRELEASE_,
		_FASTR_,
		_TZLIST_,
		_GPMSTD_,
		_GPMSTDDET_,
		_GPMSZLPDET_,
		_DMSZLPDET_,
		_DMSXSD_,
		_DMSYD_,
		_GPMSYD_,
		_DMSGZZSQ_,
		_DMSYCRT_,
		_DMSEMSRT_,
		_DMSYXRT_,
		_DMSLINE_,
		_DMSXSDET_,
		_DMSZTGJDET_,
		_DMSTRANSUP_, /*������(GIS1.6)���*/
		_DMSCBUP_,	/*������(GIS1.6)����*/
		_DMSDSCUP_,	/*������(GIS1.6)��բ*/
		_DMSUBSUP_,	/*������(GIS1.6)վ��*/
		_DMSBUSUP_,	/*������(GIS1.6)ĸ��*/
		_DMSDEVCORR_,	/*����ǰ(GIS1.5)��������(GIS1.6)�豸ID��Ӧ̨��*/
		_GOMSEND_,
		_SMNDATA_,
		_SMNDATADET_,
		_YXBS_,
		_END_,
		_UnKnownFileType_
	}g_filetype;

	const char *g_strfiletypeName[] = 
	{
		"DMSOP",
		"DATT",
		"DAGC",
		"YXBW",
		"GPZW",
		"DMSGPZW",
		"GPMSGPZW",
		"ZDZX",
		"YKCZ",
		"YXINFO",
		"YCRATE",
		"DMSBUS",
		"DMSCB",
		"DMSCB",
		"DMSLD",
		"DMSST",
		"DMSTD",
		"AUTOGRAPH",
		"TDINFO",
		"FACTION",
		"TZDETAILS",
		"GPMSTD",
		"GPMSTDDET",
		"GPMS_ZLP",
		"DMS_ZLP",
		"DMS_XSD",
		"DMS_ZTGJ",
		"DMS_YD",
		"GPMS_YD",
		"GZZSQ",
		"YCRHT",
		"ztgj",
		"line",
		"xsdcltratedet",
		"ztgjdet",
		"translistup",
		"cblistup",
		"dsclistup",
		"subslistup",
		"buslistup",
		"devlistcorr",
		"gomsend",
		"smnpush",
		"smnpushdet",
		"yxbs",
		"END"
	};
	const char *g_indexname[] = {"�����վ��ƽ��������","FAͶ����","ң����ȷ��","������Ϣһ����","������ȷ��","�ն�������","ң�سɹ���","ң��ʹ����","ң����ȷ��","DMS���òɹ�ר��̨��ƥ����",
	"DMS��ר��ɼ��ɹ���","���ĸ����������","��翪����������","��絶բ��������","�豸״̬һ����","���վ����������","����ѹ��������","DMSͣ����Ϣ����������","�Զ���ͼ��","ͣ����Ϣȷ����","ͣ����Ϣ������ʱ��",
	"ͣ����Ϣ����ʱ��","FA��������","̨�˲�ƥ����ϸ"};

	switch(arg)
	{
	case _ZZZX_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			if(dmsop_vector.size() == 0)
			{
				SetGError(0,0,NULL,_ZZZX_,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_ZZZX_,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_DATT_,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_DATT_,"���ɹ�.",threadlog);
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
		SetGError(1,0,g_strfiletypeName[arg],NULL,"���ɹ�.",threadlog);
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
		SetGError(1,0,g_strfiletypeName[arg+1],NULL,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_YXBW_-1,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_YXBW_-1,"���ɹ�.",threadlog);
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
					SetGError(1,0,g_strfiletypeName[arg],_DMSGPZW_-2,"���ɹ�.",threadlog);
				}

				free(temp_dmsbrand);
				free(attrs);
				attrs = NULL;
				temp_dmsbrand = NULL;
			}
			else
			{
				SetGError(0,0,NULL,_DMSGPZW_-2,"�ļ�����Ϊ��",threadlog);
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
					SetGError(1,0,g_strfiletypeName[arg+1],g_indexname[arg-2],"���ɹ�.",threadlog);
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
		SetGError(1,0,g_strfiletypeName[arg+1],NULL,"���ɹ�.",threadlog);
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
		SetGError(1,0,g_strfiletypeName[arg+2],NULL,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_ZDZX_-2,"�ļ�����Ϊ��",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*7);
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
			attrs[6].data_type = DCI_INT;
			attrs[6].data_size = INT_LEN;

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
				memcpy(temp_zdzx+offsetpos,&(it_zdzx->type),INT_LEN);
				offsetpos += INT_LEN;


				/*������⣬�ɼ�������һ�����ݳ���*/
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
			//����ִ��
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.ZDZX(ORGAN_CODE,COUNT_TIME,RNUM,NAME,ONLINE,SEND_TIME,TYPE) VALUES(:1,to_date(:2,'YYYY-MM-DD HH24:MI:SS'),:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7)");
			if(d5000.d5000_WriteData("ZDZX",query,temp_zdzx,number,7,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_ZDZX_-2,error_info,threadlog);
				Log("Insert ZDZX fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_ZDZX_-2,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_YKCZ_-2,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_YKCZ_-2,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_YXINFO_-2,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_YXINFO_-2,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_RATE_-2,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_RATE_-2,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_BUS_-1,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_BUS_-1,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_CB_-1,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_CB_-1,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_DSC_-1,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_DSC_-1,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_TRANS_,"�ļ�����Ϊ��",threadlog);
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
			attrs[2].data_size = STR_LEN_NAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_NAME;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_CHAR;

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
				memcpy(temp_trans + offsetpos, it_trans->tqh, STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSLD(ORGAN_CODE,DV,LD,NAME,GTIME,TQH) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),:6)");
			if(d5000.d5000_WriteData("DMSLDL",query,temp_trans,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_TRANS_,error_info,threadlog);
				Log("Insert ld fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_TRANS_,"���ɹ�.",threadlog);
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

			printf("%s:[subs]:%d\n",filename,subs_vector.size());

			if(subs_vector.size() == 0)
			{
				SetGError(0,0,NULL,_SUBS_,"�ļ�����Ϊ��",threadlog);
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

			number = subs_vector.size();
			char *temp_subs = (char *)malloc((size_t)(number*500));
			if(temp_subs == NULL)
				break;
			memset(temp_subs,0x00,number*500);
			vector<subs>::iterator it_subs;
			for(it_subs = subs_vector.begin();it_subs != subs_vector.end();++it_subs)
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
				SetGError(1,0,g_strfiletypeName[arg],_SUBS_,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_TDTRANS_,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_TDTRANS_,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_AUTOMAP_,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_AUTOMAP_,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_TQRELEASE_+1,"�ļ�����Ϊ��",threadlog);
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
				sprintf(query,"INSERT INTO EVALUSYSTEM.DETAIL.TDINFO(ORGAN_CODE,TDNUM,DMSTD,DDAFF,DMSFF,TDINTR,COUNT_TIME,SEND_TIME,SENDTYPE,JUDGED_TIME,JUDGED_TYPE,YPID) VALUES(%d,%d,to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),'%s',to_date('%s','YYYY-MM-DD HH24:MI:SS'),to_date('%s','YYYY-MM-DD HH24:MI:SS'),%d,to_date('%s','YYYY-MM-DD HH24:MI:SS'),%d,'%s')",it_tqrelease->header.code,it_tqrelease->tdnum,it_tqrelease->dmstd,it_tqrelease->ddaff,it_tqrelease->dmsaff,it_tqrelease->tdintnr,it_tqrelease->header.count,it_tqrelease->header.send,it_tqrelease->sendtype,it_tqrelease->judged_time,it_tqrelease->yptype,it_tqrelease->ypid);

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
				SetGError(1,0,g_strfiletypeName[arg],_TQRELEASE_+1,"���ɹ�.",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_TQRELEASE_+1,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_FASTR_+3,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_FASTR_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_TZLIST_+3,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_TZLIST_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_GPMSTD_+3,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_GPMSTD_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_GPMSTDDET_+3,"�ļ�����Ϊ��",threadlog);
				
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
					"(:1,:2,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:3,'���ļ�����Ϊ��','yc');");
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
				SetGError(1,0,g_strfiletypeName[arg],_GPMSTDDET_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_GPMSZLPDET_+3,"�ļ�����Ϊ��",threadlog);
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
			
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*15);
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
			attrs[12].data_type = DCI_STR;
			attrs[12].data_size = STR_LEN_SS;
			attrs[13].data_type = DCI_STR;
			attrs[13].data_size = STR_LEN_SS;
			attrs[14].data_type = DCI_STR;
			attrs[14].data_size = STR_LEN_SS;

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
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->notify_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->oprtfinish_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmszlpdet+offsetpos,it_gpmszlpdet->check_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_ZLP(ORGAN_CODE,ORGAN_NAME,DMSID,GPMSID,TYPE,CONTENT,START_TIME,ORDER_TIME,CONFIRM_TIME,"
				"COUNT_TIME,SEND_TIME,CTL_NAME,NOTIFY_TIME,OPRTFINISH_TIME,CHECK_TIME) VALUES(:1,:2,:3,:4,:5,:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'),"
				"to_date(:8,'YYYY-MM-DD HH24:MI:SS'),to_date(:9,'YYYY-MM-DD HH24:MI:SS'),to_date(:10,'YYYY-MM-DD HH24:MI:SS'),"
				"to_date(:11,'YYYY-MM-DD HH24:MI:SS'),:12,to_date(:13,'YYYY-MM-DD HH24:MI:SS'),to_date(:14,'YYYY-MM-DD HH24:MI:SS'),to_date(:15,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("GPMS_ZLP",query,temp_gpmszlpdet,number,15,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSZLPDET_+3,error_info,threadlog);
				Log("Insert GPMS_ZLP_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSZLPDET_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_DMSZLPDET_+3,"�ļ�����Ϊ��",threadlog);
				break;
			}
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
			attrs[3].data_size = STR_LEN_ZLCOT;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
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
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->order_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->confirm_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->check_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->notify_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmszlpdet+offsetpos,it_dmszlpdet->oprtfinish_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_ZLP(ORGAN_CODE,DMSID,TYPE,CONTENT,START_TIME,PLAN_OPERATE_TIME,COUNT_TIME,SEND_TIME,CONFIRM_TIME,CHECK_TIME,NOTIFY_TIME,OPRTFINISH_TIME) "
				"VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'),to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'),to_date(:9,'YYYY-MM-DD HH24:MI:SS'),"
				"to_date(:10,'YYYY-MM-DD HH24:MI:SS'),to_date(:11,'YYYY-MM-DD HH24:MI:SS'),to_date(:12,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_ZLP",query,temp_dmszlpdet,number,12,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSZLPDET_+3,error_info,threadlog);
				Log("Insert DMS_ZLP_DET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSZLPDET_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_DMSXSD_+3,"�ļ�����Ϊ��",threadlog);
				break;
			}

			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*6);
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
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;

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
				memcpy(temp_dmsxsd+offsetpos,&it_dmsxsd->xsd_num,INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsxsd+offsetpos,&it_dmsxsd->sendtime,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_XSD(ORGAN_CODE,VALUE,COUNT_TIME,SEND_TIME,NUM,EMSENDTIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'),to_date(:4,'YYYY-MM-DD HH24:MI:SS'),:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("DMS_XSD",query,temp_dmsxsd,number,6,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSXSD_+3,error_info,threadlog);
				Log("Insert DMS_XSD fail.(%s)\n",error_info);
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSXSD_+3,"���ɹ�.",threadlog);
			}

			free(temp_dmsxsd);
			free(attrs);
			attrs = NULL;
			temp_dmsxsd = NULL;
		}
		break;
	/*case _ZTGJ_:
		{
			int offsetpos = 0,number = 0;
			char query[1024];
			char sql[256];
			memset(query,0x00,sizeof(query));
			memset(sql,0x00,sizeof(sql));
			printf("%s:[ztgj]:%d\n",filename,ztgj_vector.size());
			if(ztgj_vector.size() == 0)
			{
				SetGError(0,0,NULL,_ZTGJ_+3,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_ZTGJ_+3,"���ɹ�.",threadlog);
			}

			free(temp_ztgj);
			free(attrs);
			attrs = NULL;
			temp_ztgj = NULL;
		}
		break;*/
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
				SetGError(0,0,NULL,_DMSYD_+3,"�ļ�����Ϊ��",threadlog);
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
				SetGError(1,0,g_strfiletypeName[arg],_DMSYD_+3,"���ɹ�.",threadlog);
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
				SetGError(0,0,NULL,_GPMSZLPDET_+3,"�ļ�����Ϊ��",threadlog);
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
			
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_ID;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

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
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->send_time,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gpmsyd+offsetpos,it_gpmsyd->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GPMS_YD(ORGAN_CODE,ORGAN_NAME,YDD_ID,SEND_TIME,COUNT_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'));");
		
			if(d5000.d5000_WriteData("GPMS_YD",query,temp_gpmsyd,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_GPMSYD_+3,error_info,threadlog);
				Log("Insert GPMS_YD fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_GPMSYD_+3,"���ɹ�.",threadlog);
			}

			free(temp_gpmsyd);
			free(attrs);
			attrs = NULL;
			temp_gpmsyd = NULL;
		}
		break;
	case _DMSGZZSQ_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[gzzsq]:" <<  gzzsq_vector.size() << endl;
			if(gzzsq_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSGZZSQ_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*8);
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
			attrs[4].data_type = DCI_FLT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_STR;
			attrs[5].data_size = STR_LEN_SS;
			attrs[6].data_type = DCI_FLT;
			attrs[6].data_size = INT_LEN;
			attrs[7].data_type = DCI_FLT;
			attrs[7].data_size = INT_LEN;

			number = gzzsq_vector.size();
			char *temp_gzzsq = (char *)malloc((size_t)(number*100));
			if(temp_gzzsq == NULL)
				break;
			memset(temp_gzzsq,0x00,number*100);
			vector<gzzsq>::iterator it_rate;
			for(it_rate = gzzsq_vector.begin();it_rate != gzzsq_vector.end();++it_rate)
			{
				memcpy(temp_gzzsq+offsetpos,&(it_rate->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->cover_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->shake_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->misoperation_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->missreport_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,it_rate->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->tworatiocover_rate),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_gzzsq+offsetpos,&(*(int*)&it_rate->threeratiocover_rate),INT_LEN);
				offsetpos += INT_LEN;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.GZZSQ(ORGAN_CODE,COVER_RATE,SHAKE_RATE,MISOPERATION_RATE,MISSREPORT_RATE,COUNT_TIME,TWORATIOCOVER_RATE,THREERATIOCOVER_RATE) "
				"VALUES(:1,round(:2,4),round(:3,4),round(:4,4),round(:5,4),to_date(:6,'YYYY-MM-DD HH24:MI:SS'),round(:7,4),round(:8,4))");
			if(d5000.d5000_WriteData("GZZSQ",query,temp_gzzsq,number,8,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSGZZSQ_-2,error_info,threadlog);
				Log("Insert GZZSQ fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSGZZSQ_-2,"���ɹ�.",threadlog);
			}

			free(temp_gzzsq);
			free(attrs);
			attrs = NULL;
			temp_gzzsq = NULL;
		}
		break;
	case _DMSYCRT_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsycrt]:" <<  ycrt_vector.size() << endl;
			if(ycrt_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSYCRT_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*3);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;

			number = ycrt_vector.size();
			char *temp_ycrt = (char *)malloc((size_t)(number*100));
			if(temp_ycrt == NULL)
				break;
			memset(temp_ycrt,0x00,number*100);
			vector<ycrt>::iterator it_rate;
			for(it_rate = ycrt_vector.begin();it_rate != ycrt_vector.end();++it_rate)
			{
				memcpy(temp_ycrt+offsetpos,&(it_rate->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ycrt+offsetpos,&(*(int*)&it_rate->yc_right),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_ycrt+offsetpos,it_rate->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YCRT(ORGAN_CODE,YC_RIGHT,COUNT_TIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("GZZSQ",query,temp_ycrt,number,3,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSYCRT_-2,error_info,threadlog);
				Log("Insert YCRT fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSYCRT_-2,"���ɹ�.",threadlog);
			}

			free(temp_ycrt);
			free(attrs);
			attrs = NULL;
			temp_ycrt = NULL;
		}
		break;
	case _DMSEMSRT_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsemsrt]:" <<  emsrt_vector.size() << endl;
			if(emsrt_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSEMSRT_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*3);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;

			number = emsrt_vector.size();
			char *temp_emsrt = (char *)malloc((size_t)(number*100));
			if(temp_emsrt == NULL)
				break;
			memset(temp_emsrt,0x00,number*100);
			vector<emsrt>::iterator it_rate;
			for(it_rate = emsrt_vector.begin();it_rate != emsrt_vector.end();++it_rate)
			{
				memcpy(temp_emsrt+offsetpos,&(it_rate->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt+offsetpos,&(*(int*)&it_rate->emszfrt),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt+offsetpos,it_rate->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.EMSRT(ORGAN_CODE,EMSZFRT,COUNT_TIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("EMSRT",query,temp_emsrt,number,3,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSEMSRT_-2,error_info,threadlog);
				Log("Insert YCRT fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSEMSRT_-2,"���ɹ�.",threadlog);
			}

			free(temp_emsrt);
			free(attrs);
			attrs = NULL;
			temp_emsrt = NULL;
		}
		break;
	case _DMSYXRT_:
		{
		/* // @detail ȥ��ԭ��ң��״̬��ʾ�ʣ�����Ϊһ������״̬���ƴ�����ϸ */
#if 0
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsyxrt]:" <<  yxrt_vector.size() << endl;
			if(yxrt_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSYXRT_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*3);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_FLT;
			attrs[1].data_size = INT_LEN;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_SS;

			number = yxrt_vector.size();
			char *temp_emsrt = (char *)malloc((size_t)(number*100));
			if(temp_emsrt == NULL)
				break;
			memset(temp_emsrt,0x00,number*100);
			vector<yxrt>::iterator it_rate;
			for(it_rate = yxrt_vector.begin();it_rate != yxrt_vector.end();++it_rate)
			{
				memcpy(temp_emsrt+offsetpos,&(it_rate->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt+offsetpos,&(*(int*)&it_rate->yx_right),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt+offsetpos,it_rate->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.YXRT(ORGAN_CODE,YX_RT,COUNT_TIME) VALUES(:1,round(:2,4),to_date(:3,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("YXRT",query,temp_emsrt,number,3,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSYXRT_-2,error_info,threadlog);
				Log("Insert YXRT fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSYXRT_-2,"���ɹ�.",threadlog);
			}

			free(temp_emsrt);
			free(attrs);
			attrs = NULL;
			temp_emsrt = NULL;
#endif
			int offsetpos = 0, number = 0;
			char query[200];
			memset(query, 0x00, sizeof(query));

			cout << filename << ":[ztgj]:" << ztgjfromone_vector.size() << endl;
			if (ztgjfromone_vector.size() == 0) {
				SetGError(0, 0, NULL, _DMSYXRT_ - 2, "�ļ�����Ϊ��", threadlog);  //������
				break;
			}

			struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 9);
			if (attrs == NULL)
				break;

			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_ZDNAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_CHAR;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_ZDNAME;
			attrs[5].data_type = DCI_INT;
			attrs[5].data_size = INT_LEN;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;
			attrs[7].data_type = DCI_STR;
			attrs[7].data_size = STR_LEN_SS;
			attrs[8].data_type = DCI_STR;
			attrs[8].data_size = STR_LEN_SS;

			number = ztgjfromone_vector.size();
			char *temp_emsrt = (char *)malloc((size_t)(number * 1024));
			if (temp_emsrt == NULL)
				break;

			memset(temp_emsrt, 0x00, number * 1024);

			vector<ztgjfromone>::iterator it_rate;
			for (it_rate = ztgjfromone_vector.begin(); it_rate != ztgjfromone_vector.end(); ++it_rate) {
				memcpy(temp_emsrt + offsetpos, &(it_rate->header.code), INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt + offsetpos, &(it_rate->devid), STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_emsrt + offsetpos, &(it_rate->devname), STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_emsrt + offsetpos, &(it_rate->feederid), STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_emsrt + offsetpos, &(it_rate->feedername), STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_emsrt + offsetpos, &(it_rate->type), INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_emsrt + offsetpos, it_rate->result_time, STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_emsrt + offsetpos, it_rate->header.count, STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_emsrt + offsetpos, it_rate->header.send, STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}

			strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSZTGJCBDET(ORGAN_CODE,DEVID,DEVNAME,FEEDERID,FEEDERNAME,TYPE,RESULTIME,COUNT_TIME,SEND_TIME) "
				"VALUES(:1,:2,:3,:4,:5,:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'),to_date(:9,'YYYY-MM-DD HH24:MI:SS'))");
			
			if (d5000.d5000_WriteData("DMSZTGJCBDET", query, temp_emsrt, number, 9, attrs, error_info) != 0) {
				SetGError(2, 0, g_strfiletypeName[arg], _DMSYXRT_ - 2, error_info, threadlog);
				Log("Insert ZTGJ fail.\n");
				ret = ret | errmask;
			}
			else {
				SetGError(1, 0, g_strfiletypeName[arg], _DMSYXRT_ - 2, "���ɹ�.", threadlog);
			}

			free(temp_emsrt);
			free(attrs);
			attrs = NULL;
			temp_emsrt = NULL;
		}
		break;
	case _DMSLINE_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsline]:" <<  lines_vector.size() << endl;
			if(lines_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSLINE_-2,"�ļ�����Ϊ��",threadlog);  //������
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
			attrs[3].data_type = DCI_INT;
			attrs[3].data_size = INT_LEN;
			attrs[4].data_type = DCI_INT;
			attrs[4].data_size = INT_LEN;
			attrs[5].data_type = DCI_INT;
			attrs[5].data_size = INT_LEN;
			attrs[6].data_type = DCI_STR;
			attrs[6].data_size = STR_LEN_SS;

			number = lines_vector.size();
			char *temp_line = (char *)malloc((size_t)(number*300));
			if(temp_line == NULL)
				break;
			memset(temp_line,0x00,number*300);
			vector<lines>::iterator it_lines;
			for(it_lines = lines_vector.begin();it_lines != lines_vector.end();++it_lines)
			{
				memcpy(temp_line+offsetpos,&(it_lines->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_line+offsetpos,it_lines->lineid,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_line+offsetpos,it_lines->linename,STR_LEN_NAME);
				offsetpos += STR_LEN_NAME;
				memcpy(temp_line+offsetpos,&(it_lines->indicator),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_line+offsetpos,&(it_lines->tworatio),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_line+offsetpos,&(it_lines->threeratio),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_line+offsetpos,it_lines->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMSLINE(ORGAN_CODE,LINEID,LINENAME,INDICATOR,TWORATIO,THREERATIO,COUNT_TIME) VALUES(:1,:2,:3,:4,:5,:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("LINE",query,temp_line,number,7,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSLINE_-2,error_info,threadlog);
				Log("Insert LINE fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSLINE_-2,"���ɹ�.",threadlog);
			}

			free(temp_line);
			free(attrs);
			attrs = NULL;
			temp_line = NULL;
		}
		break;
	case _DMSXSDET_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsxsdet]:" <<  dmsxsdet_vector.size() << endl;
			if(dmsxsdet_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSXSDET_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*4);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_ZDNAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;

			number = dmsxsdet_vector.size();
			char *temp_dmsxsdet = (char *)malloc((size_t)(number*500));
			if(temp_dmsxsdet == NULL)
				break;
			memset(temp_dmsxsdet,0x00,number*500);
			vector<dmsxsdet>::iterator it_dmsxsdet;
			for(it_dmsxsdet = dmsxsdet_vector.begin();it_dmsxsdet != dmsxsdet_vector.end();++it_dmsxsdet)
			{
				memcpy(temp_dmsxsdet+offsetpos,&(it_dmsxsdet->header.code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsxsdet+offsetpos,it_dmsxsdet->id,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_dmsxsdet+offsetpos,it_dmsxsdet->name,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_dmsxsdet+offsetpos,it_dmsxsdet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_XSD_DET(ORGAN_CODE,ID,NAME,COUNT_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSXSDET",query,temp_dmsxsdet,number,4,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSEMSRT_-2,error_info,threadlog);
				Log("Insert DMSXSDET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSEMSRT_-2,"���ɹ�.",threadlog);
			}

			free(temp_dmsxsdet);
			free(attrs);
			attrs = NULL;
			temp_dmsxsdet = NULL;
		}
		break;
	case _DMSZTGJDET_:
		{
			int offsetpos = 0,number = 0;
			char query[200];
			memset(query,0x00,sizeof(query));

			cout << filename << ":[dmsztgjdet]:" <<  dmsztgjdet_vector.size() << endl;
			if(dmsztgjdet_vector.size() == 0)
			{
				SetGError(0,0,NULL,_DMSZTGJDET_-2,"�ļ�����Ϊ��",threadlog);  //������
				break;
			}
			struct ColAttr* attrs = (ColAttr* )malloc(sizeof(ColAttr_t)*5);
			if(attrs == NULL)
				break;
			attrs[0].data_type = DCI_INT;
			attrs[0].data_size = INT_LEN;
			attrs[1].data_type = DCI_STR;
			attrs[1].data_size = STR_LEN_CHAR;
			attrs[2].data_type = DCI_STR;
			attrs[2].data_size = STR_LEN_ZDNAME;
			attrs[3].data_type = DCI_STR;
			attrs[3].data_size = STR_LEN_SS;
			attrs[4].data_type = DCI_STR;
			attrs[4].data_size = STR_LEN_SS;

			number = dmsztgjdet_vector.size();
			char *temp_dmsztgjdet = (char *)malloc((size_t)(number*500));
			if(temp_dmsztgjdet == NULL)
				break;
			memset(temp_dmsztgjdet,0x00,number*500);
			vector<ztgjdet>::iterator it_dmsztgjdet;
			for(it_dmsztgjdet = dmsztgjdet_vector.begin();it_dmsztgjdet != dmsztgjdet_vector.end();++it_dmsztgjdet)
			{
				memcpy(temp_dmsztgjdet+offsetpos,&(it_dmsztgjdet->organ_code),INT_LEN);
				offsetpos += INT_LEN;
				memcpy(temp_dmsztgjdet+offsetpos,it_dmsztgjdet->id,STR_LEN_CHAR);
				offsetpos += STR_LEN_CHAR;
				memcpy(temp_dmsztgjdet+offsetpos,it_dmsztgjdet->name,STR_LEN_ZDNAME);
				offsetpos += STR_LEN_ZDNAME;
				memcpy(temp_dmsztgjdet+offsetpos,it_dmsztgjdet->header.count,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
				memcpy(temp_dmsztgjdet+offsetpos,it_dmsztgjdet->header.send,STR_LEN_SS);
				offsetpos += STR_LEN_SS;
			}
			strcpy(query,"INSERT INTO EVALUSYSTEM.DETAIL.DMS_ZTGJ_DET(ORGAN_CODE,ID,NAME,COUNT_TIME,SEND_TIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");
			if(d5000.d5000_WriteData("DMSZTGJDET",query,temp_dmsztgjdet,number,5,attrs,error_info) != 0)
			{
				SetGError(2,0,g_strfiletypeName[arg],_DMSZTGJDET_-2,error_info,threadlog);
				Log("Insert DMSZTGJDET fail.\n");
				ret = ret | errmask;
			}
			else
			{
				SetGError(1,0,g_strfiletypeName[arg],_DMSZTGJDET_-2,"���ɹ�.",threadlog);
			}

			free(temp_dmsztgjdet);
			free(attrs);
			attrs = NULL;
			temp_dmsztgjdet = NULL;
		}
		break;
	case _DMSBUSUP_:
	{
		int offsetpos = 0, number = 0;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[bus]:%d\n", filename, m_mNewId2Bus.size());
		if (m_mNewId2Bus.size() == 0) {
			SetGError(0, 0, NULL, _BUS_ - 1, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 5);
		if (attrs == NULL)
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

		number = m_mNewId2Bus.size();
		char *temp_bus = (char *)malloc((size_t)(number * 500));
		if (temp_bus == NULL)
			break;

		memset(temp_bus, 0x00, number * 500);

		map<string,bus>::iterator it_bus;
		for (it_bus = m_mNewId2Bus.begin(); it_bus != m_mNewId2Bus.end(); ++it_bus) {
			memcpy(temp_bus + offsetpos, &(it_bus->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_bus + offsetpos, &(it_bus->second.dvid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_bus + offsetpos, &(it_bus->second.busid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_bus + offsetpos, &(it_bus->second.busname), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_bus + offsetpos, it_bus->second.header.count, STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSBUSUP(ORGAN_CODE,DV,DEVID,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("DMSBUS_IMAGE", query, temp_bus, number, 5, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _BUS_ - 1, error_info, threadlog);
			Log("Insert bus fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _BUS_ - 1, "���ɹ�.", threadlog);
		}

		free(temp_bus);
		free(attrs);
		attrs = NULL;
		temp_bus = NULL;
	}
	break;
	case _DMSCBUP_:
	{
		int offsetpos = 0, number = 0;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[cb]:%d\n", filename, m_mNewId2Cb.size());

		if (m_mNewId2Cb.size() == 0) {
			SetGError(0, 0, NULL, _CB_ - 1, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 9);
		if (attrs == NULL)
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

		number = m_mNewId2Cb.size();
		char *temp_cb = (char *)malloc((size_t)(number * 500));
		if (temp_cb == NULL)
			break;

		memset(temp_cb, 0x00, number * 500);
		map<string,cb>::iterator it_cb;
		for (it_cb = m_mNewId2Cb.begin(); it_cb != m_mNewId2Cb.end(); ++it_cb) {
			memcpy(temp_cb + offsetpos, &(it_cb->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_cb + offsetpos, &(it_cb->second.dvid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_cb + offsetpos, &(it_cb->second.cbid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_cb + offsetpos, &(it_cb->second.cbname), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_cb + offsetpos, &(it_cb->second.type), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_cb + offsetpos, it_cb->second.header.count, STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_cb + offsetpos, &(it_cb->second.status), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_cb + offsetpos, &(it_cb->second.flag), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_cb + offsetpos, &(it_cb->second.remotetype), INT_LEN);
			offsetpos += INT_LEN;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSCBUP(ORGAN_CODE,DV,CB,NAME,TYPE,GTIME,STATUS,FLAG,REMOTETYPE) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7,:8,:9)");

		if (d5000.d5000_WriteData("DMSCBL_IMAGE", query, temp_cb, number, 9, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _CB_ - 1, error_info, threadlog);
			Log("Insert cb fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _CB_ - 1, "���ɹ�.", threadlog);
		}

		free(temp_cb);
		free(attrs);
		attrs = NULL;
		temp_cb = NULL;
	}
	break;
	case _DMSDSCUP_:
	{
		int offsetpos = 0, number = 0;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[dsc]:%d\n", filename, m_mNewId2Dsc.size());

		if (m_mNewId2Dsc.size() == 0) {
			SetGError(0, 0, NULL, _DSC_ - 1, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 8);
		if (attrs == NULL)
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

		number = m_mNewId2Dsc.size();
		char *temp_dsc = (char *)malloc((size_t)(number * 500));
		if (temp_dsc == NULL)
			break;

		memset(temp_dsc, 0x00, number * 500);
		map<string,dsc>::iterator it_dsc;
		for (it_dsc = m_mNewId2Dsc.begin(); it_dsc != m_mNewId2Dsc.end(); ++it_dsc) {
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.dvid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.dscid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.dscname), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.type), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_dsc + offsetpos, it_dsc->second.header.count, STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.status), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_dsc + offsetpos, &(it_dsc->second.flag), INT_LEN);
			offsetpos += INT_LEN;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSCBUP(ORGAN_CODE,DV,CB,NAME,TYPE,GTIME,STATUS,FLAG) VALUES(:1,:2,:3,:4,:5,to_date(:6,'YYYY-MM-DD HH24:MI:SS'),:7,:8)");

		if (d5000.d5000_WriteData("DMSCBL", query, temp_dsc, number, 8, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _DSC_ - 1, error_info, threadlog);
			Log("Insert cb fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _DSC_ - 1, "���ɹ�.", threadlog);
		}

		free(temp_dsc);
		free(attrs);
		attrs = NULL;
		temp_dsc = NULL;
	}
	break;
	case _DMSTRANSUP_:
	{
		int offsetpos = 0, number = 0;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[trans]:%d\n", filename, m_mNewId2Trans.size());

		if (m_mNewId2Trans.size() == 0) {
			SetGError(0, 0, NULL, _TRANS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 5);
		if (attrs == NULL)
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

		number = m_mNewId2Trans.size();
		char *temp_trans = (char *)malloc((size_t)(number * 500));
		if (temp_trans == NULL)
			break;

		memset(temp_trans, 0x00, number * 500);

		map<string,trans>::iterator it_trans;
		for (it_trans = m_mNewId2Trans.begin(); it_trans != m_mNewId2Trans.end(); ++it_trans) {
			memcpy(temp_trans + offsetpos, &(it_trans->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_trans + offsetpos, &(it_trans->second.dvid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_trans + offsetpos, &(it_trans->second.transid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_trans + offsetpos, &(it_trans->second.transname), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_trans + offsetpos, it_trans->second.header.count, STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSLDUP(ORGAN_CODE,DV,LD,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("DMSLDL", query, temp_trans, number, 5, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _TRANS_, error_info, threadlog);
			Log("Insert ld fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _TRANS_, "���ɹ�.", threadlog);
		}

		free(temp_trans);
		free(attrs);
		attrs = NULL;
		temp_trans = NULL;
	}
	break;
	case _DMSUBSUP_:
	{
		int offsetpos = 0, number = 0;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[subs]:%d\n", filename, m_mNewId2Subs.size());

		if (m_mNewId2Subs.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * 5);
		if (attrs == NULL)
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

		number = m_mNewId2Subs.size();
		char *temp_subs = (char *)malloc((size_t)(number * 500));
		if (temp_subs == NULL)
			break;

		memset(temp_subs, 0x00, number * 500);

		map<string,subs>::iterator it_subs;
		for (it_subs = m_mNewId2Subs.begin(); it_subs != m_mNewId2Subs.end(); ++it_subs) {
			memcpy(temp_subs + offsetpos, &(it_subs->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_subs + offsetpos, &(it_subs->second.dvid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_subs + offsetpos, &(it_subs->second.subsid), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_subs + offsetpos, &(it_subs->second.subsname), STR_LEN_NAME);
			offsetpos += STR_LEN_NAME;
			memcpy(temp_subs + offsetpos, it_subs->second.header.count, STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSSTUP(ORGAN_CODE,DV,DEVID,NAME,GTIME) VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("DMSST", query, temp_subs, number, 5, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _SUBS_, error_info, threadlog);
			Log("Insert st fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _SUBS_, "���ɹ�.", threadlog);
		}

		free(temp_subs);
		free(attrs);
		attrs = NULL;
		temp_subs = NULL;
	}
	break;
	case _DMSDEVCORR_:
	{
		int offsetpos = 0, number = 0,str_len_ = 0,cyc_ = 4;
		char query[200];
		memset(query, 0x00, sizeof(query));

		printf("%s:[corr]:%d\n", filename, m_mNewId2CorrInfo.size());

		if (m_mNewId2CorrInfo.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * cyc_);
		if (attrs == NULL)
			break;

		attrs[0].data_type = DCI_INT;
		attrs[0].data_size = INT_LEN;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_CHAR;
		attrs[2].data_type = DCI_STR;
		attrs[2].data_size = STR_LEN_CHAR;
		attrs[3].data_type = DCI_STR;
		attrs[3].data_size = STR_LEN_SS;

		for (int i = 0; i < cyc_;++i) {
			str_len_ += attrs[i].data_size;
		} // 

		number = m_mNewId2CorrInfo.size();
		char *temp_corr = (char *)malloc((size_t)(number * (str_len_+1)));
		if (temp_corr == NULL)
			break;

		memset(temp_corr, 0x00, number * (str_len_ + 1));

		map<string,CorrInfo>::iterator it_corr;
		for (it_corr = m_mNewId2CorrInfo.begin(); it_corr != m_mNewId2CorrInfo.end(); ++it_corr) {
			memcpy(temp_corr + offsetpos, &(it_corr->second.header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_corr + offsetpos, &(it_corr->second.dev_id), STR_LEN_CHAR);
			offsetpos += STR_LEN_CHAR;
			memcpy(temp_corr + offsetpos, &(it_corr->second.dev_up), STR_LEN_CHAR);
			offsetpos += STR_LEN_CHAR;
			memcpy(temp_corr + offsetpos, &(it_corr->second.header.count), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.DMSCORR(ORGAN_CODE,DEVID,DEVUP,GTIME) VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("DMSDEVCORR", query, temp_corr, number, cyc_, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _DMSDEVCORR_, error_info, threadlog);
			Log("Insert corr fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _DMSDEVCORR_, "���ɹ�.", threadlog);
		}

		free(temp_corr);
		free(attrs);
		attrs = NULL;
		temp_corr = NULL;
	}
		break;
	case _GOMSEND_:
	{
		int offsetpos = 0, number = 0, str_len_ = 0, cyc_ = 6;
		char query[1024];
		memset(query, 0x00, sizeof(query));

		printf("%s:[gomsend]:%d\n", filename, gomsend_vector.size());

		if (gomsend_vector.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * cyc_);
		if (attrs == NULL)
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

		for (int i = 0; i < cyc_; ++i) {
			str_len_ += attrs[i].data_size;
		} // 

		number = gomsend_vector.size();
		char *temp_goms = (char *)malloc((size_t)(number * (str_len_ + 1)));
		if (temp_goms == NULL)
			break;

		memset(temp_goms, 0x00, number * (str_len_ + 1));

		vector<gomsend>::iterator it_gomsend;
		for (it_gomsend = gomsend_vector.begin(); it_gomsend != gomsend_vector.end(); ++it_gomsend) {
			memcpy(temp_goms + offsetpos, &(it_gomsend->organ_code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_goms + offsetpos, &(it_gomsend->type), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_goms + offsetpos, &(it_gomsend->recnum), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_goms + offsetpos, &(it_gomsend->suctype), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_goms + offsetpos, &(it_gomsend->header.count), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_goms + offsetpos, &(it_gomsend->header.send), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.GOMSPUSH(ORGAN_CODE,TYPE,RECORD_NUM,SUCCESS,COUNT_TIME,SEND_TIME) "
			"VALUES(:1,:2,:3,:4,to_date(:5,'YYYY-MM-DD HH24:MI:SS'),to_date(:6,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("GOMSEND", query, temp_goms, number, cyc_, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _GOMSEND_, error_info, threadlog);
			Log("Insert corr fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _GOMSEND_, "���ɹ�.", threadlog);
		}

		free(temp_goms);
		free(attrs);
		attrs = NULL;
		temp_goms = NULL;
	}
		break;
	case _SMNDATA_:
	{
		int offsetpos = 0, number = 0, str_len_ = 0, cyc_ = 5;
		char query[1024];
		memset(query, 0x00, sizeof(query));

		printf("%s:[smndata]:%d\n", filename, smndata_vector.size());

		if (smndata_vector.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * cyc_);
		if (attrs == NULL)
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

		for (int i = 0; i < cyc_; ++i) {
			str_len_ += attrs[i].data_size;
		} // 

		number = smndata_vector.size();
		char *temp_smndata = (char *)malloc((size_t)(number * (str_len_ + 1)));
		if (temp_smndata == NULL)
			break;

		memset(temp_smndata, 0x00, number * (str_len_ + 1));

		vector<smndata>::iterator it_smndata;
		for (it_smndata = smndata_vector.begin(); it_smndata != smndata_vector.end(); ++it_smndata) {
			memcpy(temp_smndata + offsetpos, &(it_smndata->organ_code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndata + offsetpos, &(it_smndata->dtnum), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndata + offsetpos, &(it_smndata->messagenum), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndata + offsetpos, &(it_smndata->header.count), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_smndata + offsetpos, &(it_smndata->header.send), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.SMNDATA(ORGAN_CODE,DTNUM,MESSAGENUM,COUNT_TIME,SEND_TIME) "
			"VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("SMNDATA", query, temp_smndata, number, cyc_, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _SMNDATA_, error_info, threadlog);
			Log("Insert corr fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _SMNDATA_, "���ɹ�.", threadlog);
		}

		free(temp_smndata);
		free(attrs);
		attrs = NULL;
		temp_smndata = NULL;
	}
		break;
	case _SMNDATADET_:
	{
		int offsetpos = 0, number = 0, str_len_ = 0, cyc_ = 5;
		char query[1024];
		memset(query, 0x00, sizeof(query));

		printf("%s:[smndatadet]:%d\n", filename, smndatadet_vector.size());

		if (smndatadet_vector.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * cyc_);
		if (attrs == NULL)
			break;

		attrs[0].data_type = DCI_INT;
		attrs[0].data_size = INT_LEN;
		attrs[1].data_type = DCI_INT;
		attrs[1].data_size = INT_LEN;
		attrs[2].data_type = DCI_STR;
		attrs[2].data_size = STR_LEN_ID;
		attrs[4].data_type = DCI_STR;
		attrs[4].data_size = STR_LEN_SS;
		attrs[5].data_type = DCI_STR;
		attrs[5].data_size = STR_LEN_SS;

		for (int i = 0; i < cyc_; ++i) {
			str_len_ += attrs[i].data_size;
		} // 

		number = smndatadet_vector.size();
		char *temp_smndatadet = (char *)malloc((size_t)(number * (str_len_ + 1)));
		if (temp_smndatadet == NULL)
			break;

		memset(temp_smndatadet, 0x00, number * (str_len_ + 1));

		vector<smndatadet>::iterator it_smndatadet;
		for (it_smndatadet = smndatadet_vector.begin(); it_smndatadet != smndatadet_vector.end(); ++it_smndatadet) {
			memcpy(temp_smndatadet + offsetpos, &(it_smndatadet->organ_code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndatadet + offsetpos, &(it_smndatadet->type), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndatadet + offsetpos, &(it_smndatadet->reason), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_smndatadet + offsetpos, &(it_smndatadet->header.count), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_smndatadet + offsetpos, &(it_smndatadet->header.send), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.SMNDATADET(ORGAN_CODE,DATA_TYPE,REASON,COUNT_TIME,SEND_TIME) "
			"VALUES(:1,:2,:3,to_date(:4,'YYYY-MM-DD HH24:MI:SS'),to_date(:5,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("SMNDATADET", query, temp_smndatadet, number, cyc_, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _SMNDATADET_, error_info, threadlog);
			Log("Insert corr fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _SMNDATADET_, "���ɹ�.", threadlog);
		}

		free(temp_smndatadet);
		free(attrs);
		attrs = NULL;
		temp_smndatadet = NULL;
	}
		break;
	case _YXBS_:
	{
		int offsetpos = 0, number = 0, str_len_ = 0, cyc_ = 8;
		char query[1024];
		memset(query, 0x00, sizeof(query));

		printf("%s:[yxbsdet]:%d\n", filename, yxbsdet_vector.size());

		if (yxbsdet_vector.size() == 0) {
			SetGError(0, 0, NULL, _SUBS_, "�ļ�����Ϊ��", threadlog);
			break;
		}

		struct ColAttr* attrs = (ColAttr*)malloc(sizeof(ColAttr_t) * cyc_);
		if (attrs == NULL)
			break;

		attrs[0].data_type = DCI_INT;
		attrs[0].data_size = INT_LEN;
		attrs[1].data_type = DCI_STR;
		attrs[1].data_size = STR_LEN_CHAR;
		attrs[2].data_type = DCI_STR;
		attrs[2].data_size = STR_LEN_ZDNAME;
		attrs[3].data_type = DCI_STR;
		attrs[3].data_size = STR_LEN_CHAR;
		attrs[4].data_type = DCI_STR;
		attrs[4].data_size = STR_LEN_ZDNAME;
		attrs[5].data_type = DCI_INT;
		attrs[5].data_size = INT_LEN;
		attrs[6].data_type = DCI_STR;
		attrs[6].data_size = STR_LEN_SS;
		attrs[7].data_type = DCI_STR;
		attrs[7].data_size = STR_LEN_SS;
		
		for (int i = 0; i < cyc_; ++i) {
			str_len_ += attrs[i].data_size;
		} // 

		number = yxbsdet_vector.size();
		char *temp_yxbsdet = (char *)malloc((size_t)(number * (str_len_ + 1)));
		if (temp_yxbsdet == NULL)
			break;

		memset(temp_yxbsdet, 0x00, number * (str_len_ + 1));

		vector<yxbsdet>::iterator it_yxbsdet;
		for (it_yxbsdet = yxbsdet_vector.begin(); it_yxbsdet != yxbsdet_vector.end(); ++it_yxbsdet) {
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->header.code), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->devid), STR_LEN_CHAR);
			offsetpos += STR_LEN_CHAR;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->devname), STR_LEN_ZDNAME);
			offsetpos += STR_LEN_ZDNAME;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->feederid), STR_LEN_CHAR);
			offsetpos += STR_LEN_CHAR;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->feedername), STR_LEN_ZDNAME);
			offsetpos += STR_LEN_ZDNAME;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->type), INT_LEN);
			offsetpos += INT_LEN;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->header.count), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
			memcpy(temp_yxbsdet + offsetpos, &(it_yxbsdet->header.send), STR_LEN_SS);
			offsetpos += STR_LEN_SS;
		}

		strcpy(query, "INSERT INTO EVALUSYSTEM.DETAIL.YXZTBSDETAIL(ORGAN_CODE,DEVID,DEVNAME,FEEDERID,FEEDERNAME,ERR_TYPE,COUNT_TIME,SEND_TIME) "
			"VALUES(:1,:2,:3,:4,:5,:6,to_date(:7,'YYYY-MM-DD HH24:MI:SS'),to_date(:8,'YYYY-MM-DD HH24:MI:SS'))");

		if (d5000.d5000_WriteData("YXBSDET", query, temp_yxbsdet, number, cyc_, attrs, error_info) != 0) {
			SetGError(2, 0, g_strfiletypeName[arg], _YXBS_, error_info, threadlog);
			Log("Insert corr fail.\n");
			ret = ret | errmask;
		}
		else {
			SetGError(1, 0, g_strfiletypeName[arg], _YXBS_, "���ɹ�.", threadlog);
		}

		free(temp_yxbsdet);
		free(attrs);
		attrs = NULL;
		temp_yxbsdet = NULL;
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
	gzzsq_vector.clear();

	zdzx_name_vector.clear();

	bus_vector.clear();
	cb_vetor.clear();
	dsc_vector.clear();
	trans_vector.clear();
	subs_vector.clear();
	lines_vector.clear();

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
	ztgjfromone_vector.clear();
	dmsyd_vector.clear();
	gpmsyd_vector.clear();
	m_mNewId2CorrInfo.clear();
	m_mNewId2Bus.clear();
	m_mNewId2Subs.clear();
	m_mNewId2Trans.clear();
	m_mNewId2Cb.clear();
	m_mNewId2Dsc.clear();

	gomsend_vector.clear();
	smndata_vector.clear();
	smndatadet_vector.clear();
	yxbsdet_vector.clear();
}


int resolvexmldata::IsFilterFile(const char *_name)
{
	vector<string>::iterator itr_ = m_vKeyToNull.begin();
	for (; itr_ != m_vKeyToNull.end();++itr_) {

		if (strstr(_name,itr_->c_str()) != NULL) {
			printf("FilterFile:%s\n", _name);
			return 1;
		} // 
	} // 

	return 0;
}

#ifdef _USE_
//������¼����log��
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
		sprintf(content,"DMS�����վ�������%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
		break;
	case _DATT_:
		sprintf(content,"DMS�����Զ���Ͷ�˼�¼%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
		break;
	case _DAGC_:
		sprintf(content,"DMS�����Զ������̼�¼%s���%s,DMS�����Զ������̼�¼%s_info���%s",g_xmltypeName[arg],(retcode << 16)?"ʧ��":"�ɹ�",g_xmltypeName[arg],(retcode >> 16)?"ʧ��":"�ɹ�");
		break;
	case _YXBW_:
		sprintf(content,"DMS�����Զ����Ŀ���ң�ż�¼%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
		break;
	case _DMSGPZW_:
		sprintf(content,"DMSDMS������Ϣ%s_brand���%s,DMSDMS��λ��Ϣ%s_status���%s",g_xmltypeName[arg],(retcode << 16)?"ʧ��":"�ɹ�",g_xmltypeName[arg],(retcode >> 16)?"ʧ��":"�ɹ�");
		break;
	case _GPMSGPZW_:
		sprintf(content,"DMSGPMS������Ϣ%s_brand���%s,DMSGPMS��λ��Ϣ%s_status���",g_xmltypeName[arg],(retcode << 16)?"ʧ��":"�ɹ�",g_xmltypeName[arg],(retcode >> 16)?"ʧ��":"�ɹ�");
		break;
	case _ZDZX_:
		sprintf(content,"DMS�ն��������%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
		break;
	case _YKCZ_:
		sprintf(content,"DMSң�ز������%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
		break;
	case _YXINFO_:
		sprintf(content,"DMSң����ȷ���%s���%s",g_xmltypeName[arg],retcode?"ʧ��":"�ɹ�");
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

//ɾ��ָ�����ڼ�¼
int resolvexmldata::deleteDBDate(int code,int arg)
{
	unsigned int mask = 1,retcode = 0;
	char sql[250];
	memset(sql,0x00,sizeof(sql));

	//enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_DMSCIME_,_GPMSCIME_,_DMSDATA_,_GPMSDATA_,_END_,_UnKnownFileType_}g_filetype;
	//enum filetype {_ZZZX_,_DATT_,_DAGC_,_YXBW_,_GPZW_,_DMSGPZW_,_GPMSGPZW_,_ZDZX_,_YKCZ_,_YXINFO_,_RATE_,_BUS_,_CB_,_DSC_,_TRANS_,_SUBS_,_TDTRANS_,_AUTOMAP_,_TQRELEASE_,_FASTR_,_TZLIST_,_GPMSTD_,_GPMSTDDET_,_GPMSZLPDET_,_DMSZLPDET_,_DMSXSD_,_DMSZTGJ_,_DMSYD_,_GZZSQ_,_YCRT_,_END_,_UnKnownFileType_}g_filetype;
	enum filetype {
		_ZZZX_,
		_DATT_,
		_DAGC_,
		_YXBW_,
		_GPZW_,
		_DMSGPZW_,
		_GPMSGPZW_,
		_ZDZX_,
		_YKCZ_,
		_YXINFO_,
		_RATE_,
		_BUS_,
		_CB_,
		_DSC_,
		_TRANS_,
		_SUBS_,
		_TDTRANS_,
		_AUTOMAP_,
		_TQREALEASE_,
		_FASTR_,
		_TZLIST_,
		_GPMSTD_,
		_GPMSTDDET_,
		_GPMSZLPDET_,
		_DMSZLPDET_,
		_DMSXSD_,
		_DMSYD_,
		_GPMSYD_,
		_GZZSQ_,
		_DMSYCRT_,
		_DMSEMSRT_,
		_DMSYXRT_,
		_DMSLINE_,
		_DMSXSDET_,
		_DMSZTGJDET_,
		_DMSTRANSUP_, /*������(GIS1.6)���*/
		_DMSCBUP_,	/*������(GIS1.6)����*/
		_DMSDSCUP_,	/*������(GIS1.6)��բ*/
		_DMSUBSUP_,	/*������(GIS1.6)վ��*/
		_DMSBUSUP_,	/*������(GIS1.6)ĸ��*/
		_DMSDEVCORR_,	/*����ǰ(GIS1.5)��������(GIS1.6)�豸ID��Ӧ̨��*/
		_GOMSEND_,
		_SMNDATA_,
		_SMNDATADET_,
		_YXBS_,
		_END_,
		_UnKnownFileType_
	}g_filetype;
	//const char *g_xmltypeName[] = {"DMSOP","DATT","DAGC","YXBW","GPZW","DMSGPZW","GPMSGPZW","ZDZX","YKCZ","YXINFO","DMSCIME","GPMSCIME","DMSDATA","GPMSDATA","END"};
	const char *g_xmltypeName[] = { /*����*/
		"DMSOP",
		"DATT",
		"DAGC",
		"YXBW",
		"GPZW",
		"DMSBRAND",
		"GPMSGPZW",
		"ZDZX",
		"YKCZ",
		"YXINFO",
		"YCRATE",
		"DMSBUS",
		"DMSCB",
		"DMSCB",
		"DMSLD",
		"DMSST",
		"DMSTD",
		"AUTOGRAPH",
		"TDINFO",
		"FACTION",
		"TZDETAILS",
		"GPMSTD",
		"GPMSTDDET",
		"GPMS_ZLP",
		"DMS_ZLP",
		"DMS_XSD",
		"DMS_YD",
		"GPMS_YD",
		"GZZSQ",
		"YCRT",
		"EMSRT",
		//"YXRT",
		/* // @detail ȥ��ԭ��ң��״̬��ʶ�ˣ�����Ϊһ������״̬��ʶ������ϸ */
		"DMSZTGJCBDET",
		"DMSLINE",
		"DMS_XSD_DET",
		"DMS_ZTGJ_DET",
		"DMSLDUP",
		"DMSCBUP",
		"DMSCBUP",
		"DMSSTUP",
		"DMSBUSUP",
		"DMSCORR", /*��Ӧ��ϵ��*/
		"GOMSPUSH",
		"SMNDATA",
		"SMNDATADET",
		"YXZTBSDETAIL",
		"END"
	};
	
	switch (arg)
	{
	case _ZZZX_:
	case _DATT_:
	case _YXBW_:
	case _ZDZX_:
	case _YKCZ_:
	case _YXINFO_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and SEND_TIME > to_date(sysdate,'YYYY-MM-DD')", g_xmltypeName[arg], code);

		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;

		break;
	case _DAGC_:
		//sprintf(sql,"DELETE FROM DETAIL.dagc WHERE COUNT_TIME = (select COUNT_TIME FROM DETAIL.dagc WHERE COUNT_TIME = '%s')",intime);
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;

		/*sprintf(sql,"DELETE FROM DETAIL.dagc_info WHERE COUNT_TIME '%s'",intime);
		retcode = d5000_insert.DeleteDBDate(sql);
		if (retcode != 0)
		retcode = retcode | (mask << 16);*/
		break;
	case _DMSGPZW_:
	{
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);

		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
	}
		break;
	case _GPMSGPZW_:
	{
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
	}
		break;
	case _BUS_:
	case _TRANS_:
	case _SUBS_:
	case _DMSBUSUP_:
	case _DMSTRANSUP_:
	case _DMSUBSUP_:
	case _DMSDEVCORR_:
	{
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
	}
		break;
	case _CB_:
	case _DMSCBUP_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD') and type = 1", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;

		break;
	case _DSC_:
	case _DMSDSCUP_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and GTIME = to_date(sysdate-1,'YYYY-MM-DD') and type = 0", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;

		break;
	case _TDTRANS_:
	case _AUTOMAP_:
	case _TQREALEASE_:
	case _FASTR_:
	case _TZLIST_:
	case _RATE_:
	case _DMSZLPDET_:
	case _DMSYD_:
	case _GZZSQ_:
	case _GPMSYD_:
	case _GPMSZLPDET_:
	case _DMSYCRT_:
	case _DMSEMSRT_:
	case _DMSLINE_:
	case _DMSXSDET_:
	case _DMSZTGJDET_:
	case _GOMSEND_:
	case _SMNDATA_:
	case _SMNDATADET_:
	case _YXBS_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
		break;
	case _DMSXSD_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code like '%d%%' and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
		break;
	case _DMSYXRT_:
		/* // @detail ״̬�����ļ�Ϊ����ʵʱ����һ��������count_timeΪǰ��ʱ�� */
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code like '%d%%' and COUNT_TIME = to_date(sysdate-2,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
		break;
	/*case _DMSTRANSUP_:
	case _DMSCBUP_:
	case _DMSDSCUP_:
	case _DMSUBSUP_:
	case _DMSBUSUP_:*/
	/*case _DMSDEVCORR_:
		sprintf(sql, "DELETE FROM DETAIL.%s WHERE organ_code = %d and COUNT_TIME = to_date(sysdate-1,'YYYY-MM-DD')", g_xmltypeName[arg], code);
		retcode = d5000.DeleteDBDate(sql);
		printf("%s\n", sql);
		if (retcode != 0)
			retcode = retcode | mask;
		break;*/
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

	cout << "ע�⣺��Ļ�����д����־�ļ�������cat����鿴" << szLogPath << "Ŀ¼����Ӧ�ļ���" << endl;

	pipe(pipefd);

	//�������������Ϊ�л���ģʽ
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

			//������µ�һ�죬�����½����µ���־�ļ�
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

int ReadFileKeyFromFilterFile()
{
	if (d5000.d5000_Connect() != 0)
		return -1;

	char query[200], err_info[200], tempstr[100], tmp[20];
	char *buf = NULL, *m_presult = NULL;
	struct ColAttr *attrs = NULL;

	m_vKeyToNull.clear();

	memset(query, 0x00, sizeof(query));
	memset(err_info, 0x00, sizeof(err_info));
	strcpy(query, "SELECT KEY FROM EVALUSYSTEM.CONFIG.FILTERFILE;");

	int rec_num = 0, attr_num = 0, rec = 0, col = 0, offset = 0;
	if (d5000.d5000_ReadData(query, &rec_num, &attr_num, &attrs, &buf, err_info) == 0) {
		m_presult = buf;

		for (rec = 0; rec < rec_num; rec++) {

			for (col = 0; col < attr_num; col++) {

				memset(tempstr, 0x00, 100);
				memcpy(tempstr, m_presult, attrs[col].data_size);
				switch (col) {
				case 0:
					m_vKeyToNull.push_back(tempstr);
					break;
				default:
					break;
				}

				m_presult += attrs[col].data_size;
			}
		}
	}
	else {
		d5000.d5000_DisConnect();
		return -1;
	}

	if (rec_num > 0)
		d5000.g_CDci.FreeReadData(attrs, attr_num, buf);
	else
		d5000.g_CDci.FreeColAttrData(attrs, attr_num);

	d5000.d5000_DisConnect();

	return 0;
}

//У��ģ���ļ��Ƿ��ͣ�add by lcm 20150910
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
			printf("model_check:%s\n",dirt->d_name);
			memset(my_organ_code, 0x00, sizeof(my_organ_code));
			pstr=dirt->d_name;
			pdstr=strstr(dirt->d_name,"_");
			if (pdstr == NULL) {

				printf("strstr error:%s\n",dirt->d_name);
				continue;
			}
			strncpy(my_organ_code,pstr,pdstr-pstr);
			organ_code = atoi(my_organ_code);
			//������ڸ�organ_code��ģ���ļ�����ɾ�����ݿ��иõ���δ�ṩģ�ͼ�¼ add by lcm 20150910
			memset(sql,0,sizeof(sql));
			sprintf(sql,"delete from evalusystem.manage.err_file where organ_code =%d and dis = 'ģ���ļ�δ�ṩ';",organ_code);
			if(d5000.d5000_ExecSingle(sql)!=0)
			{
				printf("sql���ִ��ʧ��");
				return -1;
			}
		}
	}
	closedir(dirp);
	return 0;
}

int init_modelcheck()
{
	//���ģ��У���ʼ������
	char error_info[256];
	for(int i=0;i<vec_organ.size();i++)
	{
		int offsetpos_model = 0;
		char qury[200];
		memset(qury,0x00,sizeof(qury));
		memset(error_info,0x00,sizeof(error_info));
		struct ColAttr* attrs_model = (ColAttr* )malloc(sizeof(ColAttr_t)*2);
		if(attrs_model == NULL)
			return 0;
		attrs_model[0].data_type = DCI_INT;
		attrs_model[0].data_size = INT_LEN;
		attrs_model[1].data_type = DCI_STR;
		attrs_model[1].data_size = STR_LEN_NAME;
		char *temp_err = (char *)malloc((size_t)(1000));
		if(temp_err == NULL)
			return 0;
		memset(temp_err,0x00,1000);
		
		memcpy(temp_err+offsetpos_model,&vec_organ[i],INT_LEN);
		offsetpos_model += INT_LEN;
		memcpy(temp_err+offsetpos_model,&(g_config.cime_path),STR_LEN_NAME);
		offsetpos_model += STR_LEN_NAME;
		
		sprintf(qury,"insert into evalusystem.manage.err_file(err_file_path,date,organ_code,count_file_path,dis,source) values "
			"(null,to_date(sysdate,'YYYY-MM-DD HH24:MI:SS'),:1,:2,'ģ���ļ�δ�ṩ','dms');");
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

int SplitStrByCh(const char *_str, const char *_home, char _c, char _des[][256])
{
	int pos_ = 0,count_ = 0;
	int home_len_ = strlen(_home);
	int len_temp_ = strlen(_str);

	for (int i = 0; i < len_temp_;++i) {
		
		if (i == 0) {
			sprintf(_des[0], "%s/", _home);
			count_ = home_len_+1;
		} // 

		if (_str[i] == _c	) {
			++pos_;
			sprintf(_des[pos_], "%s/", _home);
			count_ = home_len_+1;
		} //
		else {
			_des[pos_][count_++] = _str[i];
		}
	} // 

	return 0;
}
