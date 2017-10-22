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

//DCI�Զ�����������ͣ���OCI�������Ǽ��ݵ�
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


#define	ERROR_INFO_LEN		1023		//���Ĵ�����Ϣ����
#define INPUT_TEXT     0x01				//DirPathLoad�����У�������е��������ַ�������
#define INPUT_STREAM   0x02				//DirPathLoad�����У�������е������Ƕ���������
#define MAX_NAME_LEN  128				//���Ķ���������
#define MAX_FETCH_ROWS  10				//�ڻ�ȡ����������̷��صĽ����ʱ��һ���Դӷ�������ȡ������
typedef struct ErrorInfo{
	int		error_no;
	char	error_info[ERROR_INFO_LEN + 1];
	int		file_line;
	char	file_name[300];
}ErrorInfo_t;

typedef struct ColAttr{
	char	col_name[MAX_NAME_LEN + 1];
	ub2		data_type;			//DCI����������
	ub4		data_size;			//���ݵĴ�С�����ֽڼ���
	ub4		data_offset;		//ע�⣬�������������DCI�ڲ��Լ�ʹ�õ�
	ub2		precision;			//�������͵ľ���
	ub1		scale;				//���ݵĿ̶�
	void	*data;				//ע�⣬�����ʹ���а󶨵ķ�ʽ����DirPathLoadStream����ô���������û������
								//ֻ�������а󶨵���ʽ����DirPathLoadStreamByColumnBindsʱ��������ָ����е�����ָ��
	sb2		*indp;				//�����������ΪNULL����ʾ���е����ݴ�Сû�п�ֵ�����Ҫָ�����е�ĳһ��ΪNULL����Ӧ�ð�����ĸ���ֵָ��Ϊ-1
								//�ڴ�DCI�л�ȡ�����ʱ������Ҫ�жϸó�Ա�����Ӧ��λ��ֵ�Ƿ���-1������ǣ����ʾ�����ڸ��е�ֵΪNULL
}ColAttr_t;

typedef struct SimpleColAttr{
	char	col_name[128];
	ub4		data_type;			//DCI����������
	ub4		data_size;			//���ݵĴ�С�����ֽڼ���
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
	int		is_null;			//�Ƿ��ǿ�ֵ
	int		state;				//�����豸�ڸ�ʱ���״̬λ�������״̬λ���ó�Աֵ����Ч��
	union
	{
		float f_value;			//����ֵ
		ub4 i_value;			//����ֵ,���������в���ֵֻ֧��FIRST_VALUE, LAST_VALUEȡ��
	};	
	union
	{
		time_t x_time;		//������ʱ��
		float	x_value;	
	};	
};

#define MAX_VALUE		1	//�����е����ֵ
#define MIN_VALUE		2	//�����е���Сֵ
#define AVG_VALUE		3	//�����е�ƽ��ֵ
#define FIRST_VALUE		4	//�����еĵ�һ��ֵ
#define LAST_VALUE		5	//�����е����һ��ֵ
struct CurvePara {
	char		conf_id[OBJECT_NAME_LEN];			//������������ú���
	time_t		starttime;							//Ҫ��ȡ�ò�������ʼʱ��
	time_t		stoptime;							//Ҫ��ȡ�ò�������ֹʱ��
	char		dev_key_id_values[OBJECT_NAME_LEN];	//ָ��Ҫȡ�Ĳ����豸ID����ID���ַ������Ȳ��ܳ���127�ֽ�
	int			needpace;							//������ʱ�䲽�������ָ������0����ô�������ļ������õĲ���ΪĬ��ֵ
	int			data_format;						//����������������ݿ���ʵ�ʴ����Ĳ���Ҫ��
	void*		temp_memory1;						//������GetSheetData����ʱ��ʱ�ڴ��ŵ�ַ���ⲿ�����ڵ���GetSheetData�����Ժ���Ҫ�ͷ�����ڴ�
	void*		temp_memory2;						//������GetSheetData����ʱ��ʱ�ڴ��ŵ�ַ���ⲿ�����ڵ���GetSheetData�����Ժ���Ҫ�ͷ�����ڴ�
	char		resververd[32];	//�����ռ䣬���ṹ�����ı�ʱ�����Դ��г��һ�������ã��ﵽ���ֽṹ���С���䣬�Ա�֤����ǰ�ĳ�����Լ���		
	//��ô�ñ���ָ�������ַ�ʽ��ȡ�øò���������ʵ�ʴ���������ֵ��Ŀǰ��֧��MAX_VALUE, MIN_VALUE, AVG_VALUE, FIRST_VALUE, LAST_VALUE 		
};
/***********************
struct SheetPara {
    char        conf_id[OBJECT_NAME_LEN];           //������������ú���
    time_t      starttime;                          //Ҫ��ȡ�ò�������ʼʱ��
    time_t      stoptime;                           //Ҫ��ȡ�ò�������ֹʱ��
    char        dev_key_id_values[OBJECT_NAME_LEN]; //ָ��Ҫȡ�Ĳ����豸ID����ID���ַ������Ȳ��ܳ���127�ֽ�
    int         needpace;                           //������ʱ�䲽�������ָ������0����ô�������ļ������õĲ���ΪĬ��ֵ
    int         data_format;                        //����������������ݿ���ʵ�ʴ����Ĳ���Ҫ��
    //��ô�ñ���ָ�������ַ�ʽ��ȡ�øò���������ʵ�ʴ���������ֵ��Ŀǰ��֧��MAX_VALUE, MIN_VALUE, AVG_VALUE, FIRST_VALUE, LAST_VALUE 
};
**************************/



#define		MAX_DEV_KEY_ID_LIST_LEN		2048

#define CURVE_ID				"APP_ID"				//�ƶ�Ӧ�úţ����������midhs��ȡ����ʱҪ�õ�
#define CONFIG_ID				"CONFIG_ID"				//�����������ú�
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
//#define SHEET_ID				"SHEET_ID"				//�ƶ�Ӧ�ú�
//#define APP_ID					"APP_ID"				//�����������ú�

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
	char	conf_id[OBJECT_NAME_LEN];			//�������
	char	curve_id[OBJECT_NAME_LEN];			//�ƶ�ΪӦ�úţ��úſ��Ը�MIDHIS�����������ݿ��������Ӧ
	char	table_name_ft[OBJECT_NAME_LEN];		//�������ʽ
	char	table_name_model[OBJECT_NAME_LEN];		//ģ������������ƺ�table_name_ft��ָ�ı��ʽӦ�������Ӧ��
	char	state_table_name_ft[OBJECT_NAME_LEN];		//�������Ӧ��״̬�������ʽ
	char	column_name_ft[MAX_CONF_VALUE_LEN];			//�������ʽ
	char	addition_column_name_ft[OBJECT_NAME_LEN];	//��Ҫ���ڽ�����еĸ����б��ʽ,���ѡ��ֻ��GetSheetData�ӿ���Ч
	char	dev_key_col_name[OBJECT_NAME_LEN];	//�豸KEYID���ڵ�����
	char	date_col_name[OBJECT_NAME_LEN];		//��СʱΪ������λ�ı�ṹ(TABLE_TYPE_COL_HOUR24��ṹ����)��������������
	char	hour_col_name[OBJECT_NAME_LEN];		//�������Сʱ�У���������ָ��ĳ��Сʱĳ���豸�Ĳ�������ô��ߴ�Ÿ�Сʱ�е�����
	char	minute_col_name[OBJECT_NAME_LEN];	//������ڷ����У���������ָ��ĳ������ĳ���豸�Ĳ�������ô��ߴ�Ÿ÷����е�����
	char	day_col_name[OBJECT_NAME_LEN];		//��ʾ����������������
	char	month_col_name[OBJECT_NAME_LEN];	//��ʾ����������������
	char	year_col_name[OBJECT_NAME_LEN];		//��ʾ����������������
	int		base_col_index;						//����%d�ĸ�ʽ���������������ʽʱ����ֵΪ��ʽ����ʼֵ
	int		row_time_span;						//���п��ʱ�䣬��λ��
	int		time_pace;							//������λ��������λ��
	struct  Curveinfo *next;					//ָ����һ���ýṹ��
	map<string, ColType> col_attr;
	char	where_nor[OBJECT_NAME_LEN];			//Ϊ�����޵����߷���
	char	insert_exp[OBJECT_NAME_LEN];		//����������ִ��UPDATEʱ�����δ�����У���ô��������ʽ����INSERT
	char	index_str[OBJECT_NAME_LEN];
};

struct SheetColData
{
	char	col_name[OBJECT_NAME_LEN + sizeof(DCIDate)];//��������sizeof(DCIDate)��С��Ϊ�˸������ڲ�ת��time_t��DCIDateʱ�õ�
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
		sb1 sb1_value;		//��Ӧ��������DCI_INT������date_sizeֵΪ1
		sb2 sb2_value;		//��Ӧ��������DCI_INT������date_sizeֵΪ2
		sb4 sb4_value;		//��Ӧ��������DCI_INT������date_sizeֵΪ4
		sb8 sb8_value;		//��Ӧ��������DCI_INT������date_sizeֵΪ8
		float f_value;		//��Ӧ��������DCI_FLT������date_sizeֵΪ4
		double d_value;		//��Ӧ��������DCI_FLT������date_sizeֵΪ8
		time_t t_value;		//��Ӧ��������DCI_DAT
		const char* str_valuep;	//��Ӧ��������DCI_STR���ַ�������'\0'��β
		BlobData blob_value;	//��Ӧ��������DCI_CLOB��DCI_BLOB
	};
};


#endif//_DCIDEFINE_H
