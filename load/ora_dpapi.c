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

#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

#ifdef UNIX_OS
  #include <unistd.h>
#else
  #include <io.h>
#endif

#include "ora_dpapi.h"
#include "load_main.h"

#ifndef bit
# define bit(x, y) ((x) & (y))
#endif

#ifndef OER
# define OER(x) (x)
#endif

#define MAX_DP_RECLEN 4000    /*每条记录的最大长度*/

/*获取数据库错误代码*/
sb4 GetDBDPErrCode( char *errStr, OCIError *errhp )
{
    int i;
    char errBuf[512];
    sb4 errcode = 0;

    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
               (text *)errBuf, (ub4) sizeof(errBuf), OCI_HTYPE_ERROR);
    errBuf[100]='\0';
    for(i=0;i<(int)strlen(errBuf);i++)
    {
        if(errBuf[i]=='\n')
        {
            errBuf[i]='\0';
            break;
        }
    }
    sprintf(errStr, errBuf);
    return errcode;
}

#define Check_Err1(outStr, errhp, status) \
   Check_Err((char *)outStr, (OCIError *)errhp, (sword)status, \
   (text *)__FILE__, (sb4)__LINE__)

/*判断数据库错误*/
short Check_Err( char *outStr, OCIError *errhp, sword status, text *file, sb4 line  )
{
    sb4 errcode = 0;

    switch (status)
    {
        case OCI_SUCCESS:
            sprintf(outStr, "SUCCESS");
            break;
        case OCI_SUCCESS_WITH_INFO:
            sprintf(outStr, "OCI_SUCCESS_WITH_INFO");
            break;
        case OCI_NEED_DATA:
            sprintf(outStr, "OCI_NEED_DATA");
            break;
        case OCI_NO_DATA:
            sprintf(outStr, "OCI_NO_DATA");
            break;
        case OCI_ERROR:
            errcode = GetDBDPErrCode( outStr, errhp );
            break;
        case OCI_INVALID_HANDLE:
            sprintf(outStr, "OCI_INVALID_HANDLE");
            break;
        case OCI_STILL_EXECUTING:
            strcpy(outStr, "OCI_STILL_EXECUTING");
            break;
        case OCI_CONTINUE:
            sprintf(outStr, "OCI_CONTINUE");
            break;
        default:
            sprintf(outStr, "DEFAULT");
    }
    if(status == OCI_SUCCESS)
        return 0;
    else
    {
        /*sprintf(outStr, "%s.\tError occurred at File %s:%d",outStr, file, line);*/
        return -1;
    }
}

/*------------------------------ fieldFlush ------------------------------*/
/*
 NAME      fieldFlush

 FUNCTION: Helper function which cleans up the partial context
           state, and clears it.
 */
void fieldFlush(struct loadctl *ctlp, ub4 rowoff)
{
    if (ctlp->pctx_ctl.valid_pctx)
    {
        /* Partial context is valid; make sure the request is
         * for the context corresponding to the current row.
         */
        assert(rowoff == ctlp->pctx_ctl.row_pctx);
        (void) close(ctlp->pctx_ctl.fd_pctx);
        free((void *)ctlp->pctx_ctl.fnm_pctx);
    }
    CLEAR_PCTX(ctlp->pctx_ctl);
}

/*------------------------------ field_set ------------------------------*/
/*出错 返回99*/
sword field_set(char *outStr, struct loadctl *ctlp, struct tab_def *tblp, text *recp, ub4 rowoff)
{
  ub1  *cval=NULL;
  ub4   thiscol;
  ub4   clen=0, j;                                            /* column length */
  ub1   cflg=0;
  int   fd;                          /* file descriptor for out-of-line data */
  char *filename;                           /* filename for out-of-line data */
  sword  partial;

  ctlp->bufoff_ctl = 0;

  if ((partial = (sword)ctlp->pctx_ctl.valid_pctx) == TRUE)
  {
    /* partial context is valid; resume where we left off */
    assert(rowoff == ctlp->pctx_ctl.row_pctx);
    thiscol = ctlp->pctx_ctl.col_pctx;
  }
  else
    thiscol = 0;

  for (/* empty */; thiscol < tblp->col_num; thiscol++)
  {
    struct col_def *colp =  &tblp->column[thiscol];
    struct fld_def *fldp =  &tblp->field[thiscol];

    if (partial)
    {
      /* partials are always from a secondary file */
      fd       = ctlp->pctx_ctl.fd_pctx;
      filename = ctlp->pctx_ctl.fnm_pctx;
    }
    else                                                         /* !partial */
    {
      fd       = -1;
      filename = (char *)0;
      cval     = (ub1 *)recp + fldp->begpos_fld - 1;
      clen     = fldp->endpos_fld - fldp->begpos_fld + 1;

      j = 0;
      if (bit(fldp->flag_fld, FLD_STRIP_LEAD_BLANK))
      {
        /* trim leading white space */
        for (/*empty*/; j < clen; j++)
          if (!isspace((int)cval[j]))
            break;
      }

      if (j >= clen)
        clen = 0;                              /* null column, handled below */
      else
      {
        if (bit(fldp->flag_fld, FLD_STRIP_TRAIL_BLANK))
        {
          /* trim trailing white space */
          while (clen && isspace((int)cval[clen - 1]))
            clen--;
        }
        cval = cval + j;
        clen = clen - j;
      }

      if (clen)
      {
        if (bit(fldp->flag_fld, FLD_INLINE))
        {
          cflg = OCI_DIRPATH_COL_COMPLETE;
        }
        else if (bit(fldp->flag_fld, FLD_OUTOFLINE))
        {
          filename = (char *)malloc((size_t)clen+1);
          if (!filename)
          {
            sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
                    (clen + 1), __FILE__, __LINE__);
            return 99;
          }
          (void) memcpy((dvoid *)filename, (dvoid *)cval, (size_t)clen);
          filename[clen] = 0;
          fd = open(filename, O_RDONLY);
          SET_PCTX(ctlp->pctx_ctl, rowoff, thiscol, (ub4)0, fd, filename);
          LEN_PCTX(ctlp->pctx_ctl) = 0;
        }
        else
        {
          sprintf(outStr, "field_set: unknown field type.\tError occurred at File %s:%d",
                     __FILE__, __LINE__);
          return 99;
        }
      }
      else
      {
        cflg = OCI_DIRPATH_COL_NULL;               /* all spaces become null */
        cval = (ub1 *)0;
      }
    }

    if (bit(fldp->flag_fld, FLD_OUTOFLINE))
    {
      char *buf;
      int   bufsz;
      int   cnt;

      if (!ctlp->buf_ctl)
      {
        ctlp->buf_ctl   = (ub1 *)malloc((size_t)SECONDARY_BUF_SIZE);
        ctlp->bufsz_ctl = SECONDARY_BUF_SIZE;
      }

      if ((ctlp->bufsz_ctl - ctlp->bufoff_ctl) > SECONDARY_BUF_SLOP)
      {
        buf   = (char *)ctlp->buf_ctl + ctlp->bufoff_ctl;  /* buffer pointer */
        bufsz = (int)ctlp->bufsz_ctl  - ctlp->bufoff_ctl;     /* buffer size */

        if (fd == -1)
          cnt = 0;
        else
          cnt = read(fd, buf, bufsz);

        if (cnt != -1)
        {
          cval = (ub1 *)buf;
          clen = (ub4)cnt;

          if (cnt < bufsz)                    /* all file data has been read */
          {
            /* mark column as null or complete */
            if (cnt == 0 && LEN_PCTX(ctlp->pctx_ctl) == 0)
              cflg = OCI_DIRPATH_COL_NULL;
            else
              cflg = OCI_DIRPATH_COL_COMPLETE;

            fieldFlush(ctlp, rowoff);          /* close file, free filename */

            /* adjust offset into buffer for use by next field */
            ctlp->bufoff_ctl += cnt;
          }
          else
            cflg  = OCI_DIRPATH_COL_PARTIAL;
        }
        else
        {
          /* XXX: do something on read failure, like return an error context */
          fieldFlush(ctlp, rowoff);          /* close file, free filename */
          return FIELD_SET_ERROR;
        }
      }
      else
      {
        /* no room in secondary buffer, return a 0 length partial
         * and pick it up next time.
         */
        cflg = OCI_DIRPATH_COL_PARTIAL;
        clen = 0;
        cval = (ub1 *)NULL;
      }
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIDirPathColArrayEntrySet(ctlp->dpca_ctl,
        ctlp->errhp_ctl, rowoff, colp->id_col, cval, clen, cflg))<0)
        return 99;

    if (cflg == OCI_DIRPATH_COL_PARTIAL)
    {
      /* Partials only occur for OutOfLine data
       * remember the row offset, column offset,
       * total length of the column so far,
       * and file descriptor to get data from on
       * subsequent calls to this function.
       */
      LEN_PCTX(ctlp->pctx_ctl) += clen;
      return FIELD_SET_PARTIAL;
    }
  }

  CLEAR_PCTX(ctlp->pctx_ctl);
  if (ctlp->bufoff_ctl)             /* data in secondary buffer for this row */
    return FIELD_SET_BUF;
  else
    return FIELD_SET_COMPLETE;
}

/* init_load:
 *   Function which prepares for a direct path load using the direct
 *   path API on the table described by 'tblp'.  The loadctl
 *   structure given by 'ctlp' has appropriately initialized
 *   environment, and service context handles (alredy connected to
 *   the server) prior to calling this function.
 */
short init_load(char *outStr, struct loadctl *ctlp, struct tab_def *tblp)
{
    struct  col_def   *colp;
    struct  fld_def   *fldp;
    OCIDirPathCtx *dpctx;                               /* direct path context */
    OCIParam      *colDesc;                     /* column parameter descriptor */
    ub1            parmtyp;
    ub2            i;
    ub4            pos;
    int  maxreclen = MAX_DP_RECLEN;   /*每条记录最大长度*/

    /* allocate and initialize a direct path context */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->dpctx_ctl, (ub4)OCI_HTYPE_DIRPATH_CTX,(size_t)0, (dvoid **)0))<0)
        return -1;

    dpctx = ctlp->dpctx_ctl;                                      /* shorthand */

    if(Check_Err1(outStr, ctlp->errhp_ctl,OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
        (dvoid *)tblp->table_name,(ub4)strlen((const char *)tblp->table_name),
        (ub4)OCI_ATTR_NAME, ctlp->errhp_ctl))<0)
        return -1;


    if (tblp->sub_name && *tblp->sub_name)    /* set (sub)partition name */
    {
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
             (dvoid *)tblp->sub_name, (ub4)strlen((const char *)tblp->sub_name),
             (ub4)OCI_ATTR_SUB_NAME, ctlp->errhp_ctl))<0)
             return -1;
    }

    if (tblp->owner)                            /* set schema (owner) name */
    {
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
             (dvoid *)tblp->owner, (ub4)strlen((const char *)tblp->owner),
             (ub4)OCI_ATTR_SCHEMA_NAME, ctlp->errhp_ctl))<0)
             return -1;
    }

    /* Note: setting tbl default datemask will not trigger client library
     * to check strings for dates - only setting column datemask will.
     */
    if (tblp->date_mask)
    {
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
             (dvoid *)tblp->date_mask, (ub4)strlen((const char *)tblp->date_mask),
             (ub4)OCI_ATTR_DATEFORMAT, ctlp->errhp_ctl))<0)
             return -1;
    }

    if (tblp->parallel_flag)                 /* set table level parallel option */
    {
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
             (dvoid *)&tblp->parallel_flag, (ub4)0, (ub4)OCI_ATTR_DIRPATH_PARALLEL,
             ctlp->errhp_ctl))<0)
             return -1;
    }

    if (tblp->nolog_flag)                       /* set table level nolog option */
    {
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
             (dvoid *)&tblp->nolog_flag, (ub4)0, (ub4)OCI_ATTR_DIRPATH_NOLOG, ctlp->errhp_ctl))<0)
             return -1;
    }

    /* set number of columns to be loaded */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)dpctx, (ub4)OCI_HTYPE_DIRPATH_CTX,
        (dvoid *)&tblp->col_num,(ub4)0, (ub4)OCI_ATTR_NUM_COLS, ctlp->errhp_ctl))<0)
        return -1;

    /* get the column parameter list */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((dvoid *)dpctx, OCI_HTYPE_DIRPATH_CTX,
        (dvoid *)&ctlp->colLstDesc_ctl, (ub4 *)0, OCI_ATTR_LIST_COLUMNS, ctlp->errhp_ctl))<0)
        return -1;

    /* get attributes of the column parameter list */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)ctlp->colLstDesc_ctl,
        OCI_DTYPE_PARAM, (dvoid *)&parmtyp, (ub4 *)0, OCI_ATTR_PTYPE, ctlp->errhp_ctl))<0)
        return -1;

    if (parmtyp != OCI_PTYPE_LIST)
    {
        sprintf(outStr, "ERROR: expected parmtyp of OCI_PTYPE_LIST, got %d\n", (int)parmtyp);
        return -1;
    }

    /* Now set the attributes of each column by getting a parameter
     * handle on each column, then setting attributes on the parameter
     * handle for the column.
     * Note that positions within a column list descriptor are 1-based.
     */
    for (i = 0, pos = 1, colp = tblp->column, fldp = tblp->field;
         i < tblp->col_num; i++, pos++, colp++, fldp++)
    {
        /* get parameter handle on the column */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIParamGet((CONST dvoid *)ctlp->colLstDesc_ctl,
            (ub4)OCI_DTYPE_PARAM, ctlp->errhp_ctl, (dvoid **)&colDesc, pos))<0)
            return -1;

        colp->id_col = i;                            /* position in column array */

        /* set external attributes on the column */
            /* column name */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
            (dvoid *)colp->name_col,(ub4)strlen((const char *)colp->name_col), (ub4)OCI_ATTR_NAME, ctlp->errhp_ctl))<0)
            return -1;

        /* column type */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
            (dvoid *)&colp->exttyp_col, (ub4)0, (ub4)OCI_ATTR_DATA_TYPE, ctlp->errhp_ctl))<0)
            return -1;

        /* max data size */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
            (dvoid *)&fldp->maxlen_fld, (ub4)0, (ub4)OCI_ATTR_DATA_SIZE, ctlp->errhp_ctl))<0)
            return -1;

      /* If column is chrdate or date, set column (input field) date mask
       * to trigger client library to check string for a valid date.
       * Note: OCIAttrSet() may be called here w/ a null ptr or null string.
       */
        if (colp->date_col)
        {
            if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                (dvoid *)colp->datemask_col, (colp->datemask_col) ? (ub4)strlen((const char *)colp->datemask_col) : 0,
                (ub4)OCI_ATTR_DATEFORMAT, ctlp->errhp_ctl))<0)
                return -1;
        }

        if (colp->prec_col)
        {
            if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                (dvoid *)&colp->prec_col, (ub4)0, (ub4)OCI_ATTR_PRECISION, ctlp->errhp_ctl))<0)
                return -1;
        }

        if (colp->scale_col)
        {
            if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                (dvoid *)&colp->scale_col, (ub4)0, (ub4)OCI_ATTR_SCALE, ctlp->errhp_ctl))<0)
                return -1;
        }

        if (colp->csid_col)
        {
            if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)colDesc, (ub4)OCI_DTYPE_PARAM,
                (dvoid *)&colp->csid_col, (ub4)0, (ub4)OCI_ATTR_CHARSET_ID, ctlp->errhp_ctl))<0)
                return -1;
        }

        /* free the parameter handle to the column descriptor */
        if( OCIDescriptorFree((dvoid *)colDesc, OCI_DTYPE_PARAM))
            return -1;
    }

    /* read back some of the attributes for purpose of illustration */
    for (i = 0, pos = 1, colp = tblp->column, fldp = tblp->field;
         i < tblp->col_num; i++, pos++, colp++, fldp++)
    {
        text *s;
        ub4   slen;
        ub4   maxdsz;
        ub2   dty;

        /* get parameter handle on the column */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIParamGet((CONST dvoid *)ctlp->colLstDesc_ctl,
            (ub4)OCI_DTYPE_PARAM, ctlp->errhp_ctl, (dvoid **)&colDesc, pos))<0)
            return -1;

        /* get column name */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((dvoid *)colDesc,
            OCI_DTYPE_PARAM, (dvoid *)&s, (ub4 *)&slen, OCI_ATTR_NAME, ctlp->errhp_ctl))<0)
            return -1;

        /* check string length */
        if (slen != (ub4)strlen((const char *)colp->name_col))
        {
            sprintf(outStr,"ERROR: bad col name len in column parameter\texpected %d, got %d",
               (int)strlen((const char *)colp->name_col), (int)slen);
            return -1;
        }

        if (strncmp((const char *)s, (const char *)colp->name_col, (size_t)slen))
        {
            sprintf(outStr,"ERROR: bad column name in column parameter\texpected %s, got %s\n",
                (char *)colp->name_col, (char *)s);
            return -1;
        }

        /* get column type */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((dvoid *)colDesc,
            OCI_DTYPE_PARAM, (dvoid *)&dty, (ub4 *)0, OCI_ATTR_DATA_TYPE, ctlp->errhp_ctl))<0)
            return -1;
        if (dty != colp->exttyp_col)
        {
            sprintf(outStr,"ERROR: bad OCI_ATTR_DATA_TYPE in col param\tColumn name %s \texpected %d, got %d",
                colp->name_col, (int)colp->exttyp_col, (int)dty);
            return -1;
        }

        /* get the max data size */
        if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((dvoid *)colDesc, OCI_DTYPE_PARAM,
            (dvoid *)&maxdsz, (ub4 *)0, OCI_ATTR_DATA_SIZE, ctlp->errhp_ctl))<0)
            return -1;

        if (maxdsz != fldp->maxlen_fld)
        {
            sprintf(outStr,"ERROR: bad OCI_ATTR_DATA_SIZE in col param\tColumn name %s \texpected %d, got %d",
                colp->name_col, (int)fldp->maxlen_fld, (int)maxdsz);
            return -1;
        }

        /* free the parameter handle to the column descriptor */
        if(OCIDescriptorFree((dvoid *)colDesc, OCI_DTYPE_PARAM))
            return -1;
    }

    /* prepare the load */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIDirPathPrepare(dpctx, ctlp->svchp_ctl, ctlp->errhp_ctl))<0)
        return -1;

    /* Allocate column array and stream handles.
     * Note that for the column array and stream handles
     * the parent handle is the direct path context.
     * Also note that Oracle errors are returned via the
     * environment handle associated with the direct path context.
     */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->dpctx_ctl, (dvoid **)&ctlp->dpca_ctl,
        (ub4)OCI_HTYPE_DIRPATH_COLUMN_ARRAY, (size_t)0, (dvoid **)0))<0)
        return -1;

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->dpctx_ctl,(dvoid **)&ctlp->dpstr_ctl,
        (ub4)OCI_HTYPE_DIRPATH_STREAM, (size_t)0, (dvoid **)0))<0)
        return -1;

    /* get number of rows in the column array just allocated */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)(ctlp->dpca_ctl), OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
        (dvoid *)(&ctlp->nrow_ctl), (ub4 *)0, OCI_ATTR_NUM_ROWS, ctlp->errhp_ctl))<0)
        return -1;

    /* get number of columns in the column array just allocated */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)(ctlp->dpca_ctl), OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
        (dvoid *)(&ctlp->ncol_ctl), (ub4 *)0, OCI_ATTR_NUM_COLS, ctlp->errhp_ctl))<0)
        return -1;


    /* allocate buffer for input records */
    if(ctlp->inbuf_ctl != NULL)
    {
        free(ctlp->inbuf_ctl);
        ctlp->inbuf_ctl = NULL;
    }
    ctlp->inbuf_ctl = (ub1 *)malloc(ctlp->nrow_ctl * maxreclen);
    if (ctlp->inbuf_ctl == (ub1 *)0)
    {
        sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
            ctlp->nrow_ctl * maxreclen, __FILE__, __LINE__);
        return -1;
    }

    /* allocate Offset-TO-Record number mapping array */
    if(ctlp->otor_ctl != NULL)
    {
        free(ctlp->otor_ctl);
        ctlp->otor_ctl = NULL;
    }
    ctlp->otor_ctl = (ub4 *)malloc(ctlp->nrow_ctl * sizeof(ub4));
    if (ctlp->otor_ctl == (ub4 *)0)
    {
        sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
            ctlp->nrow_ctl * sizeof(ub4), __FILE__, __LINE__);
        return -1;
    }

    CLEAR_PCTX(ctlp->pctx_ctl);                  /* initialize partial context */

    return 0;
}

/*------------------------------ do_convert ------------------------------*/
/*出错 返回 99*/
sword do_convert(char *outStr, struct loadctl *ctlp, ub4 startoff, ub4 rowcnt, ub4 *cvtCntp, ub2 *badcoffp)
{
  sword retval = CONVERT_SUCCESS;
  sword ocierr;
  ub2   badcol = 0;

  *cvtCntp = 0;

  if (startoff >= rowcnt)
  {
      sprintf(outStr, "DO_CONVERT: bad startoff.\tError occurred at File %s:%d",__FILE__, __LINE__);
      return 99;
  }

  if (rowcnt)
  {
    /* convert array to stream, filter out bad records */
    ocierr = OCIDirPathColArrayToStream(ctlp->dpca_ctl, ctlp->dpctx_ctl,
                                        ctlp->dpstr_ctl, ctlp->errhp_ctl,
                                        rowcnt, startoff);
    switch (ocierr)
    {
    case OCI_SUCCESS:        /* everything succesfully converted to stream */
      retval = CONVERT_SUCCESS;
      break;

    case OCI_ERROR:            /* some error, most likely a conversion error */
      /* Tell the caller that a conversion error occurred along
       * with the number of rows successfully converted (*cvtCntp).
       * Note that the caller is responsible for adjusting startoff
       * accordingly and calling us again to resume conversion of
       * the remaining rows.
       */
      retval  = CONVERT_ERROR;                         /* conversion error */
      break;

    case OCI_CONTINUE:                              /* stream buffer is full */
      /* The stream buffer could not contain all of the data in
       * the column array.
       * The client should load the converted data, and loop
       * back to convert the remaining data in the column array.
       */
      retval  = CONVERT_CONTINUE;
      break;

    case OCI_NEED_DATA:                        /* partial column encountered */
      /* Everything converted, but have a partial column.
       * Load this stream, and return to caller for next piece.
       */
      retval = CONVERT_NEED_DATA;
      break;

    default:                                 /* unexpected OCI return value! */
      sprintf(outStr, "Unexpected OCI return code.\tError occurred at File %s:%d",__FILE__, __LINE__);
      return 99;
      /* NOTREACHED */
      break;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)ctlp->dpca_ctl, OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
       (dvoid *)(cvtCntp), (ub4 *)0, OCI_ATTR_ROW_COUNT, ctlp->errhp2_ctl))<0)
       return 99;

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)ctlp->dpca_ctl, OCI_HTYPE_DIRPATH_COLUMN_ARRAY,
       (dvoid *)(&badcol), (ub4 *)0, OCI_ATTR_COL_COUNT, ctlp->errhp2_ctl))<0)
       return 99;
  }

  *badcoffp = badcol;

  return retval;
}

/*------------------------------ do_load ------------------------------*/
/*出错 返回 99*/
sword do_load(char *outStr, struct loadctl *ctlp, ub4 *loadCntp)
{
  sword ocierr;                                          /* OCI return value */
  sword retval;                           /* return value from this function */
  sword getRowCnt = FALSE;                       /* return row count if TRUE */

  if (loadCntp != (ub4 *)0)
  {
    *loadCntp = 0;
    getRowCnt = TRUE;
  }

  ocierr = OCIDirPathLoadStream(ctlp->dpctx_ctl, ctlp->dpstr_ctl,
                                ctlp->errhp_ctl);
  switch (ocierr)
  {
    case OCI_SUCCESS:
      /* all data succcesfully loaded */
      retval    = LOAD_SUCCESS;
      break;

    case OCI_ERROR:
      /* Error occurred while loading: could be a partition mapping
       * error, null constraint violation, or an out of space
       * condition.  In any case, we return the number of rows
       * processed (successfully loaded).
       */
      retval    = LOAD_ERROR;
      break;

    case OCI_NEED_DATA:
      /* Last row was not complete.
       * The caller needs to supply another piece.
       */
      retval    = LOAD_NEED_DATA;
      break;

    case OCI_NO_DATA:
      /* the stream was empty */
      retval    = LOAD_NO_DATA;
      break;

    default:
      sprintf(outStr, "Unexpected OCI return code.\tError occurred at File %s:%d", __FILE__, __LINE__);
      return 99;
      /* NOTREACHED */
      break;
  }

  if (getRowCnt)
  {
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrGet((CONST dvoid *)(ctlp->dpstr_ctl), OCI_HTYPE_DIRPATH_STREAM,
        (dvoid *)loadCntp, (ub4 *)0, OCI_ATTR_ROW_COUNT, ctlp->errhp2_ctl))<0)
        return 99;
  }

  return retval;
}


/*------------------------------ simple_load ------------------------------*/
/*
  This function reads input records from 'inputfp', parses the input
  records into fields according to the field description given by
  tblp->fld_tbl, and loads the data into the database.

  LOBs can be loaded with this function in a piecewise manner.  This
  function is written as a state machine, which cycles through the
  following states:
    RESET, GET_RECORD, FIELD_SET, DO_CONVERT, DO_LOAD, END_OF_INPUT

  The normal case of all scalar data, where multiple records fit
  entirely in memory, cycles through the following states:
    RESET, [[GET_RECORD, FIELD_SET]+, DO_CONVERT, DO_LOAD]+, RESET

  The case of loading one or more LOB columns, which do not fit entirely
  in memory, has the following state transitions:
    RESET, GET_RECORD, [FIELD_SET, DO_CONVERT, DO_LOAD]+, RESET
  Note that the second and subsequent transitions to the FIELD_SET
  state have a partial record context.

  A mapping of column array offset to input record number (otor_ctl[])
  is maintained by this function for error reporting and recovery.
 */

short simple_load(char *outStr, long *totalNum, FILE *errfp, struct loadctl *ctlp,
                  SFILE *inputfp, char *addStr, void *vFI)
{
    sword  fsetrv;                              /* return value from field_set */
    sword  cvtrv;                              /* return value from do_convert */
    sword  ldrv;                                  /* return value from do_load */
    ub4    startoff=0;                   /* starting row offset for conversion */
    ub4    nxtLoadOff=0;              /* column array offset to be loaded next */
    ub4    rowCnt=0;                /* count of rows populated in column array */
    ub4    cvtCnt=0;                                /* count of rows converted */
    ub4    lastoff=0;                  /* last row offset used in column array */
    sword  state;                               /* current state machine state */
    sword  done;                          /* set to TRUE when load is complete */
    ub4    input_recnum;                        /* current input record number */
    ub4    load_recnum;   /* record number corresponding to last record loaded */
    ub4    err_recnum;                 /* record number corresponding to error */
    text   *recp=NULL;
    ub4    cvtcontcnt;                 /* # of times CONVERT_CONTINUE returned */
    int  maxreclen = MAX_DP_RECLEN;   /*每条记录最大长度*/
    char recStr[MAX_DP_RECLEN], *p;
    int len, i, j, k, iArr[300];
    FILE_INFO *FI = (FILE_INFO *)vFI;
    char strBuf[1024];

    /* set initial state */
    input_recnum = 0;
    load_recnum  = UB4MAXVAL;
    err_recnum   = 0;
    state        = RESET;
    fsetrv       = FIELD_SET_COMPLETE;
    cvtrv        = CONVERT_SUCCESS;
    ldrv         = LOAD_SUCCESS;
    done         = FALSE;
    cvtcontcnt   = 0;

    /*取出第1条记录*/
    memset(recStr, '\0', sizeof(recStr));
    p = sgets(recStr, (int)maxreclen, inputfp);

    while (!done)
    {
      switch (state)
      {
      case RESET:    /* Reset column array and direct stream state to be empty */
      {
        startoff   = 0;             /* reset starting offset into column array */
        lastoff    = 0;                      /* last entry set of column array */
        rowCnt     = 0;                  /* count of rows partial and complete */
        cvtCnt     = 0;                             /* count of converted rows */
        nxtLoadOff = 0;

        /* Reset column array state in case a previous conversion needed
         * to be continued, or a row is expecting more data.
         */
        (void) OCIDirPathColArrayReset(ctlp->dpca_ctl, ctlp->errhp_ctl);

        /* Reset the stream state since we are starting a new stream
         * (i.e. don't want to append to existing data in the stream.)
         */
        (void) OCIDirPathStreamReset(ctlp->dpstr_ctl,  ctlp->errhp_ctl);

        state = GET_RECORD;                     /* get some more input records */
        /* FALLTHROUGH */
      }

      case GET_RECORD:
      {
        if(p==NULL)  /*到达文件尾*/
        {
          if (lastoff)
            lastoff--;
          state = END_OF_INPUT;
          break;
        }

        assert(lastoff < ctlp->nrow_ctl);                /* array bounds check */
        recp = (text *)(ctlp->inbuf_ctl + (lastoff * maxreclen));
        /* set column array offset to input record number map */

        memset((char *)recp, '\0', MAX_DP_RECLEN); /*初始化内存，20071107 加入*/        
        /*去掉字符串的换行符（10），20071110加入*/
        len = strlen(recStr);
        if(recStr[len-2] == 13)
            recStr[len-2] = '\0';
        else
            recStr[len-1] = '\0';

        
        if(0 == FI->segChar)   /*定长文件*/
        {
            sprintf((char *)recp, "%s%s", addStr, recStr);
        }
        else                      /*变长文件*/
        {
            SplitStrM(recStr, FI->segChar, &i, 300, iArr);
            j = sprintf((char *)recp, "%s", addStr);
            for(i=0; i<FI->fieldNum; i++)
            {
                if(0 != FI->field[i].format[0])
                    continue;
                k = FI->field[i].pos - 1;
                j+=sprintf((char *)recp+j, "%-*s", FI->table_def.field[i].maxlen_fld, recStr+iArr[k]);
            }
        }
        memset(recStr, '\0', MAX_DP_RECLEN);
        p = sgets(recStr, (int)maxreclen, inputfp);

        if( (p==NULL)&&(FI->tailNum>0) )  /*忽略尾记录*/
        {
          if (lastoff)
            lastoff--;
          state = END_OF_INPUT;
          break;
        }

        ctlp->otor_ctl[lastoff] = ++input_recnum;
        state = FIELD_SET;
      }

      case FIELD_SET:
      {
        /* map input data fields to DB columns, set column array entries */
        fsetrv = field_set(outStr, ctlp, &(FI->table_def), recp, lastoff);
        if(fsetrv==99)
            return -1;
        rowCnt = lastoff + 1;

        if (rowCnt == ctlp->nrow_ctl || fsetrv != FIELD_SET_COMPLETE)
        {
          /* array is full, or have a large partial column, or the
           * secondary buffer is in use by an OUTOFLINE field.
           */
          state = DO_CONVERT;
          /* FALLTHROUGH */
        }
        else
        {
          lastoff++;                             /* use next column array slot */
          state = GET_RECORD;                               /* get next record */
          break;
        }
      }

      case DO_CONVERT:
      {
        /* Either one of the following is true:
         * - the column array is full
         * - there is a large partial column
         * - the secondary buffer used by field_set() is in use
         * - previous conversion returned CONVERT_CONTINUE and
         *   now the conversion is being resumed.
         *
         * In any case, convert and load the data.
         */
        ub4    cvtBadRoff;                   /* bad row offset from conversion */
        ub2    cvtBadCoff;                /* bad column offset from conversion */

        while (startoff <= lastoff)
        {
          ub4 cvtCntPerCall = 0;   /* rows converted in one call to do_convert */

          /* note that each call to do_convert() will convert all contiguous rows
           * in the colarray until it hit a row in error while converting.
           */
          cvtrv = do_convert(outStr, ctlp, startoff, rowCnt, &cvtCntPerCall, &cvtBadCoff);
          if(cvtrv==99)
              return -1;

          cvtCnt += cvtCntPerCall; /* sum of rows converted so far in colarray */
          if (cvtrv == CONVERT_SUCCESS)
          {
            /* One or more rows converted successfully, break
             * out of the conversion loop and load the rows.
             */
            assert(cvtCntPerCall > 0);
            state = DO_LOAD;
            break;
          }
          else if (cvtrv == CONVERT_ERROR)
          {
            /* Conversion error.  Reject the bad record and
             * continue on with the next record (if any).
             * cvtBadRoff is the 0-based index of the bad row in
             * the column array.  cvtBadCoff is the 0-based index
             * of the bad column (of the bad row) in the column
             * array.
             */
            assert(cvtCntPerCall >= 0);
            cvtBadRoff = startoff + cvtCntPerCall;
            err_recnum = ctlp->otor_ctl[cvtBadRoff];    /* map to input_recnum */
            GetDBDPErrCode( outStr, (dvoid *)(ctlp->errhp_ctl) );
            /* print err msg txt */
            fprintf(errfp, "%-10d(%d:%d): %s\n",
                             (int)err_recnum, (int)err_recnum, (int)cvtBadCoff + 1, outStr);
            
            /*memset(strBuf,0,sizeof(strBuf));
            sprintf(strBuf, "%-10d(%d:%d): %s\n",
                             (int)err_recnum, (int)err_recnum, (int)cvtBadCoff + 1, outStr);
            swriteln(strBuf,strlen(strBuf),errfp);*/

           /* Check to see if the conversion error occurred on a
            * continuation of a partially loaded row.
            * If so, either (a) flush the partial row from the server, or
            * (b) mark the column as being 0 length and complete.
            * In the latter case (b), any data already loaded into the column
            * from a previous LoadStream call remains, and we can continue
            * field setting, conversion and loading with the next column.
            * Here, we implement (a), and flush the row from the server.
            */
            if (err_recnum == load_recnum)
            {
              /* Conversion error occurred on record which has been
               * partially loaded (by a previous stream).
               * XXX May be better to have an attribute of the direct path
               * XXX context which indicates that the last row loaded was
               * XXX partial.
               *
               * Flush the output pipe.  Note that on conversion error,
               * no part of the row data for the row in error makes it
               * into the stream buffer.
               * Here we flush the partial row from the server.  The
               * stream state is reset if no rows are successfully
               * converted.
               */

              /* flush partial row from server */
              (void) OCIDirPathFlushRow(ctlp->dpctx_ctl, ctlp->errhp_ctl);
            }

            if (cvtBadRoff == lastoff)
            {
              /* Conversion error occurred on the last populated slot
               * of the column array.
               * Flush the input stream of any data for this row,
               * and re-use this slot for another input record.
               */
              fieldFlush(ctlp, lastoff);
              state    = GET_RECORD;
              startoff = cvtBadRoff;              /* only convert the last row */
              rowCnt = 0;    /* already tried converting all rows in col array */
              assert(startoff <= lastoff);
              break;
            }
            else
            {
              /* Skip over bad row and continue conversion with next row.
               * We don't attempt to fill in this slot with another record.
               */
              startoff = cvtBadRoff + 1;
              assert(startoff <= lastoff);
              continue;
            }
          }
          else if (cvtrv == CONVERT_NEED_DATA)      /* partial col encountered */
          {
            /* Partial (large) column encountered, load the piece
             * and loop back up to field_set to get the rest of
             * the partial column.
             * startoff is set to the offset into the column array where
             * we need to resume conversion from, which should be the
             * last entry that we set (lastoff).
             */
            state    = DO_LOAD;

            /* Set our row position in column array to resume
             * conversion at when DO_LOAD transitions to DO_CONVERT.
             */
            assert(cvtCntPerCall >= 0);
            startoff = startoff + cvtCntPerCall;
            assert(startoff == lastoff);
            break;
          }
          else if (cvtrv == CONVERT_CONTINUE)
          {
            /* The stream buffer is full and there is more data in
             * the column array which needs to be converted.
             * Load the stream (DO_LOAD) and transition back to
             * DO_CONVERT to convert the remainder of the column array,
             * without calling the field setting function in between.
             * The sequence {DO_CONVERT, DO_LOAD} may occur many times
             * for a long row or column.
             * Note that startoff becomes the offset into the column array
             * where we need to resume conversion from.
             */
            cvtcontcnt++;
            state    = DO_LOAD;

            /* Set our row position in column array (startoff) to
             * resume conversion at when we transition from the
             * DO_LOAD state back to DO_CONVERT.
             */
            assert(cvtCntPerCall >= 0);
            startoff = startoff + cvtCntPerCall;
            assert(startoff <= lastoff);

            break;
          }
        }                                                         /* end while */
        break;
      }

      case DO_LOAD:
      {
        ub4    loadCnt;                     /* count of rows loaded by do_load */

        ldrv       = do_load(outStr, ctlp, &loadCnt);
        if(ldrv==99)
            return -1;
        nxtLoadOff = nxtLoadOff + loadCnt;

        switch (ldrv)
        {
        case LOAD_SUCCESS:
        {
          /* The stream has been loaded successfully.  What we do next
           * depends on the result of the previous conversion step.
           */
          load_recnum = ctlp->otor_ctl[nxtLoadOff - 1];
          if (cvtrv == CONVERT_SUCCESS || cvtrv == CONVERT_ERROR)
          {
            /* The column array was successfully converted (or the
             * last row was in error).
             * Fill up another array with more input records.
             */
            state = RESET;
          }
          else if (cvtrv == CONVERT_CONTINUE)
          {
            /* There is more data in column array to convert and load. */
            state    = DO_CONVERT;
            /* Reset the direct stream state so conversion starts at
             * the beginning of the stream.
             */
            (void) OCIDirPathStreamReset(ctlp->dpstr_ctl, ctlp->errhp_ctl);
          }
          else
          {
            /* Note that if the previous conversion step returned
             * CONVERT_NEED_DATA then the load step would have returned
             * LOAD_NEED_DATA too (not LOAD_SUCCESS).
             */
              sprintf(outStr, "DO_LOAD: unexpected cvtrv.\tError occurred at File %s:%d", __FILE__, __LINE__);
              return -1;
          }
          break;
        }

        case LOAD_ERROR:
        {
          sb4  oraerr;
          ub4  badRowOff;

          badRowOff   = nxtLoadOff;
          nxtLoadOff += 1;                              /* account for bad row */
          err_recnum      = ctlp->otor_ctl[badRowOff];  /* map to input_recnum */
          /* print err msg txt */
          oraerr = GetDBDPErrCode( outStr, (dvoid *)(ctlp->errhp_ctl) );
          fprintf(errfp, "%-10ld(%ld): %s\n", (long)err_recnum, (long)err_recnum, outStr);

         /* memset(strBuf,0,sizeof(strBuf));
          sprintf(strBuf, "%-10ld(%ld): %s\n", (long)err_recnum, (long)err_recnum, outStr);
            
          swriteln(strBuf,strlen(strBuf),errfp);*/
          
          /* On a load error, all rows up to the row in error are loaded.
           * account for that here by setting load_recnum only when some
           * rows have been loaded.
           */
          if (loadCnt != 0)
            load_recnum = err_recnum - 1;

          if (oraerr == OER(600))
          {
              sprintf(outStr, "DO_LOAD:LOAD_ERROR: server internal error.\tError occurred at File %s:%d", __FILE__, __LINE__);
              return -1;
          }

          if (err_recnum == input_recnum)
          {
            /* Error occurred on last input row, which may or may not
             * be in a partial state. Flush any remaining input for
             * the bad row.
             */
            fieldFlush(ctlp, badRowOff);
          }

          if (err_recnum == load_recnum)
          {
            /* Server has part of this row already, flush it */
            (void) OCIDirPathFlushRow(ctlp->dpctx_ctl, ctlp->errhp_ctl);
          }

          if (badRowOff == lastoff)
          {
            /* Error occurred on the last entry in the column array,
             * go process more input records and set up another array.
             */
            state = RESET;
          }
          else
          {
            /* Otherwise, continue loading this stream.  Note that the
             * stream positions itself to the next row on error.
             */
            state    = DO_LOAD;
          }
          break;
        }

        case LOAD_NEED_DATA:
        {
          load_recnum = ctlp->otor_ctl[nxtLoadOff];
          if (cvtrv == CONVERT_NEED_DATA)
            state = FIELD_SET;                         /* need more input data */
          else if (cvtrv == CONVERT_CONTINUE)
            state = DO_CONVERT;   /* have input data, continue with conversion */
          else
          {
              sprintf(outStr, "DO_LOAD:LOAD_NEED_DATA: unexpected cvtrv.\tError occurred at File %s:%d", __FILE__, __LINE__);
              return -1;
          }
          /* Reset the direct stream state so conversion starts at
           * the beginning of the stream.
           */
          (void) OCIDirPathStreamReset(ctlp->dpstr_ctl, ctlp->errhp_ctl);
          break;
        }

        case LOAD_NO_DATA:
        {
          /* Attempt to either load an empty stream, or a stream
           * which has been completely processed.
           */
          if (cvtrv == CONVERT_CONTINUE)
          {
            /* Reset stream state so we convert into an empty stream buffer. */
            (void) OCIDirPathStreamReset(ctlp->dpstr_ctl,  ctlp->errhp_ctl);
            state = DO_CONVERT;           /* convert remainder of column array */
          }
          else
            state = RESET;                      /* get some more input records */
          break;
        }

        default:
          sprintf(outStr, "DO_LOAD: unexpected return value.\tError occurred at File %s:%d", __FILE__, __LINE__);
          return -1;
          break;
        }
        break;
      }

      case END_OF_INPUT:
      {
        if (cvtCnt)
          state = DO_LOAD; /* deal with data already converted, but not loaded */
        else if (rowCnt)
          state = DO_CONVERT; /* deal with a partially populated column array */
        else
          done = TRUE;
        break;
      }

      default:
        sprintf(outStr, "SIMPLE_LOAD: unexpected state.\tError occurred at File %s:%d", __FILE__, __LINE__);
        return -1;
        break;
      }                                                  /* end switch (state) */
    }

  *totalNum = (long)input_recnum;
  return 0;
}

short finish_load(char *outStr, struct loadctl *ctlp)
{
    /* Execute load finishing logic without freeing server data structures
     * related to the load.
     */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIDirPathDataSave(ctlp->dpctx_ctl,
        ctlp->errhp_ctl, (ub4)OCI_DIRPATH_DATASAVE_FINISH))<0)
        return -1;

    /* free up server data structures for the load. */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIDirPathFinish(ctlp->dpctx_ctl, ctlp->errhp_ctl))<0)
        return -1;

    if (ctlp->dpca_ctl)
    {
        OCIHandleFree((dvoid *)ctlp->dpca_ctl, OCI_HTYPE_DIRPATH_COLUMN_ARRAY);
        ctlp->dpca_ctl=NULL;
    }

    if (ctlp->dpstr_ctl)
    {
        OCIHandleFree((dvoid *)ctlp->dpstr_ctl, OCI_HTYPE_DIRPATH_STREAM);
        ctlp->dpstr_ctl=NULL;
    }

    if (ctlp->dpctx_ctl)
    {
        OCIHandleFree((dvoid *)ctlp->dpctx_ctl, OCI_HTYPE_DIRPATH_CTX);
        ctlp->dpctx_ctl=NULL;
    }

    if (ctlp->colLstDesc_ctl)
    {
        OCIHandleFree((dvoid *)(ctlp->colLstDesc_ctl), OCI_ATTR_LIST_COLUMNS);
        ctlp->colLstDesc_ctl = NULL;
    }

    if(ctlp->inbuf_ctl != NULL)
    {
        free(ctlp->inbuf_ctl);
        ctlp->inbuf_ctl = NULL;
    }

    if(ctlp->otor_ctl != NULL)
    {
        free(ctlp->otor_ctl);
        ctlp->otor_ctl = NULL;
    }

    return 0;
}

/* cleanup
 *   Free up handles and exit with the supplied exit status code.
 */
void cleanup(struct loadctl *ctlp, sb4 ex_status)
{
    /* Free the column array and stream handles if they have been
     * allocated.  We don't need to do this since freeing the direct
     * path context will free the heap which these child handles have
     * been allocated from.  I'm doing this just to exercise the code
     * path to free these handles.
     */
    if (ctlp->dpca_ctl)
        OCIHandleFree((dvoid *)ctlp->dpca_ctl, OCI_HTYPE_DIRPATH_COLUMN_ARRAY);

    if (ctlp->dpstr_ctl)
        OCIHandleFree((dvoid *)ctlp->dpstr_ctl, OCI_HTYPE_DIRPATH_STREAM);

    if (ctlp->dpctx_ctl)
        OCIHandleFree((dvoid *)ctlp->dpctx_ctl, OCI_HTYPE_DIRPATH_CTX);

    if (ctlp->errhp_ctl && ctlp->srvhp_ctl)
    {
        (void) OCIServerDetach(ctlp->srvhp_ctl, ctlp->errhp_ctl, OCI_DEFAULT );
        OCIHandleFree((dvoid *)ctlp->srvhp_ctl, OCI_HTYPE_SERVER);
    }

    if (ctlp->svchp_ctl)
        (void) OCIHandleFree((dvoid *) ctlp->svchp_ctl, OCI_HTYPE_SVCCTX);
    if (ctlp->errhp_ctl)
        (void) OCIHandleFree((dvoid *) ctlp->errhp_ctl, OCI_HTYPE_ERROR);

    return;
}

short Connect_DBDP(char *outStr, struct loadctl *ctlp,
                    char *username, char *password, char *server)
{
    memset((dvoid *)ctlp, 0, sizeof(struct loadctl));

    /* set up OCI environment and connect to the ORACLE server */
    if(OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
                            (dvoid * (*)(dvoid *, size_t)) 0,
                            (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                            (void (*)(dvoid *, dvoid *)) 0 ))
    {
        sprintf(outStr, "FAILED:OCIInitialize");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIEnvInit((OCIEnv **)&ctlp->envhp_ctl,
        OCI_DEFAULT, (size_t)0,(dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIEnvInit");
        return -1;
    }

    /* allocate error handles */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->errhp_ctl, OCI_HTYPE_ERROR,(size_t)0, (dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Error Handle");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->errhp2_ctl, OCI_HTYPE_ERROR,(size_t)0, (dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Error2 Handle");
        return -1;
    }

    /* server contexts */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->srvhp_ctl, OCI_HTYPE_SERVER,(size_t)0, (dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Srv Handle");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->svchp_ctl, OCI_HTYPE_SVCCTX,(size_t)0, (dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Svc Handle");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIServerAttach(ctlp->srvhp_ctl, ctlp->errhp_ctl,
              (OraText *)server, (sb4)strlen((const char *)server), OCI_DEFAULT))<0)
    {
        return -1;
    }

    /* set attribute server context in the service context */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)ctlp->svchp_ctl, OCI_HTYPE_SVCCTX,
        (dvoid *)ctlp->srvhp_ctl, (ub4)0, OCI_ATTR_SERVER,ctlp->errhp_ctl))<0)
    {
        sprintf(outStr, "FAILED:OCIAttrSet on OCI_ATTR_SERVER");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIHandleAlloc((dvoid *)ctlp->envhp_ctl,
        (dvoid **)&ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,(size_t)0, (dvoid **)0))<0)
    {
        sprintf(outStr, "FAILED:OCIAttrSet on OCI_HTYPE_SESSION");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,
        (dvoid *)username,(ub4)strlen((char *)username),(ub4)OCI_ATTR_USERNAME, ctlp->errhp_ctl))<0)
    {
        sprintf(outStr, "FAILED:OCIAttrSet on USERID");
        return -1;
    }

    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)ctlp->authp_ctl, (ub4)OCI_HTYPE_SESSION,
        (dvoid *)password,(ub4)strlen((char *)password),(ub4)OCI_ATTR_PASSWORD, ctlp->errhp_ctl))<0)
    {
        sprintf(outStr, "FAILED:OCIAttrSet on PASSWD");
        return -1;
    }

    /* begin a session */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCISessionBegin(ctlp->svchp_ctl,
        ctlp->errhp_ctl, ctlp->authp_ctl, OCI_CRED_RDBMS, (ub4)OCI_DEFAULT) ) <0 )
    {
        return -1;
    }

    /* set authentication context into service context */
    if(Check_Err1(outStr, ctlp->errhp_ctl, OCIAttrSet((dvoid *)ctlp->svchp_ctl, (ub4)OCI_HTYPE_SVCCTX,
        (dvoid *)ctlp->authp_ctl, (ub4)0, (ub4)OCI_ATTR_SESSION, ctlp->errhp_ctl))<0)
    {
        sprintf(outStr, "FAILED:OCIAttrSet on session" );
        return -1;
    }

    return 0;
}
