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
#define CUTRESLINK
#define LINKTAGCUT (1)

/* ���ֺǸ�ˡ��ֹ��������쥹��ɽ���פΥ�󥯤�ɽ������ */
#define RELOADLINK

/* Last-Modified ���ɲä��� */
#define LASTMOD

/* Last-Modified ��ʻ���ơ�Expires: ����Ϥ��Ƥߤ� */
/* #define EXPIRES */

/* ��Ŭ�����줿 BadAccess ����Ѥ��� */
#define NEWBA

/* nofirst �� n �ʤɡ�û�̷��ǻؼ��Ǥ���褦�ˤ��� */
#define GSTR2

/* teri �����פΥ����ɤ߹��� */
#define TYPE_TERI

/* PATH_INFO��������뤳�Ȥˤ�ꡢ
   read.cgi/board/nnnnnn/?query
   �Υꥯ�����Ȥ�����Ǥ���褦�ˤʤ� */
#define USE_PATH

/* Cookie �ˤ��̾����E-mail �ե�����ɤν���ͤ������ߤ� CGI ¦�ǹԤ�
 * Last-Modified�ղäˤ�ꡢproxy�ǥ���å��夵�줿���˳Ƽ�����Թ�
 * (�ǰ��ξ�硢����åס��ȥ�åפΥѥ�ϳ��)��ȯ�����뤿������ԲĤ�
 */
/* #define COOKIE */

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

/* if index2.cgi exists, return link use index2.cgi */
/* #define USE_INDEX2CGI */

/* check server use mod_gzip? */
#define CHECK_MOD_GZIP

/* rewrite_href2�����褵���������� */
/* #define REWRITE_HREF2 */

/* "�������"����ɽ���ˤ��� */ 
#define CUT_DATE_STRING 

#endif /* READ2CH_H__ */
