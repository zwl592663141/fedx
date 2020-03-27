/***********************************************************
 * Copyright (c) 1999,2002.
 * Descriptions:
 *      Oracle 的 directpath API.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wangjs       03/20/2007        create
 **********************************************************/

#ifndef _DPAPI_ORACLE_
#define _DPAPI_ORACLE_

#include <oratypes.h>
#include <oci.h>
#include "ora_dptype.h"
#include "sdfs.h"

struct loadctl
{
  ub4                 nrow_ctl;            /* number of rows in column array */
  ub2                 ncol_ctl;         /* number of columns in column array */
  OCIEnv             *envhp_ctl;                       /* environment handle */
  OCIServer          *srvhp_ctl;                            /* server handle */
  OCIError           *errhp_ctl;                             /* error handle */
  OCIError           *errhp2_ctl;                /* yet another error handle */
  OCISvcCtx          *svchp_ctl;                          /* service context */
  OCISession         *authp_ctl;                   /* authentication context */
  OCIParam           *colLstDesc_ctl;        /* column list parameter handle */
  OCIDirPathCtx      *dpctx_ctl;                      /* direct path context */
  OCIDirPathColArray *dpca_ctl;           /* direct path column array handle */
  OCIDirPathStream   *dpstr_ctl;                /* direct path stream handle */
  ub1                *buf_ctl;    /* pre-alloc'd buffer for out-of-line data */
  ub4                 bufsz_ctl;                 /* size of buf_ctl in bytes */
  ub4                 bufoff_ctl; /* offset into buf_ctl which is not in use */
  ub4                *otor_ctl;                  /* Offset to Recnum mapping */
  ub1                *inbuf_ctl;                 /* buffer for input records */
  struct pctx         pctx_ctl;                     /* partial field context */
};

/* External column attributes */
struct col_def
{
  char  name_col[50];                                         /* column name */
  ub2   id_col;                                            /* column load id */
  ub2   exttyp_col;                                         /* external type */
  char  datemask_col[30];                         /* datemask, if applicable */
  ub1   prec_col;                                /* precision, if applicable */
  sb1   scale_col;                                   /* scale, if applicable */
  ub2   csid_col;                                        /* character set id */
  ub1   date_col;            /* is column a chrdate or date? 1=TRUE. 0=FALSE */
};

/* Input field descriptor
 * For this example (and simplicity),
 * fields are strictly positional.
 */
struct fld_def
{
  ub4  begpos_fld;                             /* 1-based beginning position */
  ub4  endpos_fld;                             /* 1-based ending    position */
  ub4  maxlen_fld;                       /* max length for out of line field */
  ub4    flag_fld;
#define FLD_INLINE            0x1
#define FLD_OUTOFLINE         0x2
#define FLD_STRIP_LEAD_BLANK  0x4
#define FLD_STRIP_TRAIL_BLANK 0x8
};

struct tab_def
{
  char        owner[30];                                      /* table owner */
  char        table_name[100];                                 /* table name */
  char        sub_name[100];                       /* subname, if applicable */
  ub2         col_num;                       /* number of columns in col_tbl */
  char        date_mask[30];                /* table level default date mask */
  struct col_def  *column;                              /* column attributes */
  struct fld_def  *field;                                /* field descriptor */
  ub1         parallel_flag;                         /* parallel: 1 for true */
  ub1         nolog_flag;                          /* no logging: 1 for true */
};

/*登陆*/
extern short Connect_DBDP(char *outStr, struct loadctl *ctlp, char *username, char *password, char *server);

/*初始内存*/
extern short init_load(char *outStr, struct loadctl *ctlp, struct tab_def *tblp);

/*入库*/
extern short simple_load(char *outStr, long *totalNum, FILE *errfp, struct loadctl *ctlp,
                     SFILE *inputfp, char *addStr, void *vFI);

/*提交*/
extern short finish_load(char *outStr, struct loadctl *ctlp);

/*释放内存*/
extern void cleanup(struct loadctl *ctlp, sb4 ex_status);

#endif                                              /* cdemodp_ORACLE */
