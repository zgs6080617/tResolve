#include "oci_db.h"

using namespace std;

D5000_DB::D5000_DB()
{

}

D5000_DB::~D5000_DB()
{
	/*if(d5000_DisConnect(&error) == 0)
	cout << "DisConnect error_no:" << error.error_no << " error_info:" << error.error_info << endl;*/
}

int D5000_DB::d5000_Connect()
{
	ErrorInfo_t error;
	cout << "server:" << g_config.server << " user:" << g_config.user << " password:" << g_config.password << endl;
	if(g_CDci.Connect(g_config.server,g_config.user,g_config.password,&error) == 0)
	{
		cout << "Connect error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		return error.error_no;
	}
	cout<<"数据库连接成功!"<<endl;
	return 0;
}

int D5000_DB::d5000_DisConnect()
{
	ErrorInfo_t error;
	if (g_CDci.DisConnect(&error) == 0)
	{
		cout << "Disconnect error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		return error.error_no;
	}
	cout<<"数据库断开成功!"<<endl;
	return 0;
}

int D5000_DB::d5000_WriteData(IN const char * type,IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,OUT char * error_info)
{
	ErrorInfo_t error;
	if(g_CDci.WriteData(query,buf,rec_num,attr_num,attrs,&error) == 0)
	{
		//SetGError(1,type,error.error_info);
		if(error_info != NULL)
			strncpy(error_info,error.error_info,sizeof(error_info));
		cout << "WriteData " << query << type << " error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		if(error.error_no == 3113)
		{
			//d5000.d5000_Connect();
			g_CDci.ReConnect(&error);
			d5000.d5000_WriteData(type,query,buf,rec_num,attr_num,attrs,error_info);
		}
		return error.error_no;
	}
	/*else
	{
		SetGError(0,type,"Write DB Success.");
	}*/
	return 0;
}

int D5000_DB::d5000_ExecSingle(IN const char *sqlstr)
{
	ErrorInfo_t error;
	if(g_CDci.ExecSingle(sqlstr,&error) == 0)
	{
		cout << "ExSingle:" << sqlstr << " error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		if(error.error_no == 3113)
		{
			//d5000.d5000_Connect();
			g_CDci.ReConnect(&error);
			d5000.d5000_ExecSingle(sqlstr);
		}
		return error.error_no;
	}
	return 0;
}

int D5000_DB::d5000_ReadData(IN const char *query, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf,char * error_info)
{
	ErrorInfo_t error;
	if(g_CDci.ReadData(query,rec_num,attr_num,attrs,buf,&error) == 0)
	{
		if(error_info != NULL)
			strncpy(error_info,error.error_info,sizeof(error_info));
		cout << "ReadData " << query << " error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		if(error.error_no == 3113)
		{
			//d5000.d5000_Connect();
			g_CDci.ReConnect(&error);
			d5000.d5000_ReadData(query,rec_num,attr_num,attrs,buf,error_info);
		}
		return error.error_no;
	}
	return 0;
}

int D5000_DB::DeleteDBDate(const char *sql)
{
	ErrorInfo_t error;
	if(g_CDci.ExecSingle(sql,&error) == 0)
	{
		cout << "ExecSingle "<< sql << "error_no:" << error.error_no << " error_info:" << error.error_info << endl;
		if(error.error_no == 3113)
		{
			//d5000.d5000_Connect();
			g_CDci.ReConnect(&error);
			d5000.DeleteDBDate(sql);
		}
		return error.error_no;
	}
	return 0;
}
