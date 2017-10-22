// Dci.h: interface for the CDci class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DCI_H__4343FFBB_7E79_49D0_B4FA_0DAB5B1C7A81__INCLUDED_)
#define AFX_DCI_H__4343FFBB_7E79_49D0_B4FA_0DAB5B1C7A81__INCLUDED_

#include "dcidefine.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if _MSC_VER >= 1400
#pragma warning(disable : 4996)
#endif
#define TABLE_TYPE_DAY			1
#define TABLE_TYPE_MONTH		2
#define TABLE_TYPE_YEAR			3
#define TABLE_TYPE_NONE			4

#define ONE_YEAR_SECOND		(60*60*24*31*12)
#define	ONE_MONTH_SECOND	(60*60*24*31)
#define ONE_DAY_SECOND		(60*60*24)
#define ONE_HOUR_SECOND		(60*60)
#define ONE_MINUTE_SECOND	60
#define ONE_SECOND			1
#define FREE_SECOND			0
#define IN
#define OUT
#define COL_NUM 1500
#define MAX_STRING_LEN 150			//E文件中字符串允许的最大长度
#define DCI_SUCCEED 1
#define DCI_FAIL   -1
#define RECNUM 2000
#define MAX_BLOB_LEN  0x0FFF		//大字段读取时允许的最大长度
/*******
typedef struct ORA_ATTR {
	char id[MAX_NAME_LEN + 1];
	int dtype;
	int col_width;
}ORA_ATTRS;
*******/

typedef ColAttr_t ParamAttr_t;

class CDci
{
public:
	bool ExecuteSqlWithRowBind(const char *sql, char *data, ParamAttr_t *in_param, ub2 in_param_nums, ub4 rows, ub4 &row_count, ErrorInfo_t *error);

	int GetSheetData(char *sql, int top_number, char** out_buf, vector<ColAttr_t> &attr, vector<ColDataItem> &data, ErrorInfo &error);
	int GetSheetData(CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, const char* order_by, ErrorInfo &error);
	int GetSheetData(CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, ErrorInfo &error);
	int SetSheetData(char *model_name, vector<SheetRecData> &rec, ErrorInfo &error);//这个函数自动提交操作
	int ReadDataTime_t(IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf, OUT ub4 *buf_size, ErrorInfo_t* error);
	bool Connect(const char* server, const char* user_name, const char* pasword, ErrorInfo &error);
	int ExecSqlReturnAffectRowCount(IN const char *sqlstr,ErrorInfo_t* error);
	static Curveinfo* FindCurveinfo(char* conf_id);
	int SetCurveData(CurvePara &param, CurveData &curvedata, ErrorInfo &error);
	int SetCurveData(CurvePara &param, CurveData *curvedatap, ub2 items, ErrorInfo &error);
	int GetCurveData(CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error);
	int GetCurveDataBat(int nParam, CurvePara *param, CurveData **datapp, int *items, ErrorInfo &error);
	int GetSheetData(CurvePara &param, char **datapp,ColAttr_t **attrpp ,char *col_str,char *where_str,int &items, ErrorInfo &error);
	bool PutHisData(char *model_name, vector<SheetRecData> &rec, ErrorInfo &error);
	bool ReConnect(ErrorInfo *error);
	bool ExecuteSqlWithResult(const char* sql, ParamAttr_t *in_param, ub2 in_param_nums, ResultAttr_t *resultp, ErrorInfo_t *error);
	bool ReadAttrs(char *tablename, int *attr_num,  struct ColAttr** attrs,ErrorInfo_t* error);
	bool ReadWithBind(char *query, char **buf, int *rec_num, int *attr_num, struct ColAttr** attrs ,ErrorInfo_t* error);
	bool UpdateWithBind(const char *query, const char *buf, const int rec_num, const int attr_num, const struct ColAttr* attrs,ErrorInfo_t* error);
	bool InsertWithBind(const char *query, const char *buf, int rec_num, int attr_num, const struct ColAttr* attrs ,ErrorInfo_t* error);
	bool ExecNoBind(const char *sqlstr,ErrorInfo_t* error);
	bool ExecNoBindAndCmit(const char *sqlstr,ErrorInfo_t* error);
	bool ExecCommit(ErrorInfo_t* error);
	bool ExecRollback(ErrorInfo_t* error);

	bool CreateTable(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ErrorInfo_t *error);
	bool ExecuteProcedure(const char *package_name,const char *procedure_name,
							ParamAttr_t *in_param,ub2 in_param_nums, ErrorInfo_t *error);
	bool ExecuteSql(const char *sql, ParamAttr_t *in_param, ub2 in_param_nums, ub4 iters, ErrorInfo_t *error);
	bool DirPathLoadStreamByRowBinds(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, void *data, ub1 input_type, ErrorInfo_t *error);
	bool DirPathLoadStreamByColumnBinds(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, ub1 input_type, ErrorInfo_t *error);
	bool DirPathLoadStream(const char* schema, const char* table, ColAttr_t* col, ub2 cols, ub4 rows, void *data, ub1 input_type, ErrorInfo_t *error);
	bool Connect(const char* server, const char* user_name, const char* pasword, bool is_init_format_tables, ErrorInfo_t* error);
	bool Connect(const char* server, const char* user_name, const char* pasword, ErrorInfo_t* error);
	bool DisConnect(ErrorInfo_t *error);
	bool IsConnected();
	int ReadData(IN const char *query, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs, OUT char **buf,ErrorInfo_t* error);
	int ReadData(IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf,ErrorInfo_t* error);
	int ReadDataEx (IN const char *query,IN int top_number, OUT int *rec_num, OUT int *attr_num,OUT struct ColAttr ** attrs,OUT char **buf, OUT ub4 *buf_size, ErrorInfo_t* error);
	int ReadDataNext (IN OUT void **stmthpp, IN OUT void **errhpp, IN const char *query, OUT struct ColAttr ** attrs, OUT int *attr_num, OUT char **buf, ErrorInfo_t* error);
	int FreeReadDataNext(IN OUT void **stmthpp, IN OUT void **errhpp, IN struct ColAttr ** attrs, OUT char **buf);
	bool WriteData(IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,ErrorInfo_t* error);
    bool WriteDataNoCmit(IN const char *query, IN const char *buf, IN const int rec_num, IN const int attr_num, IN const struct ColAttr* attrs,ErrorInfo_t* error); //Danath 20120221不自动提交版本

	bool GetAttrs(IN const char *query, OUT int *attr_num, struct ColAttr ** attrs,ErrorInfo_t* error);
	bool ExecSingle(IN const char *sqlstr,ErrorInfo_t* error);
	bool ExecSingleNoCmit(IN const char *sqlstr,ErrorInfo_t* error); //Danath 20120221不自动提交版本

	void PrintResultData(		//打印一个结果集到屏幕上
		ColAttr_t *col_attr,	//结果集中的列描述指针，这个指针批向一个ColAttr_t结构的数组
		ub2 cols,				//col_attr中有多少个列描述成员，也就是给果集的列数
		ub4 rows				//结果集有多少行
		);
	void FreeColAttrDataEx(		//用来释放结果集描述。注意，这个函数不会释放col_attr所指向的数组内存
		ColAttr_t* col_attr,	//结果集中的列描述指针，这个指针批向一个ColAttr_t结构的数组
		int cols				//col_attr中有多少个列描述成员，也就是给果集的列数
		);//这个函数不释放col_attr
	void FreeColAttrData(		//用来释放结果集描述。注意，这个函数跟上面的FreeColAttrDataEx区别在于它会释放col_attr所指向
		ColAttr_t* col_attr,	//结果集中的列描述指针，这个指针批向一个ColAttr_t结构的数组
		int cols				//col_attr中有多少个列描述成员，也就是给果集的列数
		);//这个函数会释放col_attr
	void FreeReadData(ColAttr_t* col_attr,int colnum,char *databuf); //col_attr 指列信息 databuf指具体的数据信息
	char*** ParseResultsForReadData (int rec_num, int attr_num, ColAttr *attrs, char *buf);
	int ParseResultsForReadData(int rec_num, int attr_num, ColAttr *attrs, char *buf, char *** &data_tab);
	void FreeSpaceForReadData(int row, char ***data);
	bool CloseConnectSocket();

	CDci();
	virtual ~CDci();

private:
	DCIEnv             *envhp;                       /* environment handle */
	DCIServer          *srvhp;                            /* server handle */
//	DCIError           *errhp;                             /* error handle */
	DCISvcCtx          *svchp;                          /* service context */
	DCISession         *authp;                   /* authentication context */
//	DCIParam           *colLstDesc;        /* column list parameter handle */
	DCIDirPathCtx      *dpctx;                      /* direct path context */
	DCIDirPathColArray *dpca;           /* direct path column array handle */
	DCIDirPathStream   *dpstr;                /* direct path stream handle */
//	DCIDescribe		   *dschp;                          /* describe handle */
    //DCIStmt 		   *stmthp;    20090603 xiemei for 单链接并发的执行语句
    char				username[128];
	char				pwd[128];
	char				servername[128];

	bool is_init_schema;
	bool is_connect;
	bool is_dm_database;

private:
	bool BindParamByRowBind(DCIStmt *stmtp_sql, char*data, DCIBind **ppbnd, ParamAttr_t *param, ub2 nums, DCIError *errhp, ErrorInfo_t *error);
	int SetCurveDataToDatabase(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, ErrorInfo &error);
	//注意了，下面这个SetSheetData函数在调用以后，需要调用ExecCommit来提交操作
	int MakeSql(Curveinfo *info, CurvePara &param, char** sqlpp, char** sql2pp, int &table_count, int &col_count, ErrorInfo &error);
	int MakeSheetSql(Curveinfo *info, CurvePara &param, const char *col_str, const char *where_str, const char* order_by, char** sqlpp, int &table_count, int &col_count, ErrorInfo &error);
	int MakeSheetTable(char *sql, int top_number, char** out_buf, vector<ColAttr_t> &attr, vector<ColDataItem> &data, ErrorInfo &error);
	int MakeSheetTable(Curveinfo *info, CurvePara &param, vector<ColAttr_t> &attr, vector<ColDataItem> &data, vector<string> &colname, const char* where_str, const char* order_by, ErrorInfo &error);
	int SetSheetData(char *model_name, time_t record_time, const SheetColData *indexp, ub2 ixsize, const SheetColData *valuep, ub2 vesize, ErrorInfo &error);
	int MakeTable(Curveinfo *info, CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error);
	int MakeSheetTable(Curveinfo *info, CurvePara &param, char *col_str,char *where_str,char **datapp, ColAttr_t **attrpp,int &items, ErrorInfo &error);
	bool CreateTableEx(const char* schema, const char *table, ColAttr_t *col, ub2 cols, char**ppsql, ErrorInfo_t *error);
	bool GetDataTypeName(ub2 dtype, ub4 data_size, char* type_name, ErrorInfo_t *error);
	bool SetTableInfo(const char *schema, const char *table, ColAttr_t *col, ub2 cols, ub4 rows, ub4 *nrows, ub1 input_type,
						DCIDirPathCtx      **dpctxpp, DCIDirPathColArray **dpcapp,
						DCIDirPathStream   **dpstrpp, DCIParam           **colLstDescpp, DCIError *errhp, ErrorInfo_t *error);
	void InitColOffset(ColAttr_t* col, ub2 cols, ub4 *record_size);
	void CleanError(ErrorInfo_t* error);
	void SetError(DCIError *errhp, ErrorInfo_t* error, int error_no, const char* error_info, ub4 htype, const char* file_name, int file_line);
	void ReplaceTopSql(const char* query, char **sql, int top_number);
	int SqltLen(int sqlt, int len);
	int CheckMalloc(void *ptr);
	int GetRows(IN const char *query, OUT int *rec_num,ErrorInfo_t* error);

protected:
	bool InitializeCurveInfo();
	sword StmtExecute(DCISvcCtx *svchp, DCIStmt *stmthp, DCIError *errhp, ub4 iters, ub4 commit_flag);
	void ConvertDatToTime_t(ColAttr *attr, ub2 cols, char *data, ub4 rows, ub4 reclen);
	void ConvertDciType(ub2 &dtype, ub2 &col_width, ub2 dprecision, ub1 dscale);
	const SheetColData* FindSheetColData(const char* col_name, const SheetColData *sheet_col_datap, ub2 col_size);
	int MakeSheetUpdateSql(Curveinfo *info_sheet, const SheetColData *sheet_col_datap, ub2 col_value_size,  ub2 col_size, int is_insert, time_t record_time, char *sql, ErrorInfo &error);
	int BindSheetCol(Curveinfo *info_sheet, DCIStmt *stmthp, DCIError *errhp, DCIBind **ppbnd, const SheetColData *sheet_col_datap, ub2 col_size, ErrorInfo &error);
	void InitSchemaTableColAttr();
	int MakeTableEx(Curveinfo *info, CurvePara &param, CurveData **datapp, int &items, ErrorInfo &error);
	int MakeInsertSql(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, char **insertsql, ErrorInfo &error);
	int MakeUpdateSql(Curveinfo *info, CurvePara &param, CurveData *curvedatap, ub2 items, char ** updatesql, char* out_table_name, char **updatesql2, char* out_table_name2, ErrorInfo &error);
	void InitHandle();
	bool BindParam(DCIStmt *stmtp_sql, DCIBind **ppbnd, ParamAttr_t *param, ub2 nums, DCIError* errhp, ErrorInfo_t *error);
	bool MergerProcedureResult(void **ppresult, void **ppind, ColAttr_t *col_attr, ub2 cols, ub4 copy_rows, ub4 last_rows, ErrorInfo_t *error);
	bool BindProcedureResult(DCIStmt *stmtp, DCIDefine **ppdefn, void **ppresult, ColAttr_t *col_attr, ub2 cols, ErrorInfo_t *error);
	bool FreeResultMemory(void **ppresult, void **ppind, ub2 cols);
	bool GetResultColAttr(DCIStmt *stmtp, ColAttr_t *col_attr,ub2 cols, DCIError* errhp, ErrorInfo_t *error);
	bool BindResult(DCIStmt *stmtp_result, DCIDefine **ppdefn, void **ppresult, void **ppind, ColAttr_t *col_attr, ub2 cols, DCIError *errhp, ErrorInfo_t *error);
	void CreateProcedureSql(char *sql,const char *package_name,const char *procedure_name, ColAttr_t *in_param, ub2 in_param_nums);
	bool GetStmtResult(DCIStmt *stmtp_result, ResultAttr_t *presult, DCIError* errhp, ErrorInfo_t *error);
	bool FreeDirPathHandle(
		DCIParam           *colLstDesc,        /* column list parameter handle */
		DCIDirPathCtx      *dpctx,                      /* direct path context */
		DCIDirPathColArray *dpca,           /* direct path column array handle */
		DCIDirPathStream   *dpstr                /* direct path stream handle */
	);
	bool AllocResultMemory(void **ppresult, void **ppind, ColAttr_t *col_attr, ub2 cols, ErrorInfo_t *error);
};

#define dcisg_malloc(size)		dcisg_malloc_ex(size, __LINE__, __FILE__)
void *dcisg_malloc_ex(size_t size, int line, const char* file_name);
void dcisg_free(void *data);
void show_memory();

void aaaaa();
int total_month_end_day(int year, int month, int months);
int month_end_day(int year, int month);
int year_end_day(int year);
int GetColName(Curveinfo *info, char **col_name, int &count, int row_time_span, int time_pace, tm *tm_time, bool is_update);
int GetTableName(const char* table_name_ft, char **data_time, char **table_name, int &count, tm *tm_start, tm *tm_end);
void StrcatSql(char **sqlpp, const char* sql_srcp);
int GetTableType(const char* table_name_ft);
void convert_dat_to_time(char* date_ptr,time_t *timep);
#endif // !defined(AFX_DCI_H__4343FFBB_7E79_49D0_B4FA_0DAB5B1C7A81__INCLUDED_)

