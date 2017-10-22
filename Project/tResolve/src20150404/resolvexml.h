#ifndef _SOLVEXML_H
#define _SOLVEXML_H

#include "comm_def.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "oci_db.h"
#include <string.h>
#include <errno.h>
#include <time.h>


using namespace std;

#pragma pack(push)
#pragma pack(1)

typedef struct {
	pthread_mutex_t queue_mutex_file;          
	pthread_cond_t queue_cond_file;
	queue<string> queue_file;
}xml_arg;

//��Ϣͷ
typedef struct {
	int code;      //�����վ�������繫˾
	char count[30];		//����ͳ��ʱ��YYYY-MM-DD
	char send[30];		//��Ϣ����ʱ��YYYY-MM-DD_HH-MM-SS
}mes_header;

typedef struct {
	char source[10];	//Դϵͳ��־
	char type[10];		//ͳ������
	char sequence[50];  //��ϢΨһ��ʶ���������վ�ڲ�Ψһ
}header_sign;

//��վ��������
typedef struct {
	mes_header header;
	int online;			//��վ����ʱ������λΪ��
}dmsop;

//�����Զ���Ͷ�˼�¼
typedef struct {
	mes_header header;
	char dv[50];	//���߱��
	char gtime[30];	//�¼�����ʱ��YYYY-MM-DD_HH-MM-SS
	int src;        //�¼���0-Ͷ�룬1-�˳�
	int type;       //ȫ�Զ�����Զ� 0-ȫ�Զ� 1-���Զ�
	int allnum;     //��������
}datt;

//�����Զ������̼�¼
typedef struct {
	mes_header header;
	char da[50]; //���ι��ϴ����ı��
	char dv[50]; //���߱��
	int step;	 //�����׶Σ�0-������1-��λ��2-���룬3-�ָ���4-����
	char gtime[30]; //�׶ν���ʱ��YYYY-MM-DD_HH-MM-SS
	int result;   //�׶�ִ�н��0-ʧ�ܣ�1-�ɹ�
	char cb[50];  //�����׶Σ����ι��϶Ͽ����б�������
}dagc;

typedef struct {
	mes_header header;
	char da[30]; //���ι��ϴ����ı��
	char gtime[30]; //�׶ν���ʱ��YYYY-MM-DD_HH-MM-SS
	int step;	 //�����׶Σ�0-������1-��λ��2-���룬3-�ָ���4-����
	char cb[50];//�����׶�-���͹����źŵĿ���,����׶�-�����Ŀ���,�ָ��׶�-�����Ŀ���
	int src; //���ر�λ�����0-�Ͽ���1-�պ�
}dagc_info;


//ң�ż�¼��
typedef struct {
	mes_header header;
	char cb[50];   //���ر��
	char gtime[30]; //�¼�����ʱ��YYYY-MM-DD_HH-MM-SS
	int src;        //�¼����ͣ�0-�Ͽ���1-�պ�
	int type;       //��λԭ��0-�����ֶ���1-ң�أ�2-����������3-�˹�������4-���������Ի�λ��ԭ��
}yxbw;

//��λ��Ϣ
typedef struct {
	mes_header header;
	char devid[50]; //�豸���
	char devtype[30]; //�豸���ͣ���ֻΪcb��
	char gtime[30];   //�¼�����ʱ��YYYY-MM-DD_HH-MM-SS
	int src;          //�������ͣ�0-���ƣ�1-����ƣ�2-�˹��úϣ�3-�˹��÷�
}dmsstatus;

//������Ϣ
typedef struct {
	dmsstatus status;
	int info;         //�������ͣ�0-���ޣ�1-XX��DMS���䣩
}dmsbrand;

//�ն��������
typedef struct {
	mes_header header;
	char link[50];    //��·���նˣ����
	char name[256]; //��·���նˣ�����
	int online;   //�ն�����ʱ������λΪ��
}zdzx;

//ң�ز������
typedef struct {
	mes_header header;
	char cb[50]; //������Ӧ�Ŀ��ر��
	char gtime[30]; //����ʱ��YYYY-MM-DD_HH-MM-SS
	int src; //�������ͣ�0-�غϣ�1-�ط�
	int result; //ִ�н����0-ʧ�ܣ�1-�ɹ�
}ykcz;

//ң����ȷ���
typedef struct {
	mes_header header;
	int total;  //ң�Ŷ����ܴ���
	int matchsoe; //ң����ȷ������������soeƥ��ģ�
	int local;   //�����ֶ����µ�ң�Ŷ�����������ң���أ�
}yxinfo;

//̨����
typedef struct{
	mes_header header;
	int total;
	int sendtotal;
}tqnum;

//ֱ�ӻ�ȡ��ָ��ֵ
typedef struct {
	mes_header header;
	float tq;
	float datasuc;
}rate;

//ĸ��
typedef struct {
	mes_header header;
	char busid[100];
	char busname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//��ѹ�ȼ�
}bus;

//����
typedef struct {
	mes_header header;
	char cbid[100];
	char cbname[100];
	char dvid[100];
	char dvname[100];
	int type; //����Ϊ1
	int status;
	char vol[50];	//��ѹ�ȼ�
	int flag;      //1:����״̬�ȶ� 0:������״̬�ȶ�
	char subsid[100]; //����վ��id
}cb;

//��բ
typedef struct {
	mes_header header;
	char dscid[100];
	char dscname[100];
	char dvid[100];
	char dvname[100];
	int type; //��բΪ0
	int status;
	char vol[50];	//��ѹ�ȼ�
	int flag;      //1:����״̬�ȶ� 0:������״̬�ȶ�
	char subsid[100];//����վ��id
}dsc;

//���
typedef struct {
	mes_header header;
	char transid[100];
	char transname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//��ѹ�ȼ�
}trans;

//վ��
typedef struct {
	mes_header header;
	char subsid[100];
	char subsname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//��ѹ�ȼ�
}subs;

//DMS��Ϣ������
typedef struct {
	mes_header header;
	int tdtrans;
	int wrongtrans;
	int realtrans;
}tdtrans;

//�Զ���ͼ
typedef struct {
	mes_header header;
	int alrmap;
	int shoumap;
}automap;

//ͣ��
typedef struct {
	mes_header header;
	char dmstd[50];
	char ddaff[50];
	char dmsaff[50];
	int tdnum;
	char tdintnr[8000];
	//char sendtype[50];
	int sendtype;
	char judged_time[50];
}tqrelease;

//FAչʾ
typedef struct {
	mes_header header;
	int num;
	char dvid[100];
	char str[20000];
}fastr;

//̨��ƥ������ϸ
typedef struct {
	mes_header header;
	char tz[100];
	char belong[50];
	char id[100];
	char name[256];
}tzlist;

//���ݼ�¼
typedef struct {
	int organ_code;
	char name[100];
	int value;
	char reason[100];
}datarec;

//gpmsͣ���������
typedef struct {
	mes_header header;
	int allnum;
	int citycode;
	char cityname[100];
}gpmstd;


//gpmsͣ�������ϸ
typedef struct {
	mes_header header;
	int citycode;
	char cityname[100];
	char dmsid[50];
	char gpmsid[50];
	char fault_time[100];
	char return_time[100];
	char fault_line[256];
	char fault_no[50];
	char bills_num[50];
	char details[256];
	char tdintr[4000];
	char judged_time[50];
	char disdesk[50];
	char belarea[50];
}gpmstddet;

//����̨��ӦID
typedef struct {
	int organ_code;
	char area[50];
	char desk[50];
}disdesk;


typedef struct logstr{
	int arg;         //�ɹ���ʧ�ܱ�־0�ļ�Ϊ�ա�1�ɹ���2ʧ��
	int code;        //���繫˾����
	int  name;		//ָ����
	char log[500];   //��־��Ϣ
}log;

#pragma pack(pop)

class resolvexmldata
{

public:
	resolvexmldata(){};
	~resolvexmldata(){};
	void mem_init();
	int readdata_xml(const char* file,logstr& threadlog);
	int readdata_cime(int seq);
	int readdata_file(const char *filename,logstr& threadlog);
	int import_xml(TiXmlDocument &doc,const char *filename,logstr& threadlog);
	int Combination_cime(const char *src_file,const char *des_file,int flag);
	int insertDB(int arg,logstr& threadlog);
	int insertDBLOG(int arg,int code,int retcode,const char *filename);
	int deleteDBDate(int code,int arg);
	void SetGError(int arg,int code,const char * filename,int watch,const char * error_info,logstr& threadlog);
	void GetGError(logstr& threadlog);
	int convertUTF8toANSI(const char *inbuf,char *outbuf,int outlen);
	int convert(const char *fcode,const char *tcode,const char *inbuf,char *outbuf);

	void FreeSize();

	//D5000_DB d5000;

	vector<dmsop> dmsop_vector;
	vector<datt> datt_vector;
	vector<dagc> dagc_vector;
	vector<dagc_info> dagc_info_vector;
	vector<yxbw> yxbw_vector;
	vector<dmsbrand> dmsbrand_vector;
	vector<dmsstatus> dmsstatus_vector;
	vector<dmsbrand> gpmsbrand_vector;
	vector<dmsstatus> gpmsstatus_vector;
	vector<zdzx> zdzx_vector;
	vector<ykcz> ykcz_vector;
	vector<yxinfo> yxinfo_vector;
	vector<tqnum> tqnum_vector;
	vector<rate> rate_vector;


	vector<string> zdzx_name_vector;

	vector<bus> bus_vector;
	vector<cb> cb_vetor;
	vector<dsc> dsc_vector;
	vector<trans> trans_vector;
	vector<subs> subs_vetor;

	vector<tdtrans> tdtrans_vector;
	vector<automap> automap_vector;
	vector<tqrelease> tqrelease_vector;
	vector<fastr> fastr_vector;
	vector<tzlist> tz_vector;

	vector<gpmstd> gpmstd_vetor;
	vector<gpmstddet> gpmstddet_vector;
	vector<string> tzlist_qc_vector;
};

class mvcim
{
public:
	int mvtocim(char *code);
	int getfilenum(char *path);
	int unzip_fun(char *filename,char *code);
	int gethavexml();
	int transzip();

	vector<string> xml_vector;
	vector<string> fz_vector;
	vector<string> xm_vector;
	vector<string> nd_vector;
	vector<string> pt_vector;
	vector<string> qz_vector;
	vector<string> zz_vector;
	vector<string> ly_vector;
	vector<string> sm_vector;
	vector<string> np_vector;

	//map<string,string> xml_map;
	multimap<string,string> fz_map;
	multimap<string,string> xm_map;
	multimap<string,string> nd_map;
	multimap<string,string> pt_map;
	multimap<string,string> qz_map;
	multimap<string,string> zz_map;
	multimap<string,string> ly_map;
	multimap<string,string> sm_map;
	multimap<string,string> np_map;
};


#define XML_OVERALL extern
#define MAX_THR_RESOLVE 100

XML_OVERALL char *thr_xml_resolve[MAX_THR_RESOLVE];
XML_OVERALL pthread_t thr_xml[MAX_THR_RESOLVE];
XML_OVERALL void * thr_result[MAX_THR_RESOLVE];
XML_OVERALL xml_arg thr_xml_mutex;
XML_OVERALL pthread_rwlock_t lockfile;
XML_OVERALL int del;
XML_OVERALL pthread_mutex_t text_tqrelease_mutex;
XML_OVERALL pthread_mutex_t text_FA_mutex;
XML_OVERALL multimap<string,disdesk> disdesk_map; 

//XML_OVERALL class resolvexmldata reslov_file;

#endif