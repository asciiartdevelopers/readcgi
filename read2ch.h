/* read2ch.h */

#ifndef READ2CH_H__
#define READ2CH_H__

#define	CGINAME	"read.cgi"
#define	CGIVER	"read.cgi ver14 (01/08/28)"
#define	RES_YELLOW	900
#define	RES_REDZONE	950
#define	RES_RED		1000
#define	RES_IMODE	10
#define	RES_NORMAL	100
#define	MAX_FILESIZE	(8192 * 64)
#define	SIZE_BUF	0xa000

#define	LIMIT_PM	23
#define	LIMIT_AM	3

/* ���������Ӥˡ�>>000 �����Υ쥹�ؤΥ�󥯤������� */
#define LINKTAGCUT (1)

/* ���ֺǸ�ˡ��ֹ��������쥹��ɽ���פΥ�󥯤�ɽ������ */
#define RELOADLINK

/* nofirst �� n �ʤɡ�û�̷��ǻؼ��Ǥ���褦�ˤ��� */
#define GSTR2

/* teri �����פΥ����ɤ߹��� */
#define TYPE_TERI

/* PATH_INFO��������뤳�Ȥˤ�ꡢ
   read.cgi/board/nnnnnn/?query
   �Υꥯ�����Ȥ�����Ǥ���褦�ˤʤ� */
#define USE_PATH

/* �񤭹���ľ��Υ���ɤ��ɻߤ��� */
#define PREVENTRELOAD

/* �嵭�� PREVENTRELOAD ����ꤷ���Ȥ��ˡ���������Ƥ��Ƥ� 304 ���֤��ÿ� */
#define FORCE_304_TIME 60

/* gzip ����Ѥ��ư��̤��� */
#define GZIP

/* zlib ����Ѥ��ư��̤��� */
#define ZLIB

/* raw=xxx.yyy��ͭ���ˤ��롣 xxx=�ǽ��쥹�ֹ�, yyy=���ΤȤ��Υ�������
 * ����ܤϥ��ơ�����:
 *	[+OK] �ξ��Ϻ�ʬ�Τߤ��������롣
 *	[-INCR] (Incorrect)�ξ��Ϥ��٤ƤΥǡ������������롣
 *	[-ERR (�ƥ�����)]�ξ��Ϥʤ󤫥��顼����������
 */
#define RAWOUT

/* mmap(2) ����ѡ�
   �񸻤γ������Ѷ�Ū�˥��ܤꤿ���� */
#define USE_MMAP

/* munmap(2)��������Ū�ʻ񸻳��� */
/* #undef EXPLICIT_RELEASE */

/* read.cgi¦�ˤ��index�μ���(experimental) */
/* #define USE_INDEX */


/** Header custumization **/

/* �������ɤ�פ�Ĥ��� */
#undef ALL_ANCHOR 

/* page�� top�ˡ�CHUNK_NUM�֤��Ȥ˶��ڤä��쥹�ؤ� anchor��Ĥ���
 */
#define CHUNK_ANCHOR
#define CHUNK_NUM 50

/* �ֺǿ��쥹 LATEST_NUM�פ�Ĥ��� */
#define LATEST_ANCHOR
#define LATEST_NUM 50


/* sage�쥹�ΤȤ���̾���������ˤ��ʤ� */
/* #define SAGE_IS_PLAIN */

/* ��������������Ƚ��ϥѥ�᡼��
   ����ʤ�Фɤ�����������Ǥ���? */
#define N_INDEX_DIGESTS	10
#define N_INDEX_THREADS	30

/* ��������꤬�񤤤Ƥ���ե����롢�Ȥ����� */
#define	USE_SETTING_FILE
#define	SETTING_FILE_NAME	"SETTING_R.TXT"

/* check server use mod_gzip? */
#define CHECK_MOD_GZIP

/* "�������"����ɽ���ˤ��� */ 
#define CUT_DATE_STRING

/* <a name=...>��Ĥ��롣�����Ĥ��ʤ��Ȥ���CHUNK_ANCHOR��̵���ˤ����ۤ����ɤ� */ 
#define CREATE_NAME_ANCHOR 

/* nofirst=true��û�̷��ˤ��롣bbs.cgi�ν��Ϥ�Ʊ�������˾�ޤ��� */
/* #define NO_FIRST "&n=t" */

/* ���礭�����ޤ��פηٹ��Ф��ޤǤΥ�����(KBñ��) 
��MAX_FILESIZE - CAUTION_FILESIZE*1024 ��Ķ������ٹ� 
�����줬̤����ʤ顢�ٹ�ϽФ��ʤ� */ 
#define CAUTION_FILESIZE 16 
/* �ǥХå��� 
#define MAX_FILESIZE_BUSY (MAX_FILESIZE - 64 * 1024) */ 

/* NN4.x��gzip���̻���Content-Length�������
  Last-Modified��̵���ˤʤ�������к� */
#define NN4_LM_WORKAROUND

#endif /* READ2CH_H__ */
