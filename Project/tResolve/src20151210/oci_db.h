#ifndef  _INTROCI_H
#define  _INTROCI_H

#include "dcisg.h"
#include "comm_def.h"

#define IN
#define OUT

class D5000_DB
{
public:
	D5000_DB();
	virtual ~D5000_DB();
	int d5000_DisConnect();
	int d5000_Connect();
	int d5000_ReadData(IN const char *query, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf,char * error_info);
	int d5000_WriteData(IN const char * type,IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,OUT char * error_info);
	int d5000_ExecSingle(IN const char *sqlstr);
	int DeleteDBDate(const char *sql);

	class CDci  g_CDci;
	ErrorInfo_t error;
};

extern D5000_DB d5000;
#endif
