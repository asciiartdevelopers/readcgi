/* read2ch.h */

#ifndef READ2CH_H__
#define READ2CH_H__

#define	CGINAME	"read.cgi"
#define	CGIVER	"read.cgi ver5.26+ (01/10/21-)"
#define	RES_YELLOW	900
#define	RES_REDZONE	950
#define	RES_RED		1000
#define	RES_IMODE	10
#define	RES_NORMAL	100
#define	MAX_FILESIZE	(8192 * 64)
#define	SIZE_BUF	0xa000

/* �Ѹ��Ǥˤ���(r2chhtml.h �ǤϤʤ� r2chhtml_en.h �����󥯥롼�ɤ����) */ 
/* #define ENGLISH */

#define CONFIG_TIMEZONE "JST-9"
#define	LIMIT_PM	22
#define	LIMIT_AM	3

#define DAT_DIR "../%.256s/dat/"
#define TEMP_DIR "../%.50s/temp/"
#define KAKO_DIR "../%.50s/kako/"

/* ���������Ӥˡ�>>000 �����Υ쥹�ؤΥ�󥯤������� */
#define LINKTAGCUT (1)

/* always LINKTAGCUT */
/* #define NORMAL_TAGCUT */

/* ���ֺǸ�ˡ��ֹ��������쥹��ɽ���פΥ�󥯤�ɽ������ */
#define RELOADLINK

/* nofirst �� n �ʤɡ�û�̷��ǻؼ��Ǥ���褦�ˤ��� */
#define GSTR2

/* teri �����פΥ����ɤ߹��� */
#define TYPE_TERI

/* �񤭹���ľ��Υ���ɤ��ɻߤ��� */
#define PREVENTRELOAD

/* �嵭�� PREVENTRELOAD ����ꤷ���Ȥ��ˡ���������Ƥ��Ƥ� 304 ���֤��ÿ� */
#define FORCE_304_TIME 60

/* gzip ����Ѥ��ư��̤��� */
#define GZIP

/* zlib ����Ѥ��ư��̤��� */
#define ZLIB

/* raw=xxx.yyy��ͭ���ˤ��롣 xxx=�ǽ��쥹�ֹ�, yyy=���ΤȤ��Υ�������
 * ���Ϥΰ���ܤϥ��ơ�����:
 *	[+OK size/maxK]
 *        2���ܰʹߤ�xxx�ְʹߤκ�ʬ�ǡ�����size�Ϻ�ʬ�Υǡ�����������
 *        max��MAX_FILESIZE/1024�� 
 *	[-INCR size/maxK] (yyy �����פ��Ƥ��ʤ��ä����˵�����; ���ܡ���β�ǽ��?)
 *        2���ܰʹߤϥ���Υǡ���������size�ϥǡ�����������
 *        max��MAX_FILESIZE/1024�� 
 *	[-ERR string]
 *        �ʤ󤫥��顼����������string�ϥ��顼�����ơ�
 *          "���Υ���å��礭�����ޤ���"
 *          "�������or����åɤʤ��Ǥ���"
 *          "�����Ҹˤ�ȯ�� %s" %s�����Хѥ���.dat�ΰ��֤�ɽ����
 *          "html���Ԥ�"
 *          "����γ��ݤ˼��Ԥ��ޤ�����"
 *          "Ĵ���档����"
 *          "�ʤ���Ĵ�Ǥ���"
 *  �ꥯ�����Ȥ�If-Modified-Since��̵����
 *  �����Ѥߥ�������dat�Υե����륵���������פ�����硢�󰵽̤�
 *  [+OK 0/maxK]
 *  �ΰ�Ԥ������֤ꡢLastModified,Content-Length,Cotent-Encoding���ϴޤޤ�ʤ�
 */
#define RAWOUT

/* raw=0.0�Υꥯ�����Ȼ���st=nn, to=nn, ls=nn�򻲾Ȥ���
 * ��ͭ���ʾ��ˤϥ��ơ��������ʲ��Τ褦���Ѥ��
 * [+PARCIAL size/maxK]
 * ���ޤ����ơ������Ԥˡ��ʲ��ξ���TAB���ڤ�ǲä����롣
 * Range:n1-n2/n3
 *     �����Х��Ȥΰ��־���
 *     HTTP �쥹�ݥ󥹥إå���Content-Range: bytes �ʲ���Ʊ����Ρ�
 * Res:n1-n2/n3
 *     �����쥹�Υ쥹�ֹ����
 *     Range�Ȱۤʤꡢ1����Ϥޤ�Ǹ�ޤ����ä�����n2==n3�Ȥʤ롣
 * ������¾���ʲ��ξ��󤬴ޤޤ��褦�ˤʤ��ǽ�������롣
 * Status:Stopped
 *     ����åɥ��ȥåפ������ä���Ƚ�ꤵ�줿��硣
 * Location:temp/
 *     dat��html�����ԤäƤ�����֤β����Ǥ���(�񤭤���ʤ�)
 * Location:kako/
 *     dat��html������Ƥ�������Ǥ���(�񤭤���ʤ�)
 */
#define	RAWOUT_PARTIAL

/* raw=0.0��bbs��key�����ꤵ��Ƥ��ʤ�����
 * dat=board/key.size�����Ǥ�ʣ���Υꥯ�����Ȥ�ͭ���ˤ���
 * �����դ���ΤϺ�ʬ�Τߤǡ�size��0�ξ���
 * +OK 0/maxK
 * �ιԤ��֤���
 * Ʊ���Ĥ�Ϣ³�������board/���ά��ǽ��
 * ���ܡ�������ֿ���Katjusha_DLL_REPLY���臘��
 *
 * �ֿ��Υ��ơ������Ԥ˰ʲ��ξ����ɲä�����礬���롣
 * Request:board/key.size
 *     �ġ��Υꥯ�����Ȥ򤽤Τޤ��֤���
 * LastModify:time
 *     Last-Modified:���֤��٤��ͤ�time()�ͤΤޤ��֤���
 *
 * �ޤ���-ERR ��
 * -ERR 200 �������or����åɡ�����
 * ���Ȥ����֤���롣
 */
#define	RAWOUT_MULTI
#define	RAWOUT_MULTI_MAX	50

/* mmap(2) ����ѡ�
   �񸻤γ������Ѷ�Ū�˥��ܤꤿ���� */
#define USE_MMAP

/* munmap(2)��������Ū�ʻ񸻳��� */
/* #undef EXPLICIT_RELEASE */

/* read.cgi¦�ˤ��index�μ���(experimental) */
/* #define USE_INDEX */


/** Header custumization **/

/* �������ɤ�פ�Ĥ��� */
/* #define ALL_ANCHOR ���ON */

/* page�� top�ˡ�CHUNK_NUM�֤��Ȥ˶��ڤä��쥹�ؤ� anchor��Ĥ���
 */
#define CHUNK_ANCHOR
#define CHUNK_NUM 100

/* CUUNK_ANCHOR���ѻ���>>nn��CHUNK���ȤΥ�󥯤ˤ��� */
/* #define USE_CHUNK_LINK */

/* �ֺǿ��쥹 LATEST_NUM�פ�Ĥ��� */
/* #define LATEST_ANCHOR ���ON */
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

/* �����SETTING�ե�����ǤϤʤ���SPECIAL_SETTING�����Ѥ��� */
#define USE_INTERNAL_SETTINGS
#define SPECIAL_SETTING \
	{ "saku", "LINKTAGCUT=0" "\n" }, \
/*	{ "dancesite", "FORCE_304_TIME=3" "\n" }, */

/* check server use mod_gzip? */
#define CHECK_MOD_GZIP

/* "̾����"����ɽ���ˤ��� */ 
#define CUT_NAME_STRING

/* "�������"����ɽ���ˤ��� */ 
#define CUT_DATE_STRING

/* <a name=...>��Ĥ��롣�����Ĥ��ʤ��Ȥ���CHUNK_ANCHOR��̵���ˤ����ۤ����ɤ� */ 
/* #define CREATE_NAME_ANCHOR  */

/* nofirst=true��û�̷��ˤ��롣bbs.cgi�ν��Ϥ�Ʊ�������˾�ޤ��� */
/* #define NO_FIRST "&n=t" */

/* ���礭�����ޤ��פηٹ��Ф��ޤǤΥ�����(KBñ��)
   MAX_FILESIZE - CAUTION_FILESIZE*1024 ��Ķ������ٹ�
   ���줬̤����ʤ顢�ٹ�ϽФ��ʤ� */
#define CAUTION_FILESIZE 16
/* �ǥХå���
#define MAX_FILESIZE_BUSY (MAX_FILESIZE - 64 * 1024) */

/* ����åɥ��ȥåװ����ˤ��륵����(KBñ��)
   MAX_FILESIZE - WRITESTOP_FILESIZE*1024 ��Ķ������FORM����Ф��ʤ�
   ���줬̤����ʤ顢�礭���Ƥ⥹��åɥ��ȥåװ����Ϥ��ʤ� */
#define WRITESTOP_FILESIZE	5

/* NN4.x��gzip���̻���Content-Length�������
  Last-Modified��̵���ˤʤ�������к� */
#define NN4_LM_WORKAROUND

/* select form������ CHUNKED_ANCHOR��ɽ������ */
/* �ַǼ��Ĥ����ס֥쥹�������סֺǿ��쥹�פȤ����줬���Ƥ��ʤ� */
/* #define��CHUNKED_ANCHOR_WITH_FORM */

/* 1- 51- ...�����Υ�󥯤�岼��ʬ����ɽ������ */
/* #define SEPARATE_CHUNK_ANCHOR */

/* ��50 ��50 ��岼�ˤĤ��롣50=CHUNK_NUM */
#define PREV_NEXT_ANCHOR

/* �����塼����ˤ�륢����������䤹�� */
#define Katjusha_Beta_kisei

/* �����������gzip��DLL�Υꥯ�����Ȥ˱�������
 * raw=.nnn�����Υꥯ�����Ȥ�����դ���
 * ���ܡ������ꤵ����硢
 * [-ERR �ɤ����Ǥ��ܡ��󤬤��ä��ߤ����Ǥ���]
 * ���֤�����ʸ�κ����ϹԤ�ʤ���
 */
#define Katjusha_DLL_REPLY

/* ̾������ɽ����CSS��Ȥä�ɽ�� */
/* #define USE_CSS */

/* ���������Ӥˡ�>>nn�����Υ�󥯤��黲�Ȥ��줿����
   ����html����;ʬ�ʥ������ʤ� */
#define	REFERDRES_SIMPLE /**/

/* ���������Ӥ˥쥹��ʸ���URL���󥯲����ʤ��褦�ˤ��� */
/* #define NO_LINK_URL_BUSY */

/* ETag����Ϥ��� */
/* #define PUT_ETAG */

/* dat�����������read.cgi���ɤ� */
/* #define READ_KAKO */
/* #define READ_TEMP */
/* #define AUTO_KAKO */

/* READ_KAKO�ǲ�����ͶƳ����Ȥ����ϰϻ����Ѿ����� */
#define READ_KAKO_THROUGH

/* AUTO_KAKO�ǲ��������/�����Ǥ������¤�����
  0 = RAW�⡼�ɻ��Τߡ�temp/,kako/�ɤ����dat�������ǽ
  1 = RAW�⡼�ɰʳ��ϡ�temp/�Τ߱�����ǽ
  2 = RAW�⡼�ɰʳ��Ǥ⡢temp/,kako/�ɤ���������ǽ  */
#define	AUTO_KAKO_MODE	0

/* ��������¹Ի���Ƚ�̤��롣READ_KAKO/AUTO_KAKO�� */
/* #define AUTO_LOGTYPE */

/* ̾���������᡼��������<br>���������쥹�����ζ����
(��ǽ�ʤ�)���ƺ������ */
#define CUT_TAIL_BLANK

/* '<',"http://"��ľ����ʸ����Ĵ�١�ɬ�פʤ�ж����ä��롣 */
/* #define STRICT_ILLEGAL_CHECK */

#endif /* READ2CH_H__ */
