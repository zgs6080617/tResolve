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

//消息头
typedef struct {
	int code;      //配电主站所属供电公司
	char count[30];		//数据统计时间YYYY-MM-DD
	char send[30];		//消息发送时间YYYY-MM-DD_HH-MM-SS
}mes_header;

typedef struct {
	char source[10];	//源系统标志
	char type[10];		//统计类型
	char sequence[50];  //消息唯一标识，各配电主站内部唯一
}header_sign;

//主站在线数据
typedef struct {
	mes_header header;
	int online;			//主站在线时长，单位为秒
}dmsop;

//馈线自动化投退记录
typedef struct {
	mes_header header;
	char dv[50];	//馈线编号
	char gtime[30];	//事件发生时间YYYY-MM-DD_HH-MM-SS
	int src;        //事件：0-投入，1-退出
	int type;       //全自动或半自动 0-全自动 1-半自动
	int allnum;     //馈线总数
}datt;

//馈线自动化过程记录
typedef struct {
	mes_header header;
	char da[50]; //本次故障处理的编号
	char dv[50]; //馈线编号
	int step;	 //处理阶段：0-启动，1-定位，2-隔离，3-恢复，4-结束
	char gtime[30]; //阶段结束时间YYYY-MM-DD_HH-MM-SS
	int result;   //阶段执行结果0-失败，1-成功
	char cb[50];  //启动阶段：本次故障断开的有保护开关
}dagc;

typedef struct {
	mes_header header;
	char da[30]; //本次故障处理的编号
	char gtime[30]; //阶段结束时间YYYY-MM-DD_HH-MM-SS
	int step;	 //处理阶段：0-启动，1-定位，2-隔离，3-恢复，4-结束
	char cb[50];//启动阶段-发送故障信号的开关,隔离阶段-操作的开关,恢复阶段-操作的开关
	int src; //开关变位情况：0-断开，1-闭合
}dagc_info;


//遥信记录表
typedef struct {
	mes_header header;
	char cb[50];   //开关编号
	char gtime[30]; //事件发生时间YYYY-MM-DD_HH-MM-SS
	int src;        //事件类型：0-断开，1-闭合
	int type;       //变位原因：0-本地手动，1-遥控，2-保护动作，3-人工置数，4-其它（测试或位置原因）
}yxbw;

//置位信息
typedef struct {
	mes_header header;
	char devid[50]; //设备编号
	char devtype[30]; //设备类型（暂只为cb）
	char gtime[30];   //事件发生时间YYYY-MM-DD_HH-MM-SS
	int src;          //操作类型：0-挂牌，1-解挂牌，2-人工置合，3-人工置分
}dmsstatus;

//挂牌信息
typedef struct {
	dmsstatus status;
	int info;         //挂牌类型：0-检修，1-XX（DMS补充）
}dmsbrand;

//终端在线情况
typedef struct {
	mes_header header;
	char link[50];    //链路（终端）编号
	char name[256]; //链路（终端）名称
	int online;   //终端在线时长，单位为秒
}zdzx;

//遥控操作情况
typedef struct {
	mes_header header;
	char cb[50]; //操作对应的开关编号
	char gtime[30]; //操作时间YYYY-MM-DD_HH-MM-SS
	int src; //操作类型：0-控合，1-控分
	int result; //执行结果：0-失败，1-成功
}ykcz;

//遥信正确情况
typedef struct {
	mes_header header;
	int total;  //遥信动作总次数
	int matchsoe; //遥信正确动作次数（和soe匹配的）
	int local;   //本地手动导致的遥信动作次数（三遥开关）
}yxinfo;

//台区号
typedef struct{
	mes_header header;
	int total;
	int sendtotal;
}tqnum;

//直接获取的指标值
typedef struct {
	mes_header header;
	float tq;
	float datasuc;
}rate;

//母线
typedef struct {
	mes_header header;
	char busid[100];
	char busname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//电压等级
}bus;

//开关
typedef struct {
	mes_header header;
	char cbid[100];
	char cbname[100];
	char dvid[100];
	char dvname[100];
	int type; //开关为1
	int status;
	char vol[50];	//电压等级
	int flag;      //1:参与状态比对 0:不参与状态比对
	char subsid[100]; //所属站房id
}cb;

//刀闸
typedef struct {
	mes_header header;
	char dscid[100];
	char dscname[100];
	char dvid[100];
	char dvname[100];
	int type; //刀闸为0
	int status;
	char vol[50];	//电压等级
	int flag;      //1:参与状态比对 0:不参与状态比对
	char subsid[100];//所属站房id
}dsc;

//配变
typedef struct {
	mes_header header;
	char transid[100];
	char transname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//电压等级
}trans;

//站房
typedef struct {
	mes_header header;
	char subsid[100];
	char subsname[100];
	char dvid[100];
	char dvname[100];
	char vol[50];	//电压等级
}subs;

//DMS信息发布率
typedef struct {
	mes_header header;
	int tdtrans;
	int wrongtrans;
	int realtrans;
}tdtrans;

//自动成图
typedef struct {
	mes_header header;
	int alrmap;
	int shoumap;
}automap;

//停电
typedef struct {
	mes_header header;
	char dmstd[50];
	char ddaff[50];
	char dmsaff[50];
	int tdnum;
	char tdintnr[20000];
}tqrelease;

//FA展示
typedef struct {
	mes_header header;
	int num;
	char dvid[100];
	char str[20000];
}fastr;

//台账匹配率明细
typedef struct {
	mes_header header;
	char tz[100];
	char belong[50];
	char id[100];
	char name[256];
}tzlist;

//数据记录
typedef struct {
	int organ_code;
	char name[100];
	int value;
	char reason[100];
}datarec;

//gpms停电故障数量
typedef struct {
	mes_header header;
	int allnum;
	int citycode;
	char cityname[100];
}gpmstd;


//gpms停电故障明细
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
	char details[4000];
	char tdintr[4000];
}gpmstddet;


typedef struct logstr{
	int arg;         //成功、失败标志0文件为空、1成功、2失败
	int code;        //供电公司编码
	int  name;		//指标编号
	char log[500];   //日志信息
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

//XML_OVERALL class resolvexmldata reslov_file;

#endif
