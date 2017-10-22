#ifndef _DCIDEFINE_H
#define _DCIDEFINE_H
#include <time.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
using namespace std;

#include "dci.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef ub1

typedef unsigned char  ub1;                   
typedef   signed char  sb1;                  

typedef unsigned short    ub2;                
typedef   signed short    sb2;               

typedef unsigned int  ub4;                   
typedef   signed int  sb4;                   

#ifdef WIN32
 typedef unsigned _int64 oraub8;
 typedef signed _int64 orasb8;
#else
 typedef unsigned long long oraub8;
 typedef signed long long orasb8;
#endif

typedef oraub8 ub8;
typedef orasb8 sb8;

#endif //ub1

//DCI自定义的数据类型，跟OCI的类型是兼容的
/* input data types */
#define DCI_CHR  1                        /* (ORANET TYPE) character string */
#define DCI_NUM  2                          /* (ORANET TYPE) oracle numeric */
#define DCI_INT  3                                 /* (ORANET TYPE) integer */
#define DCI_FLT  4                   /* (ORANET TYPE) Floating point number */
#define DCI_STR  5                                /* zero terminated string */
#define DCI_VNU  6                        /* NUM with preceding length byte */
#define DCI_PDN  7                  /* (ORANET TYPE) Packed Decimal Numeric */
#define DCI_LNG  8                                                  /* long */
#define DCI_VCS  9                             /* Variable character string */
#define DCI_NON  10                      /* Null/empty PCC Descriptor entry */
#define DCI_RID  11                                                /* rowid */
#define DCI_DAT  12                                /* date in oracle format */
#define DCI_VBI  15                                 /* binary in VCS format */
#define DCI_BFLOAT 21                                /* Native Binary float*/
#define DCI_BDOUBLE 22                             /* NAtive binary double */
#define DCI_BIN  23                                  /* binary data(DTYBIN) */
#define DCI_LBI  24                                          /* long binary */
#define DCI_UIN  68                                     /* unsigned integer */
#define DCI_SLS  91                        /* Display sign leading separate */
#define DCI_LVC  94                                  /* Longer longs (char) */
#define DCI_LVB  95                                   /* Longer long binary */
#define DCI_AFC  96                                      /* Ansi fixed char */
#define DCI_AVC  97                                        /* Ansi Var char */
#define DCI_IBFLOAT  100                           /* binary float canonical */
#define DCI_IBDOUBLE 101                          /* binary double canonical */
#define DCI_CUR  102                                        /* cursor  type */
#define DCI_RDD  104                                    /* rowid descriptor */
#define DCI_LAB  105                                          /* label type */
#define DCI_OSL  106                                        /* oslabel type */
#define DCI_NTY  108                                   /* named object type */
#define DCI_REF  110                                            /* ref type */
#define DCI_CLOB 112                                       /* character lob */
#define DCI_BLOB 113                                          /* binary lob */
#define DCI_BFILEE 114                                    /* binary file lob */
#define DCI_CFILEE 115                                 /* character file lob */
#define DCI_RSET 116                                     /* result set type */
#define DCI_NCO  122      /* named collection type (varray or nested table) */
#define DCI_VST  155                                      /* OCIString type */
#define DCI_ODT  156                                        /* OCIDate type */

/* datetimes and intervals */
#define DCI_DATE                      184                      /* ANSI Date */
#define DCI_TIME                      185                           /* TIME */
#define DCI_TIME_TZ                   186            /* TIME WITH TIME ZONE */
#define DCI_TIMESTAMP                 187                      /* TIMESTAMP */
#define DCI_TIMESTAMP_TZ              188       /* TIMESTAMP WITH TIME ZONE */
#define DCI_INTERVAL_YM               189         /* INTERVAL YEAR TO MONTH */
#define DCI_INTERVAL_DS               190         /* INTERVAL DAY TO SECOND */
#define DCI_TIMESTAMP_LTZ             232        /* TIMESTAMP WITH LOCAL TZ */


#define NORMAL_ERR			-1


#define	ERROR_INFO_LEN		1023		//最大的错误消息长度
#define INPUT_TEXT     0x01				//DirPathLoad方法中，绑冲区中的数据是字符串类型
#define INPUT_STREAM   0x02				//DirPathLoad方法中，绑冲区中的数据是二进制类型
#define MAX_NAME_LEN  128				//最大的对像名长度
#define MAX_FETCH_ROWS  10				//在获取语句或存贮过程返回的结果集时，一次性从服务器提取的行数
typedef struct ErrorInfo{
	int		error_no;
	char	error_info[ERROR_INFO_LEN + 1];
	int		file_line;
	char	file_name[300];
}ErrorInfo_t;

typedef struct ColAttr{
	char	col_name[MAX_NAME_LEN + 1];
	ub2		data_type;			//DCI的数据类型
	ub4		data_size;			//数据的大小，以字节计算
	ub4		data_offset;		//注意，这个参数是留给DCI内部自己使用的
	ub2		precision;			//数据类型的精度
	ub1		scale;				//数据的刻度
	void	*data;				//注意，如果在使用行绑定的方式调用DirPathLoadStream，那么这个参数是没有作用
								//只有在以列绑定的形式调用DirPathLoadStreamByColumnBinds时，它才是指向该列的数据指针
	sb2		*indp;				//这个参数可以为NULL，表示该列的数据大小没有空值，如果要指定该列的某一行为NULL，那应该把数组的该行值指定为-1
								//在从DCI中获取结果集时，你需要判断该成员数组对应的位置值是否是-1，如果是，则表示该列在该行的值为NULL
}ColAttr_t;

typedef struct SimpleColAttr{
	char	col_name[128];
	ub4		data_type;			//DCI的数据类型
	ub4		data_size;			//数据的大小，以字节计算
}SimpleColAttr_t;

typedef ColAttr_t ParamAttr_t;

typedef struct ResultAttr{
	ColAttr_t *col_attr;
	ub2		cols;
	ub4		rows;
}ResultAttr_t;

#define		SQL_MALLOC_LEN			4096
#define		OBJECT_NAME_LEN			128
#define		X_VALUES_LEN			128
#define MAX_DEV_KEY_ID_LEN		24
struct CurveData
{
	int		is_null;			//是否是空值
	int		state;				//采样设备在该时间的状态位，如果无状态位，该成员值是无效的
	union
	{
		float f_value;			//采样值
		ub4 i_value;			//采样值,这种类型有采样值只支持FIRST_VALUE, LAST_VALUE取数
	};	
	union
	{
		time_t x_time;		//采样的时间
		float	x_value;	
	};	
};

#define MAX_VALUE		1	//步长中的最大值
#define MIN_VALUE		2	//步长中的最小值
#define AVG_VALUE		3	//步长中的平均值
#define FIRST_VALUE		4	//步长中的第一个值
#define LAST_VALUE		5	//步长中的最后一个值
struct CurvePara {
	char		conf_id[OBJECT_NAME_LEN];			//采样表定义的配置号码
	time_t		starttime;							//要求取得采样的起始时间
	time_t		stoptime;							//要求取得采样的终止时间
	char		dev_key_id_values[OBJECT_NAME_LEN];	//指定要取的采样设备ID，该ID的字符串长度不能超过127字节
	int			needpace;							//采样的时间步长，如果指定的是0，那么按配置文件中配置的步长为默认值
	int			data_format;						//如果采样步长比数据库中实际存贮的步长要大，
	void*		temp_memory1;						//按调用GetSheetData方法时临时内存存放地址，外部函数在调用GetSheetData结束以后，需要释放这段内存
	void*		temp_memory2;						//按调用GetSheetData方法时临时内存存放地址，外部函数在调用GetSheetData结束以后，需要释放这段内存
	char		resververd[32];	//保留空间，当结构发生改变时，可以从中抽出一部分它用，达到保持结构体大小不变，以保证免以前的程序可以兼容		
	//那么该变量指明按何种方式来取得该步长包含的实际存贮步长的值。目前仅支持MAX_VALUE, MIN_VALUE, AVG_VALUE, FIRST_VALUE, LAST_VALUE 		
};
/***********************
struct SheetPara {
    char        conf_id[OBJECT_NAME_LEN];           //采样表定义的配置号码
    time_t      starttime;                          //要求取得采样的起始时间
    time_t      stoptime;                           //要求取得采样的终止时间
    char        dev_key_id_values[OBJECT_NAME_LEN]; //指定要取的采样设备ID，该ID的字符串长度不能超过127字节
    int         needpace;                           //采样的时间步长，如果指定的是0，那么按配置文件中配置的步长为默认值
    int         data_format;                        //如果采样步长比数据库中实际存贮的步长要大，
    //那么该变量指明按何种方式来取得该步长包含的实际存贮步长的值。目前仅支持MAX_VALUE, MIN_VALUE, AVG_VALUE, FIRST_VALUE, LAST_VALUE 
};
**************************/



#define		MAX_DEV_KEY_ID_LIST_LEN		2048

#define CURVE_ID				"APP_ID"				//科东应用号，这个配置在midhs中取曲线时要用到
#define CONFIG_ID				"CONFIG_ID"				//采样表定义配置号
#define	TABLE_NAME_FT			"TABLE_NAME_FT"			//
#define TABLE_NAME_MODEL		"TABLE_NAME_MODEL"
#define	STATE_TABLE_NAME_FT		"STATE_TABLE_NAME_FT"
#define	COL_NAME_FT				"COL_NAME_FT"
#define ADDITION_COL_NAME_FT	"ADDITION_COL_NAME_FT"
#define DEV_KEY_COL_NAME		"DEV_KEY_COL_NAME"
#define	TABLE_TYPE				"TABLE_TYPE"
#define TABLES_BETWEEN_MONTHS	"TABLES_BETWEEN_MONTHS"
#define DATE_COL_NAME			"DATE_COL_NAME"
#define MINUTE_COL_NAME			"MINUTE_COL_NAME"
#define HOUR_COL_NAME			"HOUR_COL_NAME"
#define DAY_COL_NAME			"DAY_COL_NAME"
#define MONTH_COL_NAME			"MONTH_COL_NAME"
#define YEAR_COL_NAME			"YEAR_COL_NAME"
#define ROW_TIME_SPAN			"ROW_TIME_SPAN"
#define TIME_PACE				"TIME_PACE"
#define BASE_COL_INDEX			"BASE_COL_INDEX"
#define WHERE_NOR				"WHERE_NOR"
#define INSERT_EXP				"INSERT_EXP"
#define CONF_FILE_NAME_CURVE	"curve.conf"
#define INDEX_STR				"index_str"
//#define CONF_FILE_NAME_SHEET	"sheet.conf"
#define MAX_CONF_VALUE_LEN		2048
//#define SHEET_ID				"SHEET_ID"				//科东应用号
//#define APP_ID					"APP_ID"				//采样表定义配置号

#define		FUN_DATE_YEAR	  1   /* 'YEAR' , 'YYYY', 'YY' */
#define		FUN_DATE_MONTH    2   /* 'MONTH', 'MM', 'M'  */
#define		FUN_DATE_DAY      3   /* 'DAY'  , 'DD', 'D'  */ 
#define		FUN_DATE_HOUR     4   /* 'HOUR' , 'HH'   */
#define		FUN_DATE_MIN      5   /* 'MINUTE' , 'MI', 'N' */
#define		FUN_DATE_SEC      6   /* 'SECOND'  , 'SS', 'S'  */
#define     FUN_DATE_MSEC     7   /* 'MILLISECOND' , 'MS'   */
#define     FUN_DATE_QUARTER  8   /* 'QUARTER' , 'QQ', 'Q' */
#define     FUN_DATE_DAYOFY   9   /* 'DAYOFYEAR' , 'DY', 'Y' */
#define     FUN_DATE_WEEK     10  /* 'WEEK' , 'WK', 'WW'  */
#define		FUN_DATE_WEEKDAY  11  /* 'WEEKDAY' , 'DW'*/
#define     UPDATE_ZERO_ROWS  -20000 /* have updated zero row */
struct ColType
{
	sb2		col_type;
	ub2		col_len;	
};
struct Curveinfo{
	char	conf_id[OBJECT_NAME_LEN];			//配置项号
	char	curve_id[OBJECT_NAME_LEN];			//科东为应用号，该号可以跟MIDHIS服务器的数据库连接相对应
	char	table_name_ft[OBJECT_NAME_LEN];		//表名表达式
	char	table_name_model[OBJECT_NAME_LEN];		//模板表名，该名称和table_name_ft所指的表达式应该是相对应的
	char	state_table_name_ft[OBJECT_NAME_LEN];		//采样表对应的状态表名表达式
	char	column_name_ft[MAX_CONF_VALUE_LEN];			//列名表达式
	char	addition_column_name_ft[OBJECT_NAME_LEN];	//需要加在结果集中的附加列表达式,这个选项只对GetSheetData接口有效
	char	dev_key_col_name[OBJECT_NAME_LEN];	//设备KEYID所在的列名
	char	date_col_name[OBJECT_NAME_LEN];		//以小时为采样单位的表结构(TABLE_TYPE_COL_HOUR24表结构类型)，竖向日期列名
	char	hour_col_name[OBJECT_NAME_LEN];		//如果存在小时列，用来竖向指定某个小时某个设备的采样，那么这边存放该小时列的列名
	char	minute_col_name[OBJECT_NAME_LEN];	//如果存在分钟列，用来竖向指定某个分钟某个设备的采样，那么这边存放该分钟列的列名
	char	day_col_name[OBJECT_NAME_LEN];		//表示日期中日数的列名
	char	month_col_name[OBJECT_NAME_LEN];	//表示日期中月数的列名
	char	year_col_name[OBJECT_NAME_LEN];		//表示日期中年数的列名
	int		base_col_index;						//当以%d的格式来表达列名命名格式时，该值为格式的起始值
	int		row_time_span;						//单行跨度时间，单位秒
	int		time_pace;							//采样单位步长，单位秒
	struct  Curveinfo *next;					//指向下一个该结构体
	map<string, ColType> col_attr;
	char	where_nor[OBJECT_NAME_LEN];			//为上下限的曲线服务
	char	insert_exp[OBJECT_NAME_LEN];		//更新曲线在执行UPDATE时，如果未更新行，那么用这个表达式来做INSERT
	char	index_str[OBJECT_NAME_LEN];
};

struct SheetColData
{
	char	col_name[OBJECT_NAME_LEN + sizeof(DCIDate)];//后面多出的sizeof(DCIDate)大小是为了给程序内部转换time_t到DCIDate时用的
	union
	{
		sb1 sb1_value;
		sb2 sb2_value;
		sb4 sb4_value;
		sb8 sb8_value;
		float f_value;
		double d_value;
		time_t t_value;
		char   str_value[512];
	};
};
struct SheetRecData
{
	vector<SheetColData> index;
	vector<SheetColData> values;
	time_t rec_time;
};
#define  MAX_COL_DATA_LEN		300
struct BlobData
{
	int data_size;
	void *datap;
};
struct ColDataItem
{
	ub1 null_flag;
	ub1 col_type;
	ub2 col_len;
	union
	{
		sb1 sb1_value;		//对应数据类型DCI_INT，其中date_size值为1
		sb2 sb2_value;		//对应数据类型DCI_INT，其中date_size值为2
		sb4 sb4_value;		//对应数据类型DCI_INT，其中date_size值为4
		sb8 sb8_value;		//对应数据类型DCI_INT，其中date_size值为8
		float f_value;		//对应数据类型DCI_FLT，其中date_size值为4
		double d_value;		//对应数据类型DCI_FLT，其中date_size值为8
		time_t t_value;		//对应数据类型DCI_DAT
		const char* str_valuep;	//对应数据类型DCI_STR，字符串会以'\0'结尾
		BlobData blob_value;	//对应数据类型DCI_CLOB，DCI_BLOB
	};
};


#endif//_DCIDEFINE_H
