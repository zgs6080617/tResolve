#ifndef _DCI_H
#define _DCI_H

#ifdef WIN32
#define DCI_API   __declspec( dllexport )
#else
#define DCI_API 
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif

//单字节整数
#ifndef lint 
typedef unsigned char  ub1;                   
typedef   signed char  sb1;                  
#else 
#define ub1 unsigned char 
#define sb1 signed char 
#endif 

#define UB1MAXVAL ((ub1)UCHAR_MAX) 
#define UB1MINVAL ((ub1)        0) 
#define SB1MAXVAL ((sb1)SCHAR_MAX) 
#define SB1MINVAL ((sb1)SCHAR_MIN) 
#define MINUB1MAXVAL ((ub1)  255) 
#define MAXUB1MINVAL ((ub1)    0) 
#define MINSB1MAXVAL ((sb1)  127) 
#define MAXSB1MINVAL ((sb1) -127) 

//双字节整数
#ifndef lint 
typedef unsigned short    ub2;                
typedef   signed short    sb2;               
#else 
#define ub2  unsigned short 
#define sb2  signed short 
#endif

#define UB2MAXVAL ((ub2)USHRT_MAX) 
#define UB2MINVAL ((ub2)        0) 
#define SB2MAXVAL ((sb2) SHRT_MAX) 
#define SB2MINVAL ((sb2) SHRT_MIN) 
#define MINUB2MAXVAL ((ub2) 65535) 
#define MAXUB2MINVAL ((ub2)     0) 
#define MINSB2MAXVAL ((sb2) 32767) 
#define MAXSB2MINVAL ((sb2)-32767) 

//四字节整数
#ifndef lint 
typedef unsigned int  ub4;                   
typedef   signed int  sb4;                   
#else 
#define eb4 int 
#define ub4 unsigned int 
#define sb4 signed int 
#endif 
 
#define UB4MAXVAL ((ub4)UINT_MAX) 
#define UB4MINVAL ((ub4)        0) 
#define SB4MAXVAL ((sb4) INT_MAX) 
#define SB4MINVAL ((sb4) INT_MIN) 
#define MINUB4MAXVAL ((ub4) 4294967295) 
#define MAXUB4MINVAL ((ub4)          0) 
#define MINSB4MAXVAL ((sb4) 2147483647) 
#define MAXSB4MINVAL ((sb4)-2147483647) 

//八字节整数
#ifdef WIN32
#ifndef lint
typedef unsigned __int64 ub8;     
typedef   signed __int64 sb8;     
#else
#define ub8 unsigned __int64
#define sb8 signed __int64
#endif 
#else
#ifndef lint
typedef unsigned long long  ub8;             
typedef   signed long long  sb8;             
#else
#define ub8 unsigned long long
#define sb8 signed long long
#endif 
#endif                                                             /* _WIN64 */

#define UB8MINVAL    ((ub8)0)
#define UB8MAXVAL    ((ub8)18446744073709551615)
#define SB8MINVAL    ((sb8)-9223372036854775808)
#define SB8MAXVAL    ((sb8) 9223372036854775807)

#define MAXUB8MINVAL ((ub8)0)
#define MINUB8MAXVAL ((ub8)18446744073709551615)
#define MAXSB8MINVAL ((sb8)-9223372036854775807)
#define MINSB8MAXVAL ((sb8) 9223372036854775807)


#define UB1BITS          CHAR_BIT
#define UB1MASK          ((1 << ((uword)CHAR_BIT)) - 1)


#ifdef lint
#define dcitext unsigned char
#else
  typedef  unsigned char dcitext;
#endif

#define DciText dcitext


#ifndef lint 
typedef          char     eb1;
typedef          short    eb2;               
typedef          int      eb4;               
#else
# define         eb1      char
# define         eb2      short
# define         eb4      int
#endif

#define EB1MAXVAL      ((eb1)SCHAR_MAX) 
#define EB1MINVAL      ((eb1)        0) 
#define MINEB1MAXVAL   ((eb1)  127) 
#define MAXEB1MINVAL   ((eb1)    0) 
#define EB2MAXVAL      ((eb2) SHRT_MAX) 
#define EB2MINVAL      ((eb2)        0) 
#define MINEB2MAXVAL   ((eb2) 32767) 
#define MAXEB2MINVAL   ((eb2)     0) 
#define EB4MAXVAL      ((eb4) INT_MAX) 
#define EB4MINVAL      ((eb4)        0) 
#define MINEB4MAXVAL   ((eb4) 2147483647) 
#define MAXEB4MINVAL   ((eb4)          0) 


#ifndef lint 
typedef         sb1  b1;                   
#else 
#define         b1 sb1 
#endif  
#define  B1MAXVAL  SB1MAXVAL 
#define  B1MINVAL  SB1MINVAL 
 
#ifndef lint 
typedef         sb2      b2;              
#else 
#define         b2 sb2 
#endif  
#define  B2MAXVAL  SB2MAXVAL 
#define  B2MINVAL  SB2MINVAL 
 
#ifndef lint 
typedef         sb4    b4;                
#else 
#define         b4 sb4 
#endif  
# define  B4MAXVAL  SB4MAXVAL 
# define  B4MINVAL  SB4MINVAL 

#ifndef lint
typedef          int eword;                  
typedef unsigned int uword;                  
typedef   signed int sword;                  
#else
#define eword int
#define uword unsigned int
#define sword signed int
#endif 

#define  EWORDMAXVAL  ((eword) INT_MAX)
#define  EWORDMINVAL  ((eword)       0)
#define  UWORDMAXVAL  ((uword)UINT_MAX)
#define  UWORDMINVAL  ((uword)       0)
#define  SWORDMAXVAL  ((sword) INT_MAX)
#define  SWORDMINVAL  ((sword) INT_MIN)
#define  MINEWORDMAXVAL  ((eword)  2147483647)
#define  MAXEWORDMINVAL  ((eword)      0)
#define  MINUWORDMAXVAL  ((uword)  4294967295)
#define  MAXUWORDMINVAL  ((uword)           0)
#define  MINSWORDMAXVAL  ((sword)  2147483647)
#define  MAXSWORDMINVAL  ((sword) -2147483647)

//<SMR-LW-2011052501> ===> BEGIN SMR-LW-2011052501
//<SMR-LW-2011042201> ===> BEGIN SMR-LW-2011042201
#if !defined(SOLARIS) && !defined(HP_UX) && !defined(_AIX)
//<SMR-LW-2011042201> ===> END
//<SMR-LW-2011052501> ===> END
#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif
#endif

#undef CONST
#define CONST const

#define dvoid void

typedef void (*lgenfp_t)( void );


#define SQLT_CHR  1                        /* (ORANET TYPE) character string */
#define SQLT_NUM  2                          /* (ORANET TYPE) oracle numeric */
#define SQLT_INT  3                                 /* (ORANET TYPE) integer */
#define SQLT_FLT  4                   /* (ORANET TYPE) Floating point number */
#define SQLT_STR  5                                /* zero terminated string */
#define SQLT_VNU  6                        /* NUM with preceding length byte */
#define SQLT_PDN  7                  /* (ORANET TYPE) Packed Decimal Numeric */
#define SQLT_LNG  8                                                  /* long */
#define SQLT_VCS  9                             /* Variable character string */
#define SQLT_NON  10                      /* Null/empty PCC Descriptor entry */
#define SQLT_RID  11                                                /* rowid */
#define SQLT_DAT  12                                /* date in oracle format */
#define SQLT_VBI  15                                 /* binary in VCS format */
#define SQLT_BIN  23                                  /* binary data(DTYBIN) */
#define SQLT_LBI  24                                          /* long binary */
#define SQLT_UIN  68                                     /* unsigned integer */
#define SQLT_SLS  91                        /* Display sign leading separate */
#define SQLT_LVC  94                                  /* Longer longs (char) */
#define SQLT_LVB  95                                  /* Longer long binary */ 
#define SQLT_AFC  96                                      /* Ansi fixed char */
#define SQLT_AVC  97                                        /* Ansi Var char */
#define SQLT_CUR  102                                        /* cursor  type */
#define SQLT_RDD  104                                    /* rowid descriptor */
#define SQLT_LAB  105                                          /* label type */
#define SQLT_OSL  106                                        /* oslabel type */

#define SQLT_NTY  108                                   /* named object type */
#define SQLT_REF  110                                            /* ref type */
#define SQLT_CLOB 112                                       /* character lob */
#define SQLT_BLOB 113                                          /* binary lob */
#define SQLT_BFILEE 114                                    /* binary file lob */
#define SQLT_CFILEE 115                                 /* character file lob */
#define SQLT_RSET 116                                     /* result set type */
#define SQLT_NCO  122      /* named collection type (varray or nested table) */
#define SQLT_VST  155                                      /* OCIString type */
#define SQLT_ODT  156                                        /* OCIDate type */

/* datetimes and intervals */
#define SQLT_DATE                      184                      /* ANSI Date */
#define SQLT_TIME                      185                           /* TIME */
#define SQLT_TIME_TZ                   186            /* TIME WITH TIME ZONE */
#define SQLT_TIMESTAMP                 187                      /* TIMESTAMP */
#define SQLT_TIMESTAMP_TZ              188       /* TIMESTAMP WITH TIME ZONE */
#define SQLT_INTERVAL_YM               189         /* INTERVAL YEAR TO MONTH */
#define SQLT_INTERVAL_DS               190         /* INTERVAL DAY TO SECOND */
#define SQLT_TIMESTAMP_LTZ             232        /* TIMESTAMP WITH LOCAL TZ */

#define SQLT_PNTY   241              /* pl/sql representation of named types */
#define SQLCS_IMPLICIT 1     /* for CHAR, VARCHAR2, CLOB w/o a specified set */
/*--------------------------------------------------------------------------- 
                     PUBLIC TYPES AND CONSTANTS 
  ---------------------------------------------------------------------------*/
#define DCI_OBJECT          0x00000002  /* application in object environment */
/*-----------------------------Handle Types----------------------------------*/
#define DCI_HTYPE_ENV            1                     /* environment handle */
#define DCI_HTYPE_ERROR          2                           /* error handle */
#define DCI_HTYPE_SVCCTX         3                         /* service handle */
#define DCI_HTYPE_STMT           4                       /* statement handle */
#define DCI_HTYPE_BIND           5                            /* bind handle */
#define DCI_HTYPE_DEFINE         6                          /* define handle */
#define DCI_HTYPE_DESCRIBE       7                        /* describe handle */
#define DCI_HTYPE_SERVER         8                          /* server handle */
#define DCI_HTYPE_SESSION        9                  /* authentication handle */
#define DCI_HTYPE_AUTHINFO      DCI_HTYPE_SESSION  /* SessionGet auth handle */
#define DCI_HTYPE_DIRPATH_CTX   14                    /* direct path context */
#define DCI_HTYPE_DIRPATH_COLUMN_ARRAY 15        /* direct path column array */
#define DCI_HTYPE_DIRPATH_STREAM       16              /* direct path stream */
#define DCI_HTYPE_PROC                 17                  /* process handle */
/*---------------------------------------------------------------------------*/
#define DCI_NTV_SYNTAX 1    /* 为了兼容以前的程序，这个属性无作用 */
//<SQLT_INTERVAL_DS> ===> BEGIN SMR-SF-10101101
/*-------------------------Descriptor Types----------------------------------*/
/* descriptor values range from 50 - 255 */
#define DCI_DTYPE_FIRST 50                 /* start value of descriptor type */
#define DCI_DTYPE_LOB 50                                     /* lob  locator */
#define DCI_DTYPE_SNAP 51                             /* snapshot descriptor */
#define DCI_DTYPE_RSET 52                           /* result set descriptor */
#define DCI_DTYPE_PARAM 53  /* a parameter descriptor obtained from ocigparm */
#define DCI_DTYPE_ROWID  54                              /* rowid descriptor */
#define DCI_DTYPE_COMPLEXOBJECTCOMP  55
/* complex object retrieval descriptor */
#define DCI_DTYPE_FILE 56                                /* File Lob locator */
#define DCI_DTYPE_AQENQ_OPTIONS 57                        /* enqueue options */
#define DCI_DTYPE_AQDEQ_OPTIONS 58                        /* dequeue options */
#define DCI_DTYPE_AQMSG_PROPERTIES 59                  /* message properties */
#define DCI_DTYPE_AQAGENT 60                                     /* aq agent */
#define DCI_DTYPE_LOCATOR 61                                  /* LOB locator */
#define DCI_DTYPE_INTERVAL_YM 62                      /* Interval year month */
#define DCI_DTYPE_INTERVAL_DS 63                      /* Interval day second */
#define DCI_DTYPE_AQNFY_DESCRIPTOR  64               /* AQ notify descriptor */
#define DCI_DTYPE_DATE 65                            /* Date */
#define DCI_DTYPE_TIME 66                            /* Time */
#define DCI_DTYPE_TIME_TZ 67                         /* Time with timezone */
#define DCI_DTYPE_TIMESTAMP 68                       /* Timestamp */
#define DCI_DTYPE_TIMESTAMP_TZ 69                /* Timestamp with timezone */
#define DCI_DTYPE_TIMESTAMP_LTZ 70             /* Timestamp with local tz */
#define DCI_DTYPE_UCB           71               /* user callback descriptor */
#define DCI_DTYPE_SRVDN         72              /* server DN list descriptor */
#define DCI_DTYPE_SIGNATURE     73                              /* signature */
#define DCI_DTYPE_RESERVED_1    74              /* reserved for internal use */
#define DCI_DTYPE_LAST          74        /* last value of a descriptor type */
//<SQLT_INTERVAL_DS> ===> END
/*---------------------------------------------------------------------------*/
/*-------------------------Credential Types----------------------------------*/
#define DCI_CRED_RDBMS      1                  /* database username/password */
/*---------------------------------------------------------------------------*/
/*----------------------------Piece Definitions------------------------------*/

#define DCI_ONE_PIECE 0                                         /* one piece */

/*---------------------------------------------------------------------------*/
/*------------------------Bind and Define Options----------------------------*/
#define DCI_SB2_IND_PTR       0x00000001                           /* unused */
#define DCI_DATA_AT_EXEC      0x00000002             /* data at execute time */
#define DCI_DYNAMIC_FETCH     0x00000002                /* fetch dynamically */
#define DCI_PIECEWISE         0x00000004          /* piecewise DMLs or fetch */
#define DCI_DEFINE_RESERVED_1 0x00000008                         /* reserved */
#define DCI_BIND_RESERVED_2   0x00000010                         /* reserved */
#define DCI_DEFINE_RESERVED_2 0x00000020                         /* reserved */
#define DCI_BIND_SOFT         0x00000040              /* soft bind or define */
#define DCI_DEFINE_SOFT       0x00000080              /* soft bind or define */
/*---------------------------------------------------------------------------*/
#ifndef DCI_FLAGS
#define DCI_FLAGS
#define DCI_ONE_PIECE 0                                         /* one piece */
#define DCI_FIRST_PIECE 1                                 /* the first piece */
#define DCI_NEXT_PIECE 2                          /* the next of many pieces */
#define DCI_LAST_PIECE 3                                   /* the last piece */
#endif
//<SMR-YYL-2011062701> ===> BEGIN
/*--------------------------- FILE open modes -------------------------------*/
#define DCI_FILE_READONLY 1             /* readonly mode open for FILE types */
/*---------------------------------------------------------------------------*/
//<SMR-YYL-2011062701> ===> END
/*=============================Attribute Types===============================*/
/* 
   Note: All attributes are global.  New attibutes should be added to the end
   of the list. Before you add an attribute see if an existing one can be 
   used for your handle. 

   If you see any holes please use the holes first. 
 
*/
/*===========================================================================*/
#define DCI_ATTR_ENV  5                            /* the environment handle */
#define DCI_ATTR_SERVER 6                               /* the server handle */
#define DCI_ATTR_SESSION 7                        /* the user session handle */
#define DCI_ATTR_ROW_COUNT   9                  /* the rows processed so far */
#define DCI_ATTR_PREFETCH_ROWS  11    /* sets the number of rows to prefetch */
#define DCI_ATTR_PARAM_COUNT 18       /* number of column in the select list */
#define DCI_ATTR_USERNAME 22                           /* username attribute */
#define DCI_ATTR_PASSWORD 23                           /* password attribute */
#define DCI_ATTR_STMT_TYPE   24                            /* statement type */

#define DCI_ATTR_DATEFORMAT             75     /* default date format string */
#define DCI_ATTR_BUF_ADDR               76                 /* buffer address */
#define DCI_ATTR_BUF_SIZE               77                    /* buffer size */
#define DCI_ATTR_NUM_ROWS               81 /* number of rows in column array */
                                  /* NOTE that DCI_ATTR_NUM_COLS is a column
                                   * array attribute too.
                                   */
#define DCI_ATTR_ROWS_FETCHED               197 /* rows fetched in last call */
#define DCI_ATTR_COL_COUNT              82        /* columns of column array
                                                    processed so far.       */
#define DCI_ATTR_NUM_COLS         102                   /* number of columns */
#define DCI_ATTR_LIST_COLUMNS     103        /* parameter of the column list */
#define DCI_ATTR_LIST_ARGUMENTS   108      /* parameter of the argument list */
#define DCI_ATTR_LIST_SUBPROGRAMS 109    /* parameter of the subprogram list */
#define DCI_ATTR_NUM_PARAMS       121                /* number of parameters */
#define DCI_ATTR_OBJID            122       /* object id for a table or view */
#define DCI_ATTR_PTYPE            123           /* type of info described by */
#define DCI_ATTR_PARAM            124                /* parameter descriptor */

#define DCI_ATTR_DIRPATH_INPUT      151    /* input in text or stream format */
#define DCI_DIRPATH_INPUT_TEXT     0x01
#define DCI_DIRPATH_INPUT_STREAM   0x02
#define DCI_DIRPATH_INPUT_UNKNOWN  0x04
/*============================== End DCI Attribute Types ====================*/
/*-------------------------Object Ptr Types----------------------------------*/
#define DCI_OTYPE_NAME 1                                      /* object name */
/*---------------------------------------------------------------------------*/
/*------------------------Error Return Values--------------------------------*/
#define DCI_SUCCESS 0                      /* maps to SQL_SUCCESS of SAG CLI */
#define DCI_SUCCESS_WITH_INFO 1             /* maps to SQL_SUCCESS_WITH_INFO */
#define DCI_NO_DATA 100                               /* maps to SQL_NO_DATA */
#define DCI_ERROR -1                                    /* maps to SQL_ERROR */
#define DCI_INVALID_HANDLE -2                  /* maps to SQL_INVALID_HANDLE */
#define DCI_NEED_DATA 99                            /* maps to SQL_NEED_DATA */
#define DCI_STILL_EXECUTING -3123                   /* DCI would block error */
/*---------------------------------------------------------------------------*/
/*--------------------- User Callback Return Values -------------------------*/
#define DCI_CONTINUE -24200    /* 为了兼容以前的程序，这个属性无作用 */
/*---------------------------------------------------------------------------*/
/*------------------------Scrollable Cursor Fetch Options------------------- 
 * For non-scrollable cursor, the only valid (and default) orientation is 
 * DCI_FETCH_NEXT
 */
#define DCI_FETCH_CURRENT 0x01               /* refetching current position  */
#define DCI_FETCH_NEXT 0x02                                      /* next row */
#define DCI_FETCH_FIRST 0x04                  /* first row of the result set */
#define DCI_FETCH_LAST 0x08                /* the last row of the result set */
#define DCI_FETCH_PRIOR 0x10         /* the previous row relative to current */
#define DCI_FETCH_ABSOLUTE 0x20                /* absolute offset from first */
#define DCI_FETCH_RELATIVE 0x40                /* offset relative to current */

/*---------------------------------------------------------------------------*/
/*----------------------- Execution Modes -----------------------------------*/
#define DCI_EXACT_FETCH       0x02         /* fetch the exact rows specified */
#define DCI_DESCRIBE_ONLY     0x10            /* only describe the statement */
#define DCI_COMMIT_ON_SUCCESS 0x20        /* commit, if successful execution */
/*---------------------------------------------------------------------------*/
/*-----------------------------  Various Modes ------------------------------*/
#define DCI_DEFAULT         0x00000000 
/*---------------------------------------------------------------------------*/
/*-------------DCIInitialize Modes / DCICreateEnvironment Modes -------------*/
#define DCI_THREADED        0x00000001      /* appl. in threaded environment */
/*---------------------------------------------------------------------------*/
/*-----------Object Types **** Not to be Used **** --------------------------*/
/* Deprecated */
/*
#define DCI_OTYPE_UNK           0
#define DCI_OTYPE_TABLE         1
#define DCI_OTYPE_VIEW          2
#define DCI_OTYPE_SYN           3
#define DCI_OTYPE_PROC          4
#define DCI_OTYPE_FUNC          5
#define DCI_OTYPE_PKG           6
#define DCI_OTYPE_STMT          7
*/
/*---------------------------------------------------------------------------*/
/*------------------------Piece Information----------------------------------*/
#define DCI_PARAM_IN 0x01                                    /* in parameter */
#define DCI_PARAM_OUT 0x02                                  /* out parameter */
/*---------------------------------------------------------------------------*/
/*=======================Describe Handle Parameter Attributes ===============*/
/* 
   These attributes are orthogonal to the other set of attributes defined 
   above.  These attrubutes are tobe used only for the desscribe handle 
*/
/*===========================================================================*/
/* Attributes common to Columns and Stored Procs */
#define DCI_ATTR_DATA_SIZE      1                /* maximum size of the data */
#define DCI_ATTR_DATA_TYPE      2     /* the SQL type of the column/argument */
#define DCI_ATTR_DISP_SIZE      3                        /* the display size */
#define DCI_ATTR_NAME           4         /* the name of the column/argument */
#define DCI_ATTR_PRECISION      5                /* precision if number type */
#define DCI_ATTR_SCALE          6                    /* scale if number type */
#define DCI_ATTR_IS_NULL        7                            /* is it null ? */
#define DCI_ATTR_TYPE_NAME      8
/* name of the named data type or a package name for package private types */
#define DCI_ATTR_SCHEMA_NAME    9             /* the schema name */
#define DCI_ATTR_SUB_NAME       10      /* type name if package private type */
#define DCI_ATTR_POSITION       11
/* relative position of col/arg in the list of cols/args */
/* complex object retrieval parameter attributes */

/* Only Columns */
#define DCI_ATTR_DISP_NAME      100                      /* the display name */
/*Only Stored Procs */
#define DCI_ATTR_NUM_ARGS       215             /* total number of arguments */
/*---------------------------------------------------------------------------*/
/*-----------------------Handle Definitions----------------------------------*/
typedef struct DCIEnv           DCIEnv;            /* DCI environment handle */
typedef struct DCIError         DCIError;                /* DCI error handle */
typedef struct DCISvcCtx        DCISvcCtx;             /* DCI service handle */
typedef struct DCIStmt          DCIStmt;             /* DCI statement handle */
typedef struct DCIBind          DCIBind;                  /* DCI bind handle */
typedef struct DCIDefine        DCIDefine;              /* DCI Define handle */
typedef struct DCIDescribe      DCIDescribe;          /* DCI Describe handle */
typedef struct DCIServer        DCIServer;              /* DCI Server handle */
typedef struct DCISession       DCISession;     /* DCI Authentication handle */
/*-----------------------Descriptor Definitions------------------------------*/
typedef struct DCISnapshot      DCISnapshot;      /* DCI snapshot descriptor */
typedef struct DCILobLocator    DCILobLocator; /* DCI Lob Locator descriptor */
typedef struct DCIParam         DCIParam;        /* DCI PARameter descriptor */
/*---------------------------------------------------------------------------*/

/*--------------------------- DCI Parameter Types ---------------------------*/
#define DCI_PTYPE_UNK                 0                         /* unknown   */
#define DCI_PTYPE_TABLE               1                         /* table     */
#define DCI_PTYPE_VIEW                2                         /* view      */
#define DCI_PTYPE_PROC                3                         /* procedure */
#define DCI_PTYPE_FUNC                4                         /* function  */
#define DCI_PTYPE_PKG                 5                         /* package   */
#define DCI_PTYPE_LIST                11                        /* list      */
/*---------------------------------------------------------------------------*/
/*--------------------------- DCI Statement Types ---------------------------*/

#define  DCI_STMT_SELECT  1                              /* select statement */
#define  DCI_STMT_UPDATE  2                              /* update statement */
#define  DCI_STMT_DELETE  3                              /* delete statement */
#define  DCI_STMT_INSERT  4                              /* Insert Statement */
#define  DCI_STMT_CREATE  5                              /* create statement */
#define  DCI_STMT_DROP    6                                /* drop statement */
#define  DCI_STMT_ALTER   7                               /* alter statement */
#define  DCI_STMT_BEGIN   8                   /* begin ... (pl/sql statement)*/
#define  DCI_STMT_DECLARE 9                /* declare .. (pl/sql statement ) */
/*---------------------------------------------------------------------------*/
#define DCI_ATTR_DIRPATH_INDEX_MAINT_METHOD 138
/*----- values for action parameter to OCIDirPathDataSave -----*/
#define DCI_DIRPATH_DATASAVE_SAVEONLY 0              /* data save point only */
#define DCI_DIRPATH_DATASAVE_FINISH   1           /* execute finishing logic */
/*--------------------------------------------------------------------------- 
/* -------------------- 8.2 dpapi support of ADTs continued ---------------- */
#define DCI_ATTR_DIRPATH_OBJ_CONSTR         206 /* obj type of subst obj tbl */
#define DCI_ATTR_INST_TYPE                  207      /* oracle instance type */
/******USED attribute 208 for  OCI_ATTR_SPOOL_STMTCACHESIZE*******************/
//<SMR-SF-10071202> ===> BEGIN SMR-SF-10071202
#define DCI_ATTR_CHARSET_ID 31                           /* Character Set ID */
#define DCI_ATTR_ENV_CHARSET_ID   DCI_ATTR_CHARSET_ID   /* charset id in env */
//<SMR-SF-10071202> ===> END

#define DCI_TRANS_READONLY     0x00000100   /* starts a readonly transaction */
#define DCI_TRANS_READWRITE    0x00000200 /* starts a read-write transaction */
#define DCI_TRANS_SERIALIZABLE 0x00000400
/*   ---------------------------------------------------------------------------*/
#define DCI_HTYPE_TRANS         10                     /* transaction handle */
#define DCI_ATTR_TRANS   8                         /* the transaction handle */
#define DCI_NUMBER_SIZE 22
#define DCI_BATCH_ERRORS      0x80             /* batch errors in array dmls */
#define DCI_PTYPE_COL                 9 
#define DCI_TRANS_NEW          0x00000001 /* starts a new transaction branch */
#define DCI_NUMBER_UNSIGNED 0                        /* Unsigned type -- ubX */
#define DCI_NUMBER_SIGNED   2                          /* Signed type -- sbX */

#define DCI_ATTR_ENV_STMT_NUM	  9999
struct DCINumber
{
	ub1 DCINumberPart[DCI_NUMBER_SIZE];
};
typedef struct DCINumber DCINumber;
struct DCITime
{
  ub1 DCITimeHH;                          /* hours; range is 0 <= hours <=23 */
  ub1 DCITimeMI;                     /* minutes; range is 0 <= minutes <= 59 */
  ub1 DCITimeSS;                     /* seconds; range is 0 <= seconds <= 59 */
};
typedef struct DCITime DCITime;

/* 
 * DCITime - DCI TiMe portion of date
 *
 * This structure should be treated as an opaque structure as the format
 * of this structure may change. Use DCIDateGetTime/DCIDateSetTime 
 * to manipulate time portion of DCIDate.
 */

struct DCIDate
{
  sb2 DCIDateYYYY;         /* gregorian year; range is -4712 <= year <= 9999 */
  ub1 DCIDateMM;                          /* month; range is 1 <= month < 12 */
  ub1 DCIDateDD;                             /* day; range is 1 <= day <= 31 */
  DCITime DCIDateTime;                                               /* time */
};
typedef struct DCIDate DCIDate; 
#define DCIDateGetTime(date, hour, min, sec) \
  { \
     *hour = (date)->DCIDateTime.DCITimeHH; \
     *min = (date)->DCIDateTime.DCITimeMI; \
     *sec = (date)->DCIDateTime.DCITimeSS; \
  }
/*
   NAME: DCIDateGetTime - DCIDate Get Time portion of date
   PARAMETERS:
  	date (IN) - date whose time data is retrieved
	hour (OUT) - hour value returned
	min (OUT) - minute value returned
	sec (OUT) - second value returned
   DESCRIPTION:
  	Return time inforamtion stored in the given date. The time
	information returned is: hour, minute and seconds.
   RETURNS:
        NONE
 */
typedef struct DCIDateTime DCIDateTime;           /* OCI DateTime descriptor */
typedef struct DCIInterval DCIInterval;           /* OCI Interval descriptor */
/*--------------------------- DCIDateGetDate --------------------------------*/
/* void DCIDateGetDate(/o_ CONST DCIDate *date, sb2 *year, ub1 *month, 
                           ub1 *day _o/); */
#define DCIDateGetDate(date, year, month, day) \
  { \
     *year = (date)->DCIDateYYYY; \
     *month = (date)->DCIDateMM; \
     *day = (date)->DCIDateDD; \
  }
/*
   NAME: DCIDateGetDate - DCIDate Get Date (year, month, day) portion of date
   PARAMETERS:
  	date (IN) - date whose year, month, day data is retrieved
	year (OUT) - year value returned
	month (OUT) - month value returned
	day (OUT) - day value returned
   DESCRIPTION:
  	Return year, month, day inforamtion stored in the given date.
   RETURNS:
        NONE
 */

/*--------------------------- DCIDateSetTime --------------------------------*/
/* void DCIDateSetTime(/o_ DCIDate *date, ub1 hour, ub1 min, 
                           ub1 sec _o/); */
#define DCIDateSetTime(date, hour, min, sec) \
  { \
     (date)->DCIDateTime.DCITimeHH = hour; \
     (date)->DCIDateTime.DCITimeMI = min; \
     (date)->DCIDateTime.DCITimeSS = sec; \
  }
/*
   NAME: DCIDateSetTime - DCIDate Set Time portion of date
   PARAMETERS:
  	date (OUT) - date whose time data is set
	hour (IN) - hour value to be set 
	min (IN) - minute value to be set
	sec (IN) - second value to be set
   DESCRIPTION:
  	Set the date with the given time inforamtion.
   RETURNS:
        NONE
 */

/*--------------------------- DCIDateSetDate --------------------------------*/
/* void DCIDateSetDate(/o_ DCIDate *date, sb2 year, ub1 month, ub1 day _o/); */
#define DCIDateSetDate(date, year, month, day) \
  { \
     (date)->DCIDateYYYY = year; \
     (date)->DCIDateMM = month; \
     (date)->DCIDateDD = day; \
  }
     /*----- values for cflg argument to DCIDirpathColArrayEntrySet -----*/
#define DCI_DIRPATH_COL_COMPLETE 0                /* column data is complete */
#define DCI_DIRPATH_COL_NULL     1                         /* column is null */
#define DCI_DIRPATH_COL_PARTIAL  2                 /* column data is partial */
#define DCI_DIRPATH_COL_ERROR    3               /* column error, ignore row */


typedef struct DCIDirPathCtx      DCIDirPathCtx;                  /* context */
typedef struct DCIDirPathColArray DCIDirPathColArray;        /* column array */
typedef struct DCIDirPathStream   DCIDirPathStream;                /* stream */

DCI_API sword   
DCIInitialize(
	ub4 mode, 
	dvoid *ctxp, 
	dvoid *(*malocfp)(
		dvoid *ctxp, 
		size_t size
		),
	dvoid *(*ralocfp)(
		dvoid *ctxp, 
		dvoid *memptr, 
		size_t newsize
		),
	void (*mfreefp)(
		dvoid *ctxp, 
		dvoid *memptr
		)
	);
DCI_API sword   
DCIEnvInit(
	DCIEnv **envp, 
	ub4 mode, 
	size_t xtramem_sz, 
	dvoid **usrmempp
	);

DCI_API sword	
DCIEnvCreate(
	DCIEnv **envhpp, 
	ub4 mode, 
	CONST dvoid *ctxp, 
	CONST dvoid *(*malocfp)(
		dvoid *ctxp, 
		size_t size
		), 
	CONST dvoid *(*ralocfp)(
		dvoid *ctxp, 
		dvoid *memptr, 
		size_t newsize
		), 
	CONST void (*mfreefp)(
		dvoid *ctxp, 
		dvoid *memptr
		), 
	size_t xtramemsz, 
	dvoid **usrmempp
	);

DCI_API sword   
DCIHandleAlloc(
	dvoid *parenth, 
	dvoid **hndlpp, 
	CONST ub4 type, 
	CONST size_t xtramem_sz, 
	dvoid **usrmempp
	);

DCI_API sword   
DCIHandleFree(
	dvoid *hndlp, 
	CONST ub4 type
	);

DCI_API sword   
DCIAttrSet(
	dvoid *trgthndlp, 
	ub4 trghndltyp, 
	dvoid *attributep,
	ub4 size, 
	ub4 attrtype, 
	DCIError *errhp
	);

DCI_API sword   
DCISessionBegin(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	DCISession *usrhp,
	ub4 credt, 
	ub4 mode
	);

DCI_API sword   
DCIServerAttach(
	DCIServer *srvhp, 
	DCIError *errhp,
	CONST DciText *dblink, 
	sb4 dblink_len, 
	ub4 mode
	);

DCI_API sword   
DCIStmtPrepare(
	DCIStmt *stmtp, 
	DCIError *errhp, 
	DciText *stmt,
	ub4 stmt_len, 
	ub4 language, 
	ub4 mode
	);

DCI_API sword   
DCIStmtExecute(
	DCISvcCtx *svchp, 
	DCIStmt *stmtp, 
	DCIError *errhp, 
	ub4 iters, 
	ub4 rowoff, 
	CONST DCISnapshot *snap_in, 
	DCISnapshot *snap_out, 
	ub4 mode
	);

DCI_API sword   
DCIStmtFetch(
	DCIStmt *stmtp, 
	DCIError *errhp, 
	ub4 nrows, 
	ub2 orientation, 
	ub4 mode
	);

DCI_API sword   
DCIDefineByPos(
	DCIStmt *stmtp, 
	DCIDefine **defnp, 
	DCIError *errhp,
	ub4 position, 
	dvoid *valuep, 
	sb4 value_sz, 
	ub2 dty,
	dvoid *indp, 
	ub2 *rlenp, 
	ub2 *rcodep, 
	ub4 mode
	);

DCI_API sword   
DCIErrorGet(
	dvoid *hndlp, 
	ub4 recordno, 
	DciText *sqlstate,
	sb4 *errcodep, 
	DciText *bufp, 
	ub4 bufsiz, 
	ub4 type
	);

DCI_API sword
DCIBindByPos(
	DCIStmt *stmtp, 
	DCIBind **bindp, 
	DCIError *errhp,
	ub4 position, 
	dvoid *valuep, 
	sb4 value_sz,
	ub2 dty, 
	dvoid *indp, 
	ub2 *alenp, 
	ub2 *rcodep,
	ub4 maxarr_len, 
	ub4 *curelep, 
	ub4 mode
	);

DCI_API sword   
DCIBindByName(
	DCIStmt *stmtp, 
	DCIBind **bindp, 
	DCIError *errhp,
	CONST DciText *placeholder, 
	sb4 placeh_len, 
    dvoid *valuep, 
	sb4 value_sz, 
	ub2 dty, 
    dvoid *indp, 
	ub2 *alenp, 
	ub2 *rcodep, 
	ub4 maxarr_len, 
	ub4 *curelep, 
	ub4 mode
	);

DCI_API sword   
DCILogon(
	DCIEnv *envhp, 
	DCIError *errhp, 
	DCISvcCtx **svchp, 
	CONST DciText *username, 
	ub4 uname_len, 
	CONST DciText *password, 
	ub4 passwd_len, 
	CONST DciText *dbname, 
	ub4 dbname_len
	);

DCI_API sword   
DCILogoff(
	DCISvcCtx *svchp, 
	DCIError *errhp
	);

DCI_API sword   
DCITransCommit(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	ub4 flags
	);

DCI_API sword
DCITransRollback(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	ub4 flags
	);

DCI_API sword   
DCIServerDetach(
	DCIServer *srvhp, 
	DCIError *errhp, 
	ub4 mode
	);

DCI_API sword   
DCISessionEnd(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	DCISession *usrhp, 
	ub4 mode
	);

DCI_API sword   
DCIAttrGet(
	CONST dvoid *trgthndlp, 
	ub4 trghndltyp, 
	dvoid *attributep, 
	ub4 *sizep, 
	ub4 attrtype, 
	DCIError *errhp
	);

DCI_API sword   
DCIAttrSet(
	dvoid *trgthndlp, 
	ub4 trghndltyp, 
	dvoid *attributep,
	ub4 size, 
	ub4 attrtype, 
	DCIError *errhp
	);

DCI_API sword   
DCIParamGet(
	CONST dvoid *hndlp, 
	ub4 htype, 
	DCIError *errhp, 
	dvoid **parmdpp, 
	ub4 pos
	);

DCI_API sword   
DCIParamSet(
	dvoid *hdlp, 
	ub4 htyp, 
	DCIError *errhp, 
	CONST dvoid *dscp,
	ub4 dtyp, 
	ub4 pos
	);

DCI_API sword   
DCIDescribeAny(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	dvoid *objptr, 
	ub4 objnm_len, 
	ub1 objptr_typ, 
	ub1 info_level,
	ub1 objtyp, 
	DCIDescribe *dschp
	);

DCI_API sword   
DCIBindArrayOfStruct(
	DCIBind *bindp, 
	DCIError *errhp, 
    ub4 pvskip, 
	ub4 indskip,
    ub4 alskip, 
	ub4 rcskip
	);

DCI_API sword   
DCIDefineArrayOfStruct(
	DCIDefine *defnp, 
	DCIError *errhp, 
	ub4 pvskip,
	ub4 indskip, 
	ub4 rlskip, 
	ub4 rcskip
	);

DCI_API sword   
DCIDescriptorAlloc(
	CONST dvoid *parenth, 
	dvoid **descpp, 
	CONST ub4 type, 
	CONST size_t xtramem_sz, 
	dvoid **usrmempp
	);

DCI_API sword
DCIDescriptorFree(
	dvoid *descp, 
	CONST ub4 type
	);

DCI_API sword   
DCILobWrite(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	DCILobLocator *locp,
	ub4 *amtp, 
	ub4 offset, 
	dvoid *bufp, 
	ub4 buflen, 
	ub1 piece, 
	dvoid *ctxp,
	sb4 (*cbfp)(dvoid *ctxp, 
		dvoid *bufp, 
		ub4 *len, 
		ub1 *piece),
	ub2 csid, 
	ub1 csfrm
	);

DCI_API sword   
DCILobGetLength(
	DCISvcCtx *svchp, 
	DCIError *errhp,
	DCILobLocator *locp,
    ub4 *lenp
	);

DCI_API sword
DCILobRead(
	DCISvcCtx *svchp, 
	DCIError *errhp, 
	DCILobLocator *locp,
    ub4 *amtp, 
	ub4 offset, 
	dvoid *bufp, 
	ub4 bufl, 
    dvoid *ctxp, 
	sb4 (*cbfp)(dvoid *ctxp, 
		CONST dvoid *bufp, 
		ub4 len, 
		ub1 piece
		),
	ub2 csid, 
	ub1 csfrm
	);

//<SMR-YYL-2011062701> ===> BEGIN
DCI_API sword 
DCILobCopy(
     DCISvcCtx        *svchp,
     DCIError         *errhp,
     DCILobLocator    *dst_locp,
     DCILobLocator    *src_locp,
     ub4              amount,
     ub4              dst_offset,
     ub4              src_offset 
);
DCI_API sword 
DCILobTrim(
    DCISvcCtx      *svchp,
    DCIError       *errhp,
    DCILobLocator  *locp,
    ub4            newlen
);
DCI_API sword
DCILobLoadFromFile (
    DCISvcCtx    *svchp, 
    DCIError          *errhp, 
    DCILobLocator    *dst_locp, 
    DCILobLocator    *src_locp, 
    ub4                amount, 
    ub4                dst_offset, 
    ub4                src_offset 
); 

DCI_API sword 
DCILobFileExists(
    DCISvcCtx       *svchp,
    DCIError        *errhp,
    DCILobLocator   *filep,
    ub4         *flag 
);
DCI_API sword 
DCILobFileOpen( 
    DCISvcCtx            *svchp,
    DCIError             *errhp,
    DCILobLocator        *filep,
    ub1                  mode 
);
DCI_API sword 
DCILobFileSetName( 
    DCIEnv             *envhp,
    DCIError           *errhp,
    DCILobLocator      **filepp,
    const DciText      *dir_alias,
    ub2                d_length, 
    const DciText      *filename, 
    ub2                f_length 
);
DCI_API sword 
DCILobFileClose( 
    DCISvcCtx            *svchp,
    DCIError             *errhp,
    DCILobLocator        *filep 
);
//<SMR-YYL-2011062701> ===> END

DCI_API sword   
DCIStmtFetch2(
	DCIStmt *stmtp, 
	DCIError *errhp, 
	ub4 nrows,
	ub2 orientation, 
	sb4 scrollOffset, 
	ub4 mode
	);

DCI_API sword   
DCIPrepareForUpdate(
	DCISvcCtx *svchp,	//上下文句柄
	DCIStmt *stmtp,		//语句句柄
	DCIError *errhp,	//错误信息句柄
	DciText *db,		//库名
	ub2 db_len,			//库名长度
	DciText *schema,	//模式名
	ub2 schema_len,		//模式名长度
	DciText *table,		//表名，调用该函数，该表上必需要有主键，并且主键数据类型为INT
	ub2 table_len,		//表名长度
	DciText *column,		//待更新的列名
	ub2 column_len,		//列名长度
	ub4 *itersp			//输出参数，返回将要更新的行
	);

DCI_API sword   
DCIBindParamForUpdate(
	DCISvcCtx *svchp,	//上下文句柄
	DCIStmt *stmtp,		//语句句柄
	DCIError *errhp,	//错误信息句柄
	dvoid *value_pkp,		//待绑定的表主键指针，主键列类型只允许int类型，也就是说值的大小已经必需固定为4
	ub4 ppkvskip,		//主键每个值之间相隔的字节数，也就是主键值的下一跳字节数
	dvoid *valuep,		//待绑定的缓冲区指针，该指针指向待更新列存放值的内存地址
	sb4 value_sz,		//单个值所占的字节数
	ub4 pvskip,			//更新列每个值的间隔字节数，也就是该值的下一跳字节数
    ub2 dty,			//待更新列的DCI数据类型
	dvoid *indp,		//指示符指针，该指针是一个sb2类型，存放对应列元组的大小，如果指定的值长度为-1，则表示要更新成一个空值
	ub4 indskip			//指示符间隔字节数，也就是该值的下一跳字节数
	);

DCI_API sword   
DCIExecuteForUpdate(
	DCISvcCtx *svchp,	//上下文句柄
	DCIStmt *stmtp,		//语句句柄
	DCIError *errhp,	//错误信息句柄
	ub4 rowoff,			//起始行偏移，默认为0
	ub4 mode			//更新的模式，一种是DCI_DEFAULT(需要手动提交),一种是DCI_COMMIT_ON_SUCCESS（更新成功便自动提交）
	);

DCI_API sword
DCIDirPathPrepare(
	DCIDirPathCtx *dpctx, 
	DCISvcCtx *svchp,
    DCIError  *errhp
	);

DCI_API sword
DCIDirPathColArrayReset(
	DCIDirPathColArray *dpca, 
	DCIError *errhp 
	);

DCI_API sword
DCIDirPathStreamReset(
	DCIDirPathStream *dpstr, 
	DCIError *errhp 
	);

DCI_API sword
DCIDirPathColArrayEntrySet(
	DCIDirPathColArray *dpca, 
	DCIError *errhp,
    ub4 rownum, 
	ub2 colIdx, 
	ub1 *cvalp, 
	ub4 clen,
    ub1 cflg 
	);

DCI_API sword
DCIDirPathDataSave( 
	DCIDirPathCtx *dpctx, 
	DCIError *errhp, 
	ub4 action 
	);

DCI_API sword
DCIDirPathColArrayToStream(
	DCIDirPathColArray *dpca,  
	DCIDirPathCtx *dpctx,
    DCIDirPathStream   *dpstr, 
	DCIError      *errhp,
    ub4 rowcnt, ub4 rowoff
	);

DCI_API sword
DCIDirPathFinish(
	DCIDirPathCtx *dpctx,
	DCIError  *errhp
	);

DCI_API sword
DCIDirPathLoadStream(
	DCIDirPathCtx *dpctx, 
	DCIDirPathStream *dpstr,
    DCIError      *errhp
	);

DCI_API sword
DCIDirPathFlushRow( 
	DCIDirPathCtx *dpctx, 
	DCIError  *errhp 
	);

DCI_API sword
DCIDirPathAbort(
	DCIDirPathCtx *dpctx, 
	DCIError *errhp 
	);
DCI_API sword 
DCIDateTimeGetDate(
				   dvoid *hndl,
				   DCIError *err,  
				   CONST DCIDateTime *date, 
				   sb2 *yr, 
				   ub1 *mnth, 
				   ub1 *dy
				   );
DCI_API sword 
DCIDateTimeGetTime(dvoid *hndl, DCIError *err, DCIDateTime *datetime, 
				   ub1 *hr, ub1 *mm, ub1 *ss, ub4 *fsec);

DCI_API sword   
DCIStmtGetPieceInfo(
					DCIStmt *stmtp, 
					DCIError *errhp,
					dvoid **hndlpp, 
					ub4 *typep, 
					ub1 *in_outp,
					ub4 *iterp, 
					ub4 *idxp, 
					ub1 *piecep);
DCI_API sword   
DCIStmtSetPieceInfo(
					dvoid *hndlp, 
					ub4 type, 
					DCIError *errhp,
					dvoid *bufp, 
					ub4 *alenp, 
					ub1 piece,
					CONST dvoid *indp, 
	ub2 *rcodep);
DCI_API sword   
DCIPrintMemory();
//<SQLT_INTERVAL_DS> ===> BEGIN SMR-SF-10101101
DCI_API sword 
DCIIntervalGetDaySecond(
	dvoid *hndl, 
	DCIError *err, 
	sb4 *dy, 
	sb4 *hr,
    sb4 *mm,
	sb4 *ss, 
	sb4 *fsec, 
	CONST DCIInterval *result);

DCI_API sword 
DCIIntervalFromText(
	dvoid *hndl, 
	DCIError *err, 
	CONST DciText *inpstr, 
	size_t str_len, 
	DCIInterval *result);

DCI_API sword 
DCIDateTimeConstruct(
	dvoid  *hndl,
	DCIError *err,
	DCIDateTime *datetime,
	sb2 yr,
	ub1 mnth,
	ub1 dy,
	ub1 hr,
	ub1 mm,
	ub1 ss,
	ub4 fsec,
    DciText  *timezone,
	size_t timezone_length);
//<SQLT_INTERVAL_DS> ===> END
//<OCITRANS> ===> BEGIN SMR-SF-10101402
DCI_API sword 
DCITransStart( 
	DCISvcCtx *svchp,
	DCIError *errhp,
	uword timeout,
	ub4 flags);

DCI_API sword 
DCITransDetach( 
	DCISvcCtx *svchp,
	DCIError *errhp,
	ub4 flags);
//<OCITRANS> ===> END
DCI_API sword 
DCINumberToReal(
	DCIError *err, 
	CONST DCINumber *number,
	uword rsl_length, 
	dvoid *rsl);

DCI_API sword
DCINumberToInt(
	DCIError *err, 
	CONST DCINumber *number,
	uword rsl_length, 
	uword rsl_flag, 
	dvoid *rsl);

DCI_API sword 
DCINumberToText(
	DCIError *err, 
	CONST DCINumber *number, 
	CONST DciText *fmt, 
	ub4 fmt_length,
	CONST DciText *nls_params, 
	ub4 nls_p_length,
	ub4 *buf_size, 
	DciText *buf);

//<SMR-SF-2011071201> ===> BEGIN 
DCI_API sword
DCICloseSvcSocket(
	DCISvcCtx *svchp
);
//<SMR-SF-2011071201> ===> END 

#ifdef __cplusplus
}
#endif

#endif
