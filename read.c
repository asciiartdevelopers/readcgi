#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<time.h>
#include	<unistd.h>
#ifdef HAVE_READ2CH_H
# include	"read2ch.h"
#endif
#ifdef USE_MMAP
#include	<sys/mman.h>
#endif

#ifdef USE_INDEX
#include "datindex.h"
#include "digest.h"
#endif
#include "read.h"

#ifdef ZLIB
# ifndef GZIP
#  define GZIP			/* gzip�R���̃R�[�h���g�p����̂� */
# endif
#endif
#if	defined(ZLIB) || defined(PUT_ETAG)
#include	<zlib.h>
#endif

#if defined(GZIP) && !defined(ZLIB)
#include        <sys/wait.h>
static int pid;
#endif

#ifdef ENGLISH
#include	"r2chhtml_en.h"
#else
#include	"r2chhtml.h"
#endif

#ifdef PREVENTRELOAD
# ifndef FORCE_304_TIME
#  define FORCE_304_TIME  30    /* �b�Ŏw�� */
# endif
# include        "util_date.h" /* from Apache 1.3.20 */
#endif

#if	('\xFF' != 0xFF)
#error	-funsigned-char required.
	/* ���̃V�X�e���ł́A-funsigned-char��v������ */
#endif

#if	(defined(CHUNK_ANCHOR) && CHUNK_NUM > RES_NORMAL) 
# error "Too large CHUNK_NUM!!"
#endif

/* CHUNK_ANCHOR �̃R�[�h�Ɉˑ����Ă��� */
#if defined(SEPARATE_CHUNK_ANCHOR) && !defined(CHUNK_ANCHOR)
#error	SEPARATE_CHUNK_ANCHOR require CHUNK_ANCHOR
#endif

#if !defined(READ_KAKO)
# undef READ_TEMP
#endif
#if	defined(RAWOUT_MULTI) && !(defined(RAWOUT) && defined(Katjusha_DLL_REPLY))
# undef RAWOUT_MULTI
#endif

int zz_head_request; /* !0 = HEAD request */
char const *zz_remote_addr;
char const *zz_remote_host;
char const *zz_http_referer;
char const *zz_server_name;
char const *zz_script_name;
int need_basehref;

char const *zz_path_info;
/* 0 �̂Ƃ��́Apath�͓K�p����Ă��Ȃ�
   read.cgi/tech/998845501/ �̂Ƃ��́A3�ɂȂ� */
int path_depth;
char const *zz_query_string;
char *zz_temp;
char const *zz_http_user_agent;
char const *zz_http_language;
#if	defined(GZIP) || defined(RAWOUT)
enum compress_type_t {
	compress_none,
	compress_gzip,
	compress_x_gzip,
} gzip_flag;
#endif
#ifdef	GZIP
char *content_encoding[] = {
	"",
	"Content-Encoding: gzip" "\n",
	"Content-Encoding: x-gzip" "\n",
};
#endif
#ifdef CHECK_MOD_GZIP
int mod_gzip_exist;
#endif

char const *zz_http_if_modified_since;
time_t zz_fileLastmod;
char lastmod_str[1024];

#ifdef USE_MMAP
static int zz_mmap_fd;
static size_t zz_mmap_size;
static int zz_mmap_mode = MAP_SHARED;	/* MAP_PRIVATE */
#endif

char zz_bs[1024];
char zz_ky[1024];
char zz_ls[1024];
char zz_st[1024];
char zz_to[1024];
char zz_nf[1024];
char zz_im[1024];
char zz_parent_link[128];
char zz_cgi_path[128];
long nn_ky;	/* zz_ky�𐔎��ɂ������� */
#ifdef RAWOUT
char zz_rw[1024];
#endif
#ifdef READ_KAKO
char read_kako[256] = "";
#endif
#ifdef	AUTO_KAKO
int zz_dat_where;	/* 0: dat/  1: temp/  2: kako/ */
#endif
#ifdef	AUTO_LOGTYPE
int zz_log_type;	/* 0: non-TYPE_TERI  1: TYPE_TERI */
#endif
int nn_st, nn_to, nn_ls;

#define MAX_RANGE 20 
struct range {
	int count;
	struct range_elem { 
		int from, to; 
	} array[MAX_RANGE]; 
} nn_range = {0};

char *BigBuffer = NULL;
char const *BigLine[RES_RED + 2];
char zz_title[256];

#define is_imode() (*zz_im == 't')
#define is_nofirst() (*zz_nf == 't')
#define is_head() (zz_head_request != 0)

#define set_imode_true()	(*zz_im = 't')
#define set_nofirst_true()	(*zz_nf = 't')
#define set_nofirst_false()	(*zz_nf = '\0')

char *KARA = "";
int zz_fileSize = 0;
int lineMax = -1;
int out_resN = 0;

#if defined(SEPARATE_CHUNK_ANCHOR) || defined(PREV_NEXT_ANCHOR)
int first_line = 1;
int last_line = 1;
#endif

time_t t_now;
struct tm tm_now;
long currentTime;
int isbusytime = 0;

char const *LastChar(char const *src, char c);
char const *GetString(char const *line, char *dst, size_t dst_size, char const *tgt);
time_t getFileLastmod(char const *file);
int print_error(enum html_error_t errorcode, int isshowcode, const char *extinfo);
static void create_fname(char *fname, const char *bbs, const char *key);
void html_foot_im(int,int);
void html_head(int level, char const *title, int line);
static void html_foot(int level, int line,int);
void kako_dirname(char *buf, const char *key);
int getLineMax(void);
int IsBusy2ch(void);
int getFileSize(char const *file);
static int isprinted(int lineNo);
#ifdef RELOADLINK
void html_reload(int);
#endif
#ifdef RAWOUT
int rawmode;
int raw_lastnum, raw_lastsize; /* client�������Ă���f�[�^�̔ԍ��ƃT�C�Y */
#ifdef	Katjusha_DLL_REPLY
int zz_katjusha_raw;
#endif
#endif
#ifdef PREV_NEXT_ANCHOR
int need_tail_comment = 0;
#endif

#ifdef ZLIB
/*  zlib�Ή� */
gzFile pStdout; /*  = (gzFile) stdout; */
zz_printf_t pPrintf = (zz_printf_t) fprintf;

/* zlib�Ǝ����� */
extern int gz_getdata(char **buf);
/*
 * gzdopen(0,"w")��memory�Ɉ��k�������ʂ��󂯎��
 * len = gz_getdata(&data);
 *   int   len  : ���k���byte��
 *   char *data : ���k��̃f�[�^�A�g�p��free()���ׂ���
 */
#endif

#ifdef	USE_SETTING_FILE
/*
	SETTING_R.TXT��
	---
	FORCE_304_TIME=30
	LIMIT_PM=23
	RES_NORMAL=50
	MAX_FILESIZE=512
	LINKTAGCUT=0
	---
	�ȂǁB��s�B
	#��;����J�n�̓R�����g�E�E�E�Ƃ������A
	=���Ȃ�������A�}�b�`���Ȃ������肵���疳��
	�Ō�̍s�ɉ��s�������ĂȂ�������A��������������(�o�O�Ə����Ďd�l�Ɠǂ�)
	
	RES_YELLOW-RES_NORMAL�܂ł́A#define�̂܂܂ł��������B
*/
struct {
	int	Res_Yellow;
	int Res_RedZone;
	int	Res_Imode;
	int Res_Normal;
	int Max_FileSize;	/*	���������AKByte�P��	*/
	int Limit_PM;
	int Limit_AM;
#ifdef PREVENTRELOAD
	int Force_304_Time;
#endif
	int Latest_Num;
	int LinkTagCut;
} Settings = {
	RES_YELLOW,
	RES_REDZONE,
	RES_IMODE,
	RES_NORMAL,
	MAX_FILESIZE / 1024,
	LIMIT_PM,
	LIMIT_AM,
#ifdef	PREVENTRELOAD
	FORCE_304_TIME,
#endif
	LATEST_NUM,
	LINKTAGCUT,
};
struct {
	const char *str;
	int *val;
	int len;
	/*	������̒��������炩���ߐ�������A�Q���T���p�ɕ��ׂĂ����̂́A
		�g�����鎞�ɂ�����ƁB
		���ׂ����E�ɂ��Ă�����l����ׂ��B
	*/
} SettingParam[] = {
	{	"RES_YELLOW",	&Settings.Res_Yellow,	},
	{	"RES_REDZONE",	&Settings.Res_RedZone,	},
	{	"RES_IMODE",	&Settings.Res_Imode,	},
	{	"RES_NORMAL",	&Settings.Res_Normal,	},
	{	"MAX_FILESIZE",	&Settings.Max_FileSize,	},
	{	"LIMIT_PM",		&Settings.Limit_PM,		},
	{	"LIMIT_AM",		&Settings.Limit_AM,		},
#ifdef	PREVENTRELOAD
	{	"FORCE_304_TIME",	&Settings.Force_304_Time,	},
#endif
	{	"LATEST_NUM",	&Settings.Latest_Num,	},
	{	"LINKTAGCUT",	&Settings.LinkTagCut	},
};
#undef	RES_YELLOW
#define	RES_YELLOW	Settings.Res_Yellow
#undef	RES_REDZONE
#define	RES_REDZONE	Settings.Res_RedZone
#undef	RES_IMODE
#define	RES_IMODE	Settings.Res_Imode
#undef	RES_NORMAL
#define	RES_NORMAL	Settings.Res_Normal
#undef	MAX_FILESIZE
#define	MAX_FILESIZE	(Settings.Max_FileSize * 1024)
#undef	LIMIT_PM
#define	LIMIT_PM	Settings.Limit_PM
#undef	LIMIT_AM
#define	LIMIT_AM	Settings.Limit_AM
#ifdef	PREVENTRELOAD
#undef	FORCE_304_TIME
#define	FORCE_304_TIME	Settings.Force_304_Time
#endif
#undef	LATEST_NUM
#define	LATEST_NUM	Settings.Latest_Num
#undef	LINKTAGCUT
#define	LINKTAGCUT	Settings.LinkTagCut
#endif	/*	USE_SETTING_FILE	*/

/* <ctype.h>���Ƃ��Ԃ��Ă���A�v�u�� */
#define false (0)
#define true (!false)
#define _C_ (1<<0) /* dat�`�F�b�N�p��؂蕶���� */
#define _U_ (1<<1) /* URL�Ɏg������ */
#define _S_ (1<<2) /* SJIS1�o�C�g�ځ�<br>�^�O���O�̋󔒂��폜����K���ɔ��� */

/* #define isCheck(c) (flagtable[(unsigned char)(c)] & _C_) */
#define isCheck(c) (flagtable[/*(unsigned char)*/(c)] & _C_)
#define isSJIS1(c) (flagtable[(unsigned char)(c)] & _S_)
#define hrefStop(c) (!(flagtable[(unsigned char)(c)] & _U_))

#define	LINK_URL_MAXLEN		256
		/*	���X����URL�Ƃ݂Ȃ�������̍ő咷�B�Z���H	*/

#define _0____ (1<<0)
#define __1___ (1<<1)
#define ___2__ (1<<2)
#define ____3_ (1<<3)
#define _____4 (1<<4)

#define ______ (0)
#define _01___ (_0____|__1___|0)
#define __1_3_ (__1___|____3_|0)
#define ___23_ (___2__|____3_|0)
#define _0_23_ (_0____|___2__|____3_|0)


/*
	'\n'��':'�� isCheck(_C_) �ɒǉ�
	TYPE_TERI�̎��́A'\x81'��','���͂����Ă݂�
	�����[�[�������Ⴄ����
*/
char flagtable[256] = {
	_0____,______,______,______,______,______,______,______, /*  00-07 */
	______,______,_0____,______,______,______,______,______, /*  08-0F */
	______,______,______,______,______,______,______,______, /*  10-17 */
	______,______,______,______,______,______,______,______, /*  18-1F */
	_0____,__1___,______,__1___,__1___,__1___,_01___,______, /*  20-27 !"#$%&' */
#if	defined(TYPE_TERI) && !defined(AUTO_LOGTYPE)
	______,______,__1___,__1___,__1___,__1___,__1___,__1___, /*  28-2F ()*+,-./ */
#else
	______,______,__1___,__1___,_01___,__1___,__1___,__1___, /*  28-2F ()*+,-./ */
#endif
	__1___,__1___,__1___,__1___,__1___,__1___,__1___,__1___, /*  30-37 01234567 */
	__1___,__1___,_01___,__1___,_0____,__1___,______,__1___, /*  38-3F 89:;<=>? */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  40-47 @ABCDEFG */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  48-4F HIJKLMNO */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  50-57 PQRSTUVW */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  58-5F XYZ[\]^_ */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  60-67 `abcdefg */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  68-6F hijklmno */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_, /*  70-77 pqrstuvw */
	__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,__1_3_,______, /*  78-7F xyz{|} */
#if	defined(TYPE_TERI) && !defined(AUTO_LOGTYPE)
	____3_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  80-87 */
#else
	____3_,_0_23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  80-87 */
#endif
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  88-8F */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  90-97 */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  98-9F */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  A0-A7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  A8-AF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  B0-B7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  B8-BF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  C0-C7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  C8-CF */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  D0-D7 */
	____3_,____3_,____3_,____3_,____3_,____3_,____3_,____3_, /*  D8-DF */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E0-E7 */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E8-EF */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  F0-F7 */
	___23_,___23_,___23_,___23_,___23_,______,______,______, /*  F8-FF */
};

typedef struct { /*  class... */
	char **buffers; /* csv�̗v�f */
	int rest; /* �c��̃o�b�t�@�T�C�Y�E�E�����ɂ͔��肵�ĂȂ��̂ŁA���o�C�g�͗]�T���~���� */
} ressplitter;

/****************************************************************/
/* �͈̓��X�g range �֒ǉ�����
 * st, to��1�����ڂ����� '-' �Ȃ�Ώ������Ȃ�
 * range�����t�Ȃ�Βǉ����Ȃ�
 */
static void add_range( struct range *range, const char *st, const char *to )
{
	int i_st;
	int i_to;
	int i;

	if ( *st == '-' && *to == '-' )
		return;

	i_st = atoi(st);
	i_to = atoi(to);
	if ( i_st < 1 )
		i_st = 0;
	if ( i_to < 0 )
		i_to = 0;
	switch ( *to ) {
	case '-':
		i_to = i_st;
		break;
	case '\0':
		i_to = RES_RED;
		break;
	}
	if ( i_st > i_to ) {
		i_to = i_st;
	}

	/* merge */
	for ( i = 0 ; i < range->count ; ++i ) {
		if ( i_st <= (range->array[i].to + 1) && (range->array[i].from - 1) <= i_to ) {
			if ( range->array[i].from > i_st )
				range->array[i].from = i_st;
			if ( range->array[i].to < i_to )
				range->array[i].to = i_to;
			return;
		}
	}

	/* append */
	if ( range->count >= MAX_RANGE )
		return;

	range->array[range->count].from = i_st;
	range->array[range->count].to = i_to;
	range->count++;
}

/* range�̏㉺���擾
 */
int get_range_minmax( const struct range * range, int * st, int *to )
{

	int i, i_st, i_to;
	if ( range->count <= 0 )
		return false;

	i_st = range->array[0].from;
	i_to = range->array[0].to;
	for ( i = 1 ; i < range->count ; ++i ) {
		if ( i_st > range->array[i].from )
			i_st = range->array[i].from;
		if ( i_to < range->array[i].to )
			i_to = range->array[i].to;
	}
	if ( i_to >= RES_RED )
		i_to = 0;
	*st = i_st;
	*to = i_to;
	return true;
}

/* range������
   range����Ȃ��false��Ԃ��Ă��܂��̂Œ���
 */
int in_range( const struct range * range, int lineNo )
{
	int i;
	for ( i = 0 ; i < range->count ; ++i ) {
		if ( range->array[i].from <= lineNo &&
		     range->array[i].to >= lineNo )
			return true;
	}
	return false;
}

/* �����A�n�C�t���A�J���}��������Ȃ镶���񂩂�range��g�ݗ��Ă�
 */
const char *add_ranges_simple( struct range *range, const char *p )
{
	int st, to;
	for (;;) {
		char w_st[12];
		char w_to[12];
		while ( *p == ',' )
			++p;
		if ( *p != '-' && !isdigit(*p) )
			break;
		st = to = atoi(p);
		sprintf( w_st, "%d", st );
		sprintf( w_to, "%d", to );
		while ( isdigit(*p) )
			++p;
		if (*p == '-') {
			++p;
			if ( !isdigit(*p) )
				w_to[0] = '\0';
			else {
				to = atoi(p);
				sprintf( w_to, "%d", to );
			}
			while (isdigit(*p))
				++p;
		}
		add_range( range, w_st, w_to );
	} 
	return p;
}

/****************************************************************/
/* link�̐擪����(�܂�)�̐���
 * �߂�l�͏I�[null�������w��
 */
char *create_link_head(char * url_expr)
{
	char *p = url_expr;
#if defined(USE_INDEX) || defined(CREATE_OLD_LINK)
	const char * key = zz_ky;
#ifdef READ_KAKO
	if ( read_kako[0] )
		key = read_kako;
#endif
#endif

#ifdef	CREATE_OLD_LINK
	if (path_depth) {
#endif
#ifdef USE_INDEX
		if (path_depth == 2) {
			p += sprintf(p, "%.40s/", key );
		}
#endif
#ifdef	CREATE_OLD_LINK
	} else {
		p = url_expr;
		p += sprintf(url_p, "\"" CGINAME "?bbs=%.20s&key=%.40s", zz_bs, key);
	}
#endif
	*p = '\0';
	return p;
}

/* link�̒��ԕ����̐���
 */
char *create_link_mid(char * p, int st, int to, int ls)
{
#ifdef	CREATE_OLD_LINK
	if (path_depth) {
#endif
		if (ls) {
			p += sprintf(p,"l%d",ls);
		} else if (to==0) {
			if (st>1)
				p += sprintf(p,"%d-",st);
			
		} else {
			if ( st != to ) {
				if ( st > 1 )
					p += sprintf(p, "%d", st); /* �n�_ */
				*p++ = '-';
			}
			p += sprintf(p, "%d", to); /* �I�_ */
		}
#ifdef	CREATE_OLD_LINK
	} else {
		if (ls) {
			p += sprintf(p,"&ls=%d",ls);
		} else {
			if ( st > 1 )
				p += sprintf(p, "&st=%d", st);
			if ( to )
				p += sprintf(p, "&to=%d", to);
		}
	}
#endif
	*p = '\0';
	return p;
}

/* link�̖��������̐���
 */
char *create_link_tail(char * url_expr, char * p, int st, int nf, int sst)
{
#ifdef	CREATE_OLD_LINK
	if (path_depth) {
#endif
		if (nf)
			*p++ = 'n';
		if (is_imode())
			*p++ = 'i';
#ifdef CREATE_NAME_ANCHOR
		if (sst && sst!=st) {
			p += sprintf(p,"#%d",sst);
		}
#endif
		if ( p == url_expr )
			p += sprintf(p, "./"); /* �S�� */
#ifdef	CREATE_OLD_LINK
	} else {
		if (nf)
			p += sprintf(p, NO_FIRST );
		if (is_imode()) {
			p += sprintf(p, "&imode=true" );
		}
#ifdef CREATE_NAME_ANCHOR
		if (sst && sst!=st) {
			p += sprintf(p,"#%d",sst);
		}
#endif
		*p++ = '\"';
	}
#endif
	*p = '\0';
	return p;
}

/* read.cgi�Ăяo����LINK��쐬 */
/* ��� pPrintf() �ň�x�����g���Ă͂����Ȃ� */
/* st,to,ls,nf�́A���ꂼ��̌Ăѐ�Bnf=1��nofirst=true
** �g��Ȃ����̂́A0�ɂ��ČĂ�
** sst�́ACHUNK_LINK�̏ꍇ��#�ԍ�
*/
const char *create_link(int st, int to, int ls, int nf, int sst)
{
	static char url_expr[128];
	static char * mid_start = NULL;
	char *p;
	if ( !mid_start )
		mid_start = create_link_head(url_expr);
	p = mid_start;
	if (is_imode() && st==0 && to==0)	/* imode��0-0��ls=10���� */
		ls = 10;
	if ( nf && ls == 0 )
		if ( st == 1 || st == to )	/* �P�_��1���܂ނƂ��͂�'n'�s�v */
			nf = 0;
	p = create_link_mid(p, st, to, ls);
	create_link_tail(url_expr, p, st, nf, sst);
	return url_expr;
}

/* range�ɏ]���ă����N����
 * �߂�l�͕�����
 */
int create_link_range(char * url_expr, const struct range * range, int nf, int sst)
{
	int st, to;
	char *p;
	int i;
	st = to = 0;

	get_range_minmax( range, &st, &to );

	p = create_link_head(url_expr);
	if (is_imode() && st==0 && to==0) {	/* imode��0-0��ls=10���� */
		p = create_link_mid(p, 0, 0, 10);
  	} else {
		if ( st <= 1 || st == to )	/* �P�_��1���܂ނƂ��͂�'n'�s�v */
			nf = 0;
		for ( i = 0 ; i < range->count ; ++i ) {
			int i_to;
#ifdef	CREATE_OLD_LINK
			if (path_depth)
#endif
			{
				if ( i > 0 ) {
					*p++ = ',';
				}
			}
			i_to = range->array[i].to;
			if ( i_to >= RES_RED )
				i_to = 0;
			p = create_link_mid(p, range->array[i].from, i_to, 0);
		}
	}

	p = create_link_tail(url_expr, p, st, nf, sst);
	return p - url_expr;
}

/* �f�B���N�g�����w��i�オ�镶������쐬����
 *
 * �߂�l�͐���������
 * buf�̃T�C�Y�� (up * 3 + 1)�K�v
 */
int up_path( char * buf, size_t up )
{
	size_t i;
	char * w = buf;
	for ( i = 0 ; i < up ; ++i )
	{
		w[0] = '.';
		w[1] = '.';
		w[2] = '/';
		w += 3;
	}
	*w = '\0';
	return w - buf;
}

/* �f���ɖ߂��LINK��쐬 */
void zz_init_parent_link(void)
{
	char * p = zz_parent_link;

#ifdef USE_INDEX
	if (path_depth==2)
		p += up_path(p, 1);
	else
#endif
	{
		p += up_path(p, path_depth+1);
		p += sprintf(p, "%.20s/", zz_bs);
	}
	if (is_imode() ) {
		strcpy(p,"i/");
		return;
	}
#if	0
#ifdef CHECK_MOD_GZIP
	if (mod_gzip_exist)
		return;
#endif
#ifdef GZIP
	if (gzip_flag)
		strcpy(p,"index.htm");
#endif
#endif
}

/* bbs.cgi��LINK��쐬 */
void zz_init_cgi_path(void)
{
	zz_cgi_path[0] = '\0';
	up_path( zz_cgi_path, path_depth );
}

/*
  ������
  toparray �|�C���^�z��̃A�h���X
  buff �R�s�[��̃o�b�t�@�̐擪
  bufsize �����ɂ͔��肵�ĂȂ��̂ŁA���o�C�g�͗]�T���~����
  */

void ressplitter_init(ressplitter *This, char **toparray, char *buff, int bufsize)
{
	This->buffers = toparray;
	This->rest = bufsize;
	*This->buffers = buff;
}

/* <a href="xxx">��PATH�����ɑł�����
   *sp��"<a "����n�܂��Ă��邱�ƁB
   dp, sp �̓|�C���^��i�߂���
   ����������ꍇ��</a>�܂ŏ������邪�A
   ���������s�v�ȏꍇ��<a ....>�܂Ń}�}�R�s(�������͍��)����
   �����s�\�ȏꍇ�́A0 ��߂�

   ad hoc�ɁA���g��&gt;�Ŏn�܂��Ă���
   href��������Ԏ̂ĂāA
   <A></A>�ň͂܂ꂽ���ʂ����o���A
   ����ł������悤�ɂ���
   �z��t�H�[�}�b�g�͈ȉ��̂��́B
   &gt;&gt;nnn
   &gt;&gt;xxx-yyy */
static int rewrite_href(char **dp,		/* �������݃|�C���^ */
			 char const **sp,	/* �ǂݏo���|�C���^ */
			 int istagcut)		/* �^�O�J�b�g���Ă�����? */
{
	char *d = *dp;
	char const *s = *sp;
	char const * copy_start;
	int copy_len;
	int st, to;
	struct range range;
	range.count = 0;
	
	while (*s != '>' && *s != '\n')
		s++;
	if (memcmp(s, ">http://", 8) == 0 ) {
		/* URL�����N�������ꍇ�A���̂܂܏o�͂��Ă݂� */
		s += 8; /* skip ">http://" */
		s = strstr(s, "</a>");
		if ( !s )
			return 0;
		s += 4; /* length of "</a>" */
		copy_start = *sp;
		copy_len = s - copy_start;
		memcpy( d, copy_start, copy_len );
		d += copy_len;
		*sp = s;
		*dp = d;
		return 1;
	}
	if (memcmp(s, ">&gt;&gt;", 9) == 0)
		s += 9;
	copy_start = s;

	s = add_ranges_simple(&range, s);
	if ( memcmp(s, "</a>", 4) == 0 )
		add_ranges_simple(&range, s + 4);

	st = to = -1;
 	if ( range.count > 0 )
		get_range_minmax( &range, &st, &to );
	if ( st < 1 )
		st = 1;
	if ( to < 1 )
		to = lineMax;

	copy_len = s - copy_start;
	if (copy_len == 0 || memcmp(s, "</a>", 4) != 0)
		return 0;
	s += 4;

	if (0 < st && st <= lineMax && 0 < to && to <= lineMax &&
		!(istagcut && isprinted(st) && isprinted(to))) {
			copy_len += 4;	/* strlen("</a>") */
#ifdef CREATE_NAME_ANCHOR
			/* �V�����\�����u�`���� */
			if (isprinted(st) && isprinted(to)) {
				d += sprintf(d, "<a href=#%u>", st);
			} else
#endif
			{
				static const char *target_blank[] = {
					" " TARGET_BLANK,
					"",
				};
				int nofirst = true;
#if defined(CHUNK_ANCHOR) && defined(CREATE_NAME_ANCHOR) && defined(USE_CHUNK_LINK)
				nofirst = false;
				/* chunk�d�l�𐶂������߂�kludge�͈ȉ��ɁB */
				mst = (st - 1) / CHUNK_NUM;
				mto = (to - 1) / CHUNK_NUM;

				if (range.count <= 1 && mst == mto && (st != 1 || to != 1) ) {
					/* chunk�͈� */
					mst = mst * CHUNK_NUM + 1;
					mto = mto * CHUNK_NUM + CHUNK_NUM;
					range.count = 1;
					range.array[0].from = mst;
					range.array[0].to = mto;
				} else 
#endif
				{
					/* chunk���܂��������Ȃ̂ŁA�ŏ��P�ʂ��B*/
				}
				d += sprintf(d, "<a href=" );
				d += create_link_range( d, &range, nofirst, st );
				d += sprintf(d, "%s>", target_blank[is_imode()] );
			}
	}

	/* "&gt;&gt;"�� >> �ɒu�������A�Â�.."</a>"���ێʂ� */
	*d++ = '>';
	*d++ = '>';
	memcpy( d, copy_start, copy_len );
	d += copy_len;
	*sp = s;
	*dp = d;
	return 1;
}
/*
	p�́A"http://"��':'���w���Ă���
	�����������̂ڂ邩��Ԃ��B0�Ȃ�non-match
	�S�����O(http�̏ꍇ)����X�L�������Ă���̂ŁA
	���S���m�F���Ă���Ă�
*/
static int isurltop(const char *p)
{
	if (strncmp(p-4, "http://", 7) == 0)
		return 7-3;
	if (strncmp(p-3, "ftp://", 6) == 0)
		return 6-3;
	return 0;
}
/*
	p�́A"http://www.2ch.net/..."�̍ŏ���'w'���w���Ă�
	http://wwwwwwwww�����ł��邾������
	url�łȂ��Ƃ݂Ȃ�����A0��Ԃ�
*/
static int geturltaillen(const char *p)
{
	const char *top = p;
	int len = 0;
	while (!hrefStop(*p)) {
		if (*p == '&') {
			/* &quot;�ň͂܂ꂽURL�̖������� */
			if (strncmp(p, "&quot;", 6) == 0)
				break;
			if (strncmp(p, "&lt;", 4) == 0)
				break;
			if (strncmp(p, "&gt;", 4) == 0)
				break;
		}
		++len;
		++p;
	}
	if (len) {
		if (memchr(top, '.', len) == NULL	/* '.'���܂܂�Ȃ�URL�͖��� */
			|| *(top + len - 1) == '.')	/* '.'�ŏI���URL��(����)���� */
			len = 0;
		if (len > LINK_URL_MAXLEN)	/* ����������p�� */
			len = 0;
	}
	return len;
}

/*
	url����n�܂�urllen���̕������URL�Ƃ݂Ȃ���bufp�ɃR�s�[�B
	���v�������o�b�t�@�ɓ��ꂽ����Ԃ��B
*/
static int urlcopy(char *bufp, const char *url, int urllen)
{
	return sprintf(bufp,
		"<a href=\"%.*s\" " TARGET_BLANK ">%.*s</a>", 
		urllen, url, urllen, url);
}

/*
	resno���A
	�o�͂���郌�X�ԍ��ł���ꍇtrue
	�͈͊O�ł���ꍇfalse
	�����dat_out()����{��
*/
static int isprinted(int lineNo)
{
	if (lineNo == 1) {
		if (is_nofirst())
			return false;
	} else {
		if (nn_st && lineNo < nn_st)
			return false;
		if (nn_to && lineNo > nn_to)
			return false;
		if (nn_ls && (lineNo-1) < lineMax - nn_ls)
			return false;
		if ( nn_range.count > 0 ) {
			return in_range( &nn_range, lineNo );
		}
	}
	return true;
}

#define	needspace_simple(buftop, bufp)	\
	((bufp) != (buftop) && isSJIS1(*((bufp)-1)))

#ifdef	CUT_TAIL_BLANK
/* ���O�̕������V�t�gJIS�P�o�C�g�ڂł��邩��Ԃ��B */
static int needspace_strict(const char *buftop, const char *bufp)
{
	const char *p = bufp-1;
	while (p >= buftop && isSJIS1(*p))
		p--;
	return (bufp - p) % 2 == 0;
}
# define	needspace(buftop, bufp)	\
	(needspace_simple((buftop), (bufp)) && needspace_strict((buftop), (bufp)))
#else
# define	needspace(buftop, bufp)	\
	needspace_simple((buftop), (bufp))
#endif	/* CUT_TAIL_BLANK */

#define	add_tailspace(buftop, bufp)	\
	(needspace(buftop, bufp) ? (void) (*bufp++ = ' ') : (void)0)

#ifdef	STRICT_ILLEGAL_CHECK
# define	add_tailspace_strict(buftop, bufp)	add_tailspace(buftop, bufp)
#else
# define	add_tailspace_strict(buftop, bufp)	((void)0)
#endif

#if	defined(AUTO_LOGTYPE)
#define		IS_TYPE_TERI	(zz_log_type/* != 0 */)
#elif	defined(TYPE_TERI)
#define		IS_TYPE_TERI	1
#else
#define		IS_TYPE_TERI	0
#endif
/*
	���X��S�������邪�A�R�s�[�ƕϊ�(�ƍ폜)�𓯎��ɍs��
	p �R�s�[�O�̃��X(BigBuffer���̂P���X)
	resnumber	���X�{���ł���ꍇ�Ƀ��X�ԍ�(�s�ԍ��{�P)�A����ȊO��0��n��
	istagcut	<a href=...>��</a>��cut���邩
	Return		����p�̐擪
	non-TYPE_TERI��dat�ɂ�,"<>"�͊܂܂�Ȃ��͂��Ȃ̂ŁA#ifdef TYPE_TERI �͗�
*/
const char *ressplitter_split(ressplitter *This, const char *p, int resnumber)
{
	char *bufp = *This->buffers;
	int bufrest = This->rest;
	int istagcut = (LINKTAGCUT && isbusytime && resnumber > 1) || is_imode();
	/*	���[�v���A*This->Buffers�̓o�b�t�@�̐擪��ێ����Ă���	*/
	while (--bufrest > 0) {
		int ch = *p;
		if (isCheck(ch)) {
			switch (ch) {
			case ' ':
				/* ���Ӗ��ȋ󔒂͂P�����ɂ��� */
				while (*(p+1) == ' ')
					p++;
				if (*(p+1) != '<')
					break;
				if (*(p+2) == '>') {
					if (bufp == *This->buffers) /* ���O�������p�󔒂P�̏ꍇ�ɕK�v */
						*bufp++ = ' ';
					p += 3;
					goto Teri_Break;
				}
				if (memcmp(p, " <br> ", 6) == 0) {
					add_tailspace(*This->buffers, bufp);
					memcpy(bufp, "<br>", 4);
					p += 6;
					bufp += 4;
					continue;
				}
				break;
			case '<': /*  �X���� */
				if (*(p+1) == '>') {
					p += 2;
					goto Teri_Break;
				}
				add_tailspace_strict(*This->buffers, bufp);
				if (resnumber && p[1] == 'a' && isspace(p[2])) {
					char *tdp = bufp;
					char const *tsp = p;
					if (!rewrite_href(&tdp, &tsp, istagcut))
						break; /* �����s�\��<a >�^�O�͂��̂܂܏o�͂��A���s */
					bufrest -= tdp - bufp;
					bufp = tdp;
					p = tsp;
					continue;
				}
				break;
			case '&':
				/* add_tailspace_strict(*This->buffers, bufp); */
				if (memcmp(p+1, "amp", 3) == 0) {
					if (*(p + 4) != ';')
						p += 4 - 1;
				}
				/* &MAIL->&amp;MAIL�̕ϊ��́Adat���ǂ݂̂�����`���Ⴊ�Ή����Ȃ��Ɩ���
				   �����ϊ�����Ȃ�bbs.cgi */
				break;
			case ':':
#ifndef NO_LINK_URL_BUSY
				if (resnumber)
#else
				if (resnumber && !istagcut)
					/* url�̃����N��(���ԑтɂ����)�p�~����Ȃ� */
#endif
				{
					if (*(p+1) == '/' && *(p+2) == '/' && isalnum(*(p+3))) {
						/*
							�����dat(���O�������Ȃ��Ƃ�1����)�Ȃ�΁A
							p�̒��O�͍Œ�A" ,,,"�ƂȂ��Ă���(����ɓ��e���̗]�T������)�B
							�Ȃ̂ŁA4����("http")�܂ł̓I�[�o�[�t���[�̊댯�͂Ȃ�
							�������A�����́Aresnumber���A���m�ɓn����Ă���̂��O��B
						*/
						int pdif = isurltop(p);	/*	http://�̏ꍇ�A4���Ԃ�	*/
						if (pdif) {
							int taillen = geturltaillen(p + 3);
							if (taillen && bufrest > taillen * 2 + 60) {
								bufp -= pdif, p -= pdif, bufrest += pdif, taillen += pdif + 3;
								add_tailspace_strict(*This->buffers, bufp);
								pdif = urlcopy(bufp, p, taillen);
								bufp += pdif, bufrest -= pdif, p += taillen;
								continue;
							}
						}
					}
				}
				break;
			case '\n':
				goto Break;
				/*	break;	*/
#if	!defined(TYPE_TERI) || defined(AUTO_LOGTYPE)
			case COMMA_SUBSTITUTE_FIRSTCHAR: /*  *"��"(8197)  "�M"(814d) */
				if (!IS_TYPE_TERI) {
					/* �����͓��ɕp�x���Ⴂ���߁A�ǐ��A�ێ琫�̂ق����d�v */
					if (memcmp(p, COMMA_SUBSTITUTE, COMMA_SUBSTITUTE_LEN) == 0) {
						ch = ',';
						p += 4 - 1;
					}
				}
				break;
			case ',':
				if (!IS_TYPE_TERI) {
					p++;
					goto Break;
				}
				break;
#endif
			case '\0':
				if (p >= BigBuffer + zz_fileSize)
					goto Break;
				/* �ǂݔ�΂��̂ق����A����Ƃ��Ă͓K�؂��� */
				ch = '*';
				break;
			default:
				break;
			}
		}
		*bufp++ = ch;
		p++;
	}
Teri_Break:
	/* ���O����','�������Ă��鎞��split���~�X��̂ŁA�����\��������̂ŁA */
	/* This->isTeri = true; */
Break:
	add_tailspace(*This->buffers, bufp);
	*bufp++ = '\0';
	This->rest -= bufp - *This->buffers;
	*++This->buffers = bufp;
	
	/* ��؂蒼��̋󔒂��폜 */
	if (*p == ' ') {
		if (!IS_TYPE_TERI) {
			if (!(*(p+1) == ','))
				++p;
		} else {
			if (!(*(p+1) == '<' && *(p+2) == '>'))
				++p;
		}
	}
	return p;
}

void splitting_copy(char **s, char *bufp, const char *p, int size, int linenum)
{
	ressplitter res;
	ressplitter_init(&res, s, bufp, size);
	
	p = ressplitter_split(&res, p, false); /* name */
	p = ressplitter_split(&res, p, false); /* mail */
	p = ressplitter_split(&res, p, false); /* date */
	p = ressplitter_split(&res, p, linenum+1); /* text */
	p = ressplitter_split(&res, p, false); /* title */
	if (s[1][0] == '0' && s[1][1] == '\0')
		s[1][0] = '\0';
}

#ifdef	AUTO_LOGTYPE
static void check_logtype()
{
	if (lineMax) {
		const char *p;
		
		zz_log_type = 0;	/* non-TYPE_TERI */
		for (p = BigLine[0]; p < BigLine[1]; ++p) {
			if (*p == '<' && *(p+1) == '>') {
				zz_log_type = 1;	/* TYPE_TERI */
				break;
			}
		}
	}
}
#else
#define	check_logtype()		/*	*/
#endif

/* �^�C�g�����擾����zz_title�ɃR�s�[
*/
static void get_title()
{
	char *s[20];
	char p[SIZE_BUF];
	
	if (lineMax) {
		splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
		strncpy(zz_title, s[4], sizeof(zz_title)-1);
		zz_title[sizeof(zz_title)-1] = '\0';
	}
}

/* �X�g�b�p�[�E1000 Over�̔���
*/
static int isthreadstopped()
{
	char *s[20];
	char p[SIZE_BUF];
	static const char * stoppers[] = {
		STOPPER_MARKS
	};
	int i;
	
	if (lineMax >= RES_RED)
		return 1;
#ifdef WRITESTOP_FILESIZE
	if (zz_fileSize > MAX_FILESIZE - WRITESTOP_FILESIZE * 1024)
		return 1;
#endif
	if (lineMax) {
		splitting_copy(s, p, BigLine[lineMax-1], sizeof(p) - 20, lineMax-1);
		for ( i = 0 ; i < (sizeof stoppers)/(sizeof stoppers[0]) ; ++i )
			if ( strstr( s[2], stoppers[i] ) )
				return 1;
		return 0;
	}
	return 1;
}


/****************************************************************/
/*	BadAccess						*/
/* ����=0, �s����=1��Ԃ�                                       */
/****************************************************************/
int BadAccess(void)
{
	static char *agent_kick[] = {
#ifdef	Katjusha_Beta_kisei
		"Katjusha",
#endif
		"WebFetch",
		"origin",
		"Nozilla",
	};
	int i;

	if ( is_head() )
		return 0;
#if defined(RAWOUT)
	if ( rawmode ) {
#ifdef	CHECK_MOD_GZIP
		if (mod_gzip_exist > 1)
			return 0;
#endif
		return !gzip_flag;
	}
#endif
	if (!*zz_http_user_agent && !*zz_http_language)
		return 1;

	for (i = 0; i < sizeof(agent_kick) / sizeof(char *); i++) {
		if (strstr(zz_http_user_agent, agent_kick[i]))
			return 1;
	}

	return 0;
}
/****************************************************************/
/*	Log Out							*/
/****************************************************************/
int logOut(char const *txt)
{
#ifdef LOGLOGOUT
	FILE *fp;
#endif

#ifdef DEBUG
	return 1;
#endif

	if (!BadAccess())
		return 1;

#ifdef	LOGLOGOUT
/*
if(strcmp(zz_bs,"ascii"))	return 1;
*/
	fp = fopen("./logout.txt", "a+");
	if (fp == NULL)
		return;
	fprintf(fp, "%02d:%02d:%02d %8s:", tm_now.tm_hour, tm_now.tm_min,
		tm_now.tm_sec, zz_bs);
	fprintf(fp, "(%15s)", zz_remote_addr);
	fprintf(fp, "%s***%s\n", zz_http_language, zz_http_user_agent);

/*
	fprintf(fp,"%s\n",zz_query_string);
	if(strstr(zz_http_user_agent,"compatible"))
		fprintf(fp,"%s\n",zz_http_language);
	else
		fprintf(fp,"%s***%s\n",zz_http_language,zz_http_user_agent);
	if(*zz_http_referer)	fprintf(fp,"%s\n",zz_http_referer);
	else			fprintf(fp,"%s***%s\n",zz_http_language,zz_http_user_agent);
	else			fprintf(fp,"%s\n",zz_http_user_agent);
*/

	fclose(fp);
#endif
	html_error(ERROR_LOGOUT);
	return 1;
}
/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
void html_bannerNew(void)
{
	pPrintf(pStdout, R2CH_HTML_NEW_BANNER);
}

/****************************************************************/
/*	Get file size(out_html1)				*/
/****************************************************************/
static int out_html1(int level)
{
	if (out_resN)
		return 0;
	html_head(level, zz_title, lineMax);
	out_resN++;
	return 0;
}
/****************************************************************/
/*	Get file size(out_html)					*/
/****************************************************************/
static int out_html(int level, int line, int lineNo)
{
	char *s[20];
	char *r0, *r1, *r3;
	char p[SIZE_BUF];

#ifdef	CREATE_NAME_ANCHOR
#define	LineNo_			lineNo, lineNo
#else
#define	LineNo_			lineNo
#endif

/*printf("line=%d[%s]<P>\n",line,BigLine[line]);return 0;*/

	if (!out_resN) {	/* Can I write header ?   */
		html_head(level, zz_title, lineMax);
	}
	out_resN++;

	splitting_copy(s, p, BigLine[line], sizeof(p) - 20, line);
	if (!*p)
		return 1; 
	
	r0 = s[0];
	r1 = s[1];
	r3 = s[3];

	if (!is_imode()) {	/* no imode */
		if (*r3 && s[4]-r3 < 8192) {
			if (*r1) {
#ifdef SAGE_IS_PLAIN
				if (strcmp(r1, "sage") == 0)
					pPrintf(pStdout,
						R2CH_HTML_RES_SAGE("%d", "%d", "%s", "%s", "%s"),
						LineNo_, r0, s[2], r3);
				else
#endif
					pPrintf(pStdout,
						R2CH_HTML_RES_MAIL("%d", "%d", "%s", "%s", "%s", "%s"),
						LineNo_, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_RES_NOMAIL("%d", "%d", "%s", "%s", "%s"),
					LineNo_, r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_RES_BROKEN_HERE("%d"),
				lineNo);
		}
		if (isbusytime && out_resN > RES_NORMAL) {
#ifdef PREV_NEXT_ANCHOR
			need_tail_comment = 1;
#else
#ifdef SEPARATE_CHUNK_ANCHOR
			pPrintf(pStdout, R2CH_HTML_TAIL_SIMPLE("%02d:00","%02d:00"),
				LIMIT_PM - 12, LIMIT_AM);
#else
			pPrintf(pStdout, R2CH_HTML_TAIL("%s","%d"),
				create_link(lineNo,
					lineNo + RES_NORMAL, 0,1,0),
				RES_NORMAL);
			pPrintf(pStdout,
				R2CH_HTML_TAIL2("%s", "%d") R2CH_HTML_TAIL_SIMPLE("%02d:00", "%02d:00"),
				create_link(0,0, RES_NORMAL, 1,0),
				RES_NORMAL,
				LIMIT_PM - 12, LIMIT_AM);
#endif
#endif
			return 1;
		}
	} else {		/* imode  */
		if (*r3) {
			if (*s[1]) {
				pPrintf(pStdout, R2CH_HTML_IMODE_RES_MAIL,
					lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_IMODE_RES_NOMAIL,
					lineNo, r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_IMODE_RES_BROKEN_HERE,
				lineNo);
		}
		if (out_resN > RES_IMODE && lineNo != lineMax ) {
#ifndef PREV_NEXT_ANCHOR
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL("%s", "%d"),
				create_link(lineNo, 
					lineNo + RES_IMODE, 0,0,0),
				RES_IMODE);
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL2("%s", "%d"),
				create_link(0, 0, 0 /*RES_IMODE*/, 1,0),	/* imode�̓X����ls=10���� */
				RES_IMODE);
#endif
			return 1;
		}
	}

	return 0;
#undef	LineNo_
}
/****************************************************************/
/*	Output raw data file					*/
/****************************************************************/
#ifdef RAWOUT
int dat_out_raw(void)
{
	const char *begin = BigLine[0];
	const char *end = BigLine[lineMax];
	char statusline[512];
	char *vp = statusline;
#ifdef	RAWOUT_PARTIAL
	int first = 0, last = 0;
#endif

#ifdef	Katjusha_DLL_REPLY
	if (zz_katjusha_raw) {
		if (BigBuffer[raw_lastsize-1] != '\n') {
			html_error(ERROR_ABORNED);
			return 0;
		}
		begin = BigBuffer + raw_lastsize;
		vp += sprintf(vp, "+OK");
	} else
#endif
#ifdef	RAWOUT_PARTIAL
	if (raw_lastnum == 0 && raw_lastsize == 0
		&& (nn_st || nn_to || nn_ls > 1)) {
		/* nn_xx��nofirst�̊֌W���ŕω����Ă��邩������Ȃ��̂ōĎZ�o */
		int st = atoi(zz_st), to = atoi(zz_to), ls = atoi(zz_ls);
		
		first = 1, last = lineMax;
		if (ls > 1)		/* for Ver5.22 bug */
			st = lineMax - ls + 1;
		if (0 < st && st <= lineMax)
			first = st;
		if (0 < to && to <= lineMax)
			last = to;
		if (first > last)
			last = first;
		
		begin = BigLine[first-1];
		end = BigLine[last];
		vp += sprintf(vp, "+PARTIAL");
	} else
#endif
	/* �����񍐂��ꂽ�ŏI���X�ԍ�����уT�C�Y����v���Ă��Ȃ�
	   ��΁A�ŏ��̍s�ɂ��̎|������ */
	/* raw_lastsize > 0 �ɂ����nnn.0�ł��ځ[�񌟏o�𖳌��ɂł��邪
	   �T�[�o�[�ō폜�������̂̓N���C�A���g�ł��폜�����ׂ��ł��� */
	if(raw_lastnum > 0
		&& raw_lastsize >= 0
		&& !(raw_lastnum <= lineMax
		 && BigLine[raw_lastnum] - BigBuffer == raw_lastsize)) {
		vp += sprintf(vp, "-INCR");
		/* �S���𑗐M����悤�ɕύX */
	} else {
		vp += sprintf(vp, "+OK");
		/* �������M�p�ɐ擪��ݒ� */
		begin = BigLine[raw_lastnum];
	}
	vp += sprintf(vp, " %d/%dK", end - begin, MAX_FILESIZE / 1024);
#ifdef	RAWOUT_PARTIAL
	if (first && last) {
		vp += sprintf(vp, "\t""Range:%u-%u/%u",
			begin - BigLine[0], end - BigLine[0] - 1, BigLine[lineMax] - BigLine[0]);
		vp += sprintf(vp, "\t""Res:%u-%u/%u", first, last, lineMax);
	}
#endif
#ifdef	AUTO_KAKO
	{
		static char *messages[] = {
			"",
			"\t""Status:Stopped",
			"\t""Location:temp/",
			"\t""Location:kako/",
			/* ���m�Ȉʒu��m�点��K�v�͂Ȃ��͂� */
		};
		int where = zz_dat_where;
		if (where || isthreadstopped())
			where++;
		vp += sprintf(vp, "%s", messages[where]);
	}
#endif
	pPrintf(pStdout, "%s\n", statusline);
	/* raw_lastnum ����S���𑗐M���� */
#ifdef ZLIB
	if (gzip_flag)
		gzwrite(pStdout, (const voidp)begin, end - begin);
	else
#endif
		fwrite(begin, 1, end - begin, pStdout);

	return 1;
}

#ifdef	RAWOUT_MULTI
static int split_multireq(const char *req, char *bbs, char *key)
{
	const char *p;
	p = strchr(req, '/');
	if (p) {
		memcpy(bbs, req, p-req);
		bbs[p-req] = '\0';
		req = p + 1;
	}
	*key = '\0';
	p = strchr(req, '.');
	if (!p)
		return 0;
	memcpy(key, req, p-req);
	key[p-req] = '\0';
	return atoi(p+1);
}

/* �G���[�����Ŋy�����邽�� */
struct multiout_resource {
	char *Buffer;
	int fd;
	int fileSize;
	char *info;
};

/* �߂�l��
   0   ����I��
   -1  ���X�Vor�擾�ς݂͈̔͂�0�ȉ�  ("+OK 0/512K"��Ԃ�)
   ��  html_error_t�̃G���[�R�[�h +1  ("-ERR ..."��Ԃ�) */
static int exec_multiout(struct multiout_resource *rp, const char *req)
{
	int lastsize;
	char fname[1024];
	time_t lastmod;
	char *ip = rp->info;
	
	lastsize = split_multireq(req, zz_bs, zz_ky);
	ip += sprintf(ip, "\t""Request:%s/%s.%u", zz_bs, zz_ky, lastsize);
	if (lastsize <= 0)
		return -1;
	
	create_fname(fname, zz_bs, zz_ky);
	lastmod = getFileLastmod(fname);
	rp->fileSize = zz_fileSize;
	if (lastmod == -1 || rp->fileSize == 0)
		return 1 + ERROR_NOT_FOUND;
	ip += sprintf(ip, "\t""LastModify:%u", (unsigned)lastmod);
	
	if (rp->fileSize > MAX_FILESIZE)
		return 1 + ERROR_TOO_HUGE;
	if (rp->fileSize == lastsize)
		return -1;
	if (rp->fileSize < lastsize)
		return 1 + ERROR_ABORNED;
	
	rp->fd = open(fname, O_RDONLY);
	if (rp->fd < 0)
		return 1 + ERROR_NOT_FOUND;
	
#ifdef USE_MMAP
	rp->Buffer = mmap(NULL, rp->fileSize, PROT_READ, MAP_PRIVATE, rp->fd, 0);
	if (rp->Buffer == MAP_FAILED)
		return 1 + ERROR_NO_MEMORY;
#else
	rp->Buffer = malloc(rp->fileSize);
	if (!rp->Buffer)
		return 1 + ERROR_NO_MEMORY;
	/* �S���ǂނ͖̂��ʂ����A��{�I��mmap���g����͂��Ȃ̂ŋC�ɂ��Ȃ� */
	read(rp->fd, rp->Buffer, rp->fileSize);
#endif
	if (rp->Buffer[lastsize-1] != '\n')
		return 1 + ERROR_ABORNED;
	
	pPrintf(pStdout, "+OK %d/%dK%s\n",
		rp->fileSize - lastsize, MAX_FILESIZE / 1024, rp->info);
	
#ifdef ZLIB
	if (gzip_flag)
		gzwrite(pStdout, (const voidp)(rp->Buffer + lastsize), rp->fileSize - lastsize);
	else
#endif
		fwrite(rp->Buffer + lastsize, 1, rp->fileSize - lastsize, pStdout);
	
	/* �����̉���͖߂��Ă��� */
	return 0;
}

/* ������������Ԃ�
*/
int rawout_multi()
{
	char buff[256];
	char info[512];
	struct multiout_resource res;
	int count = 0;
	const char *query = zz_query_string;
	
	buff[0] = '\0';
	while ((query = GetString(query, buff, sizeof(buff), "dat")) != NULL) {
		int retcode;
		buff[40] = '\0';
		/* �蔲���p�̏����� */
		res.Buffer = NULL;
		res.fd = -1;
		res.fileSize = 0;
		res.info = info;
		
		retcode = exec_multiout(&res, buff);
		if (retcode) {
			/* �G���[���̏��� */
			if (retcode < 0) {
				pPrintf(pStdout, "+OK 0/%dK" "%s" "\n", MAX_FILESIZE / 1024, info);
			} else {
				print_error((enum html_error_t)(retcode-1), true, info);
			}
		}
		
		/* �蔲���̎������ */
		if (res.fd >= 0)
			close(res.fd);
#ifdef	USE_MMAP
		if (res.Buffer && res.Buffer != MAP_FAILED)
			munmap(res.Buffer, res.fileSize);
#else
		if (res.Buffer)
			free(res.Buffer);
#endif
		
		if (++count > RAWOUT_MULTI_MAX)
			break;
	}
	return count;
}
#endif

#endif

/****************************************************************/
/*	Get file size(dat_out)					*/
/*	Level=0�̂Ƃ��́A�O���̏o��				*/
/*	Level=1�̂Ƃ��́A�����̏o��				*/
/****************************************************************/

int dat_out(int level) 
{ 
	int line; 
	int threadStopped=0; 

#ifdef READ_KAKO
	if (read_kako[0]) {
		threadStopped = 1;
		/* �ߋ����O��FORM��RELOADLINK����\���ɂ��邽�� */
	}
#endif
	for (line = 0; line < lineMax; line++) { 
		int lineNo = line + 1; 
		if (!isprinted(lineNo)) 
			continue; 
		if (out_html(level, line, lineNo)) { 
			line++; 
			break; /* ��0���Ԃ�̂́A�G���[����RES_NORMAL/RES_IMODE�ɒB������ */ 
		} 
		if (lineNo==1 && is_imode() && nn_st<=1 && nn_ls==0) 
			++out_resN; 
	} 
	out_html1(level); /* ���X���P���\������Ă��Ȃ����Ƀw�b�_��\������ */ 

	if (isthreadstopped())
		threadStopped=1;
	if ( !is_imode() )
		pPrintf(pStdout, R2CH_HTML_PREFOOTER);
#ifdef	AUTO_KAKO
	if (zz_dat_where) {
		pPrintf(pStdout, R2CH_HTML_CAUTION_KAKO);
		threadStopped = 1;
	}
#endif
#ifdef RELOADLINK
	if (
#ifdef USE_INDEX
	    !level && 
#endif
	    lineMax == line && !threadStopped) {
	
		html_reload(line);	/*  Button: Reload */
	}
#endif
	html_foot(level, lineMax, threadStopped);

	return 0;
}
/****************************************************************/
/*	Get file size(dat_read)					*/
/*	BigBuffer, BigLine, LineMax���X�V�����͂�		*/
/****************************************************************/
int dat_read(char const *fname,
	     int st,
	     int to,
	     int ls)
{
	int in;

#ifdef	PUT_ETAG
	if (BigBuffer)
		return 0;
#endif
#ifdef USE_INDEX	/* USE_DIGEST */
	zz_fileSize = getFileSize(fname);
#endif

	if (zz_fileSize > MAX_FILESIZE)
		html_error(ERROR_TOO_HUGE);
	if (zz_fileSize < 10)
		html_error(ERROR_NOT_FOUND); /* �G���[��ʂ͕ʂɂ��������������� */
	if (*zz_ky == '.')
		html_error(ERROR_NOT_FOUND);

	nn_st = st;
	nn_to = to;
	nn_ls = ls;

	in = open(fname, O_RDONLY);
	if (in < 0)
	{
		html_error(ERROR_NOT_FOUND);
		return 0;
	}
#ifdef USE_MMAP
	if (!isbusytime)
		zz_mmap_mode = MAP_PRIVATE;
	BigBuffer = mmap(NULL,
			 zz_mmap_size = zz_fileSize + (zz_mmap_mode == MAP_SHARED),
			 PROT_READ,
			 zz_mmap_mode,
			 zz_mmap_fd = in,
			 0);
	if (BigBuffer == MAP_FAILED)
		html_error(ERROR_NO_MEMORY);
#else
	BigBuffer = malloc(zz_fileSize + 32);
	if (!BigBuffer)
		html_error(ERROR_NO_MEMORY);

	read(in, BigBuffer, zz_fileSize);
	close(in);
#endif

#if	defined(RAWOUT) && defined(Katjusha_DLL_REPLY)
	if (zz_katjusha_raw) {
		BigLine[lineMax = 0] = BigBuffer + zz_fileSize;
		return 0;
	}
#endif
	lineMax = getLineMax();
#ifdef	RAWOUT
	if (rawmode)
		return 0;
#endif
	check_logtype();
	get_title();
/*
html_error(ERROR_MAINTENANCE);
*/
	return 0;
}
/****************************************************************/
/*	Get line number						*/
/****************************************************************/
int getLineMax(void)
{
	int line = 0;
	const char *p = BigBuffer;
	const char *p1;

	if (!p)
		return -8;

	p1 = BigBuffer + zz_fileSize;	/* p1 = �Ō��\n�̎��̃|�C���^ */
	while (p < p1 && *(p1-1) != '\n')	/* �Ō�̍s����T�� �����dat�Ȃ��薳�� */
		p1--;
	if (p1 - p < 10)	/* �K�������ǁA���͏o�Ȃ��͂� */
		return -8;
	do {
		if ( line >= (sizeof BigLine) / (sizeof BigLine[0]) - 1 )
		{
			/* ����s���𒴂���悤�Ȃ�A�������Ȃ��ăt�@�C�������Ƃ݂Ȃ�
			   �������邱�Ƃɂ���Ĉ�ꂽ�����ׂĂ������1�s�Ƃ��ď�������
			   ���܂�Ȃ��悤�ɂ���
			*/
			zz_fileSize = p - BigBuffer;
			break;
		}
		BigLine[line] = p;
		++line;
		p = (char *)memchr(p, '\n', p1-p);
		if (p == NULL)
			break;
		++p;
	} while (p < p1);
	
	/*
		�Ō�̃��X�̎��ɁA�t�@�C�����ւ̃|�C���^�����Ă����B
		����ɂ��A���X�̒����̓|�C���^�̍��ł��ށB
		(dat_out_raw��strlen���Ă��镔���ւ̑Ή�)
	*/
	BigLine[line] = BigBuffer + zz_fileSize;
	return line;
}
/****************************************************************/
/*	Get file size						*/
/****************************************************************/
int getFileSize(char const *file)
{
	struct stat CountStat;
	int ccc = 0;
	if (!stat(file, &CountStat))
		ccc = (int) CountStat.st_size;
	return ccc;
}
/****************************************************************/
/*	Get file last-modified(getFileLastmod)			*/
/****************************************************************/
time_t getFileLastmod(char const *file)
{
	struct stat CountStat;
	time_t ccc;
	if (!stat(file, &CountStat)) {
		ccc = CountStat.st_mtime;
		zz_fileSize = CountStat.st_size;
		return ccc;
	} else
		return -1;
}
/****************************************************************/
/*	Get file last-modified(get_lastmod_str)			*/
/****************************************************************/
int get_lastmod_str(char *buf, time_t lastmod)
{
	strftime(buf, 1024, "%a, %d %b %Y %H:%M:%S GMT",
		 gmtime(&lastmod));
	return (1);
}
/****************************************************************/
/*	PATH_INFO�����						*/
/*	/board/							*/
/*	/board/							*/
/*	/board/datnnnnnn/[range] �ł���Ƃ݂Ȃ�			*/
/*	return: path���L����������1��Ԃ�			*/
/*	����p: zz_bs, zz_ky, zz_st, zz_to, zz_nf		*/
/*		�Ȃǂ��X�V�����ꍇ������			*/
/****************************************************************/
static void parse_path_param(const char *s)
{
	/* st/to ���݂ق��̃`�F�b�N�̂��� "-"�����Ă��� */
	strcpy(zz_st, "-");
	strcpy(zz_to, "-");

	/* strtok()�Ő؂�o�����o�b�t�@�͑���������
	   �������Ă���̂ŁAbuffer overrun�͂Ȃ��͂� */
	while (s[0]) {
		char *p;
		/* �͈͎w��̃t�H�[�}�b�g�͈ȉ��̂��̂�����

		   /4	(st=4&to=4)
		   /4-	(st=4)
		   /-6	(to=6)
		   /4-6	(st=4to=4)
		   /l10 (ls=10)
		   /i   (imode=true)
		   /.   (nofirst=false)
		   /n   (nofirst=true)

		   �J�L�R1�͓��ʈ����ŁAnofirst�ɂ��
		   ���삪���E�������ۂ��̂ŁA�ǂ����悤 */

		/* st �����o�� */
		if (isdigit(*s)) {
			for (p = zz_st; isdigit(*s); p++, s++)
				*p = *s;
			*p = 0;
		} else if (*s == 'i') {
			set_imode_true();
			s++;
		} else if (*s == '.') {
			set_nofirst_false();
			s++;
		} else if (*s == 'n') {
			set_nofirst_true();
			s++;
		} else if (*s == 'l') {
			s++;
			/* ls�����o�� */
			if (isdigit(*s)) {
				for (p = zz_ls; isdigit(*s); p++, s++)
					*p = *s;
				*p = 0;
			}
#if 0
			/* ls= ��nofirst=true ��W���� */
			if (!zz_nf[0]) {
				set_nofirst_true();
			}
#endif
		} else if (*s == '-') {
			s++;
			/* to�����o�� */
			if (isdigit(*s)) {
				for (p = zz_to; isdigit(*s); p++, s++)
					*p = *s;
				*p = 0;
			} else {
				/* to=���Ƃ��� */
				zz_to[0] = '\0';
			}
		} else if (*s == ',') {
			s++;
			add_range( &nn_range, zz_st, zz_to );
			*zz_st = *zz_to = '-';
		} else {
			/* �K�肳��ĂȂ������������̂ŕ]������߂� */
			if (strchr(s, '/'))
				need_basehref = 1;
			break;
		}
	}

	add_range( &nn_range, zz_st, zz_to );
	*zz_st = *zz_to = '\0';

	if ( zz_ls[0] == '\0' && nn_range.count > 0 ) {
		get_range_minmax( &nn_range, &nn_st, &nn_to );

		sprintf( zz_st, "%d", nn_st );
		if ( nn_st > 0 && nn_st == nn_to ) {
			/* �P�_�́Anofirst=true��default�� */
			if (!zz_nf[0]) {
				set_nofirst_true();
			}
		} else {
			sprintf( zz_to, "%d", nn_to );
		}
	}
}

static const char *get_path_token(const char *s, char *buf)
{
	const char *next = strchr(s, '/');
	if (next) {
		memcpy(buf, s, next-s);
		buf[next-s] = '\0';
		path_depth++;
		s = next + 1;
	}
	return s;
}

static int get_path_info(char const *path_info)
{
	char const *s = path_info;

#if	1
	/* index.html ��base���o�͂����ꍇ�Ɉꉞ������B�ז��Ȃ�����āB */
	static const char index_based_path[] = "/test/" CGINAME;
	if (memcmp(s, index_based_path, sizeof(index_based_path)-1) == 0) {
		s += sizeof(index_based_path)-1;
		need_basehref = 1;
	}
#endif
	path_depth = 0;
	/* PATH_INFO �́A'/' �Ŏn�܂��Ă�悤�ȋC�������� */
	if (s[0] != '/')
		return 0;
	
	/* ��������PATH_INFO�͕|���̂Ŏ󂯕t���Ȃ� */
	if (strlen(s) >= 256)
		return 0;
	
	++s, ++path_depth;
	s = get_path_token(s, zz_bs);
#ifdef	READ_KAKO
	if (memcmp(s, "kako/", 5) == 0
#ifdef	READ_TEMP
		|| memcmp(s, "temp/", 5) == 0
#endif
		) {
		memcpy(read_kako, s, 5);
		get_path_token(s += 5, read_kako+5);
	}
#endif
	s = get_path_token(s, zz_ky);
	
	if (!*zz_ky && isdigit(*s)/* && strlen(s) >= 9*/) {
		/* /test/read.cgi/board/999999999 �ł������N���邽�߂̏��u */
		strcpy(zz_ky, s);
#ifdef	READ_KAKO
		if (read_kako[0]) {
			strcpy(read_kako+5, s);
			++path_depth;
		}
#endif
		s += strlen(s);
		++path_depth;
		need_basehref = 1;
	}

	parse_path_param(s);
	/* �����͊����������̂Ƃ݂Ȃ� */
	return 1;
}

/****************************************************************/
/*	SETTING_R.TXT�̓ǂ݂���					*/
/****************************************************************/
#ifdef	USE_SETTING_FILE
static void parseSetting(const void *mmptr, int size)
{
	/* SETTING_R.TXT��ǂ� */
	char const *cptr;
	char const *endp;
	int i;
	for (i = 0; i < sizeof(SettingParam) / sizeof(SettingParam[0]); i++)
		SettingParam[i].len = strlen(SettingParam[i].str);
	for (cptr = mmptr, endp = cptr + size - 1;
	     cptr < endp && *endp != '\n'; endp--)
		;
	for ( ; cptr && cptr < endp;
	      cptr = memchr(cptr, '\n', endp - cptr), cptr?cptr++:0) {
		if (*cptr != '\n' && *cptr != '#' && *cptr != ';') {
			int i;
			for (i = 0; i < sizeof(SettingParam)
				     / sizeof(SettingParam[0]); i++) {
				int len = SettingParam[i].len;
				if (cptr + len < endp 
				    && cptr[len] == '='
				    && strncmp(cptr,
					       SettingParam[i].str,
					       len) == 0) {
					*SettingParam[i].val
						= atoi(cptr + len + 1);
					break;
				}
			}
		}
	}
}

void readSettingFile(const char *bbsname)
{
#ifndef USE_INTERNAL_SETTINGS
	char fname[1024];
	int fd;
	sprintf(fname, "../%.256s/%s", bbsname, SETTING_FILE_NAME);
	fd = open(fname, O_RDONLY);
	if (fd >= 0) {
#ifdef	USE_MMAP
		struct stat st;
		void *mmptr;
		fstat(fd, &st);
		mmptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		parseSetting(mmptr, st.st_size);
#ifdef  EXPLICIT_RELEASE
		munmap(mmptr, st.st_size);
		close(fd);
#endif
#else
		/* �܂��ݒ�t�@�C����8k�ȏ�����Ȃ���΂����Ƃ������Ƃ� */
		char mmbuf[8192];
		int size = read(fd, mmbuf, sizeof(mmbuf));
		parseSetting(mmbuf, size);
		close(fd);
#endif	/* USE_MMAP */
	}
#else	/* USE_INTERNAL_SETTINGS */
	static struct _setting {
		char *board_name;
		char *settings;
	} special_setting[] = {
		SPECIAL_SETTING
		{ NULL, },
	};
	struct _setting *setting;
	for (setting = special_setting; setting->board_name; setting++) {
		if (!strcmp(bbsname, setting->board_name)) {
			parseSetting(setting->settings, strlen(setting->settings));
			break;
		}
	}
#endif	/* USE_INTERNAL_SETTINGS */
}
#endif	/*	USE_SETTING_FILE	*/

/****************************************************************/
/*	cell-phone detection					*/ 
/****************************************************************/
int is_imode_agent( const char * user_agent )
{
	const static char * small_screen_agents[] = {
	"DoCoMo/",	/* i-Mode */
	"UP.Browser/",	/* EZWeb */
	"J-PHONE/", 	/* J-Phone */
	"ASTEL/",	/* Dot i */
	};
	int i;
	int len;
	int user_agent_len;

	user_agent_len = strlen( user_agent );

	for ( i = 0 ; i < ((sizeof small_screen_agents) / (sizeof small_screen_agents[0])) ; ++i )
	{
		len = strlen( small_screen_agents[i] );
		if ( len < user_agent_len && !memcmp(user_agent, small_screen_agents[i], len) )
		{
			return true;
		}
	}
	return false;
}
/****************************************************************/
/*	GET Env							*/
/****************************************************************/
void zz_GetEnv(void)
{
	char const * request_method;
	char const * env;
	currentTime = (long) time(&t_now);
#ifdef CONFIG_TIMEZONE
	putenv("TZ=" CONFIG_TIMEZONE);
#endif
	tzset();
	tm_now = *localtime(&t_now);

	request_method = getenv("REQUEST_METHOD");
	zz_head_request = request_method && (strcmp(request_method, "HEAD") == 0);
	zz_remote_addr = getenv("REMOTE_ADDR");
	zz_remote_host = getenv("REMOTE_HOST");
	zz_http_referer = getenv("HTTP_REFERER");
	zz_server_name = getenv("HTTP_HOST");
	if (!zz_server_name)
		zz_server_name = getenv("SERVER_NAME");
	zz_script_name = getenv("SCRIPT_NAME");
	zz_path_info = getenv("PATH_INFO");
	zz_query_string = getenv("QUERY_STRING");
	zz_temp = getenv("REMOTE_USER");
	zz_http_user_agent = getenv("HTTP_USER_AGENT");
	zz_http_language = getenv("HTTP_ACCEPT_LANGUAGE");
	zz_http_if_modified_since = getenv("HTTP_IF_MODIFIED_SINCE");

	if (!zz_remote_addr)
		zz_remote_addr = KARA;
	if (!zz_remote_host)
		zz_remote_host = KARA;
	if (!zz_http_referer)
		zz_http_referer = KARA;
	if (!zz_path_info)
		zz_path_info = "";	/* XXX KARA���g���񂷂͕̂|�� */
	if (!zz_query_string)
		zz_query_string = KARA;
	if (!zz_temp)
		zz_temp = KARA;
	if (!zz_http_user_agent)
		zz_http_user_agent = KARA;
	if (!zz_http_language)
		zz_http_language = KARA;

	zz_bs[0] = zz_ky[0] = zz_ls[0] = zz_st[0] = '\0';
	zz_to[0] = zz_nf[0] = zz_im[0] = '\0';
	if (!get_path_info(zz_path_info)) {
		/* ����ȍ~�Apath ���t�^����Ă��邩�ǂ�����
		   ����� zz_path_info �̃e�X�g�ōs���Ă��� */
		zz_path_info = NULL;
	}
	GetString(zz_query_string, zz_bs, sizeof(zz_bs), "bbs");
	GetString(zz_query_string, zz_ky, sizeof(zz_ky), "key");
	GetString(zz_query_string, zz_ls, sizeof(zz_ls), "ls");
	GetString(zz_query_string, zz_st, sizeof(zz_st), "st");
	GetString(zz_query_string, zz_to, sizeof(zz_to), "to");
	GetString(zz_query_string, zz_nf, sizeof(zz_nf), "nofirst");
	if ( is_imode_agent( zz_http_user_agent ) )
		set_imode_true();
	GetString(zz_query_string, zz_im, sizeof(zz_im), "imode");
#ifdef RAWOUT
	zz_rw[0] = '\0';
	GetString(zz_query_string, zz_rw, sizeof(zz_rw), "raw");
#endif

#ifdef READ_KAKO
	if (strncmp(zz_ky, "kako/", 5)==0) {
		strcpy(read_kako, zz_ky);
		strcpy(zz_ky, zz_ky+5);
	} else if (strncmp(zz_ky, "temp/", 5)==0) {
		strcpy(read_kako, zz_ky);
		strcpy(zz_ky, zz_ky+5);
	}
#endif
	/* zz_ky �͒P�Ȃ�32�r�b�g���l�Ȃ̂ŁA
	   �ȍ~�A�����ł�������悤�ɂ��Ă��� */
	nn_ky = atoi(zz_ky);

#if	defined(GZIP) || defined(RAWOUT)
	gzip_flag = compress_none;
	env = getenv("HTTP_ACCEPT_ENCODING");
	if (env) {
		if (strstr(env, "x-gzip")) {
			gzip_flag = compress_x_gzip;
		} else if (strstr(env, "gzip")) {
			gzip_flag = compress_gzip;
		}
	}
#ifdef CHECK_MOD_GZIP
	env = getenv("SERVER_SOFTWARE");
	mod_gzip_exist = (env && strstr(env,"mod_gzip/")!=NULL);
#endif
#endif
#ifdef RAWOUT
	rawmode = (*zz_rw != '\0');
	if(rawmode) {
		char *p = strchr(zz_rw, '.');
		if(p) {
			/* raw=(last_article_no).(local_dat_size) */
			raw_lastsize = atoi(p + 1);
		}
		raw_lastnum = atoi(zz_rw);
		if(raw_lastnum<0)
			raw_lastnum = 0;
		if(!p || raw_lastsize < 0) {
			raw_lastsize = 0;	/* -INCR ��Ԃ����� */
		}
#ifdef	Katjusha_DLL_REPLY
		zz_katjusha_raw = (zz_rw[0] == '.' && raw_lastsize > 0
			/*&& strstr(zz_http_user_agent, "Katjusha")*/);
#endif
	}
#endif
#ifdef	USE_SETTING_FILE
	readSettingFile(zz_bs);
#endif
	isbusytime = IsBusy2ch();
}

/*----------------------------------------------------------------------
	�I������
----------------------------------------------------------------------*/
void atexitfunc(void)
{
	/* html_error()�ł̓�d�Ăяo���h�~ */
	static int isCalled = 0;
	if (isCalled) return;
	isCalled = 1;

#ifdef EXPLICIT_RELEASE
#ifdef USE_MMAP
	if (BigBuffer && BigBuffer != MAP_FAILED) {
		munmap(BigBuffer, zz_mmap_size);
		close(zz_mmap_fd);
	}
#else
	/* ���������ɎU��΂��Ă��̂ł܂Ƃ߂Ă݂� */
	if (BigBuffer)
		free(BigBuffer);
#endif
#endif /* EXPLICIT_RELEASE */
#ifdef ZLIB
	if (gzip_flag) {
		int outlen;
		char *outbuf;

		gzclose(pStdout);
		outlen = gz_getdata(&outbuf);	/* ���k�f�[�^���󂯎�� */

		if ( outlen != 0 && outbuf == NULL ) {
			/* ���k����malloc�G���[���� */
			pPrintf = (zz_printf_t) fprintf;
			pStdout = (gzFile) stdout;
			putchar('\n');
			html_error(ERROR_NO_MEMORY);
		}
		printf("%s", content_encoding[gzip_flag]);
#ifdef NN4_LM_WORKAROUND
		if (!strncmp(zz_http_user_agent, "Mozilla/4.", 10)
		    && !strstr(zz_http_user_agent, "compatible"))
			putchar('\n');
		else
#endif
			printf("Content-Length: %d\n\n", outlen);

		if ( !is_head() )
			fwrite(outbuf,1,outlen,stdout);
		/* fflush(stdout); XXX ����fflush()���ĕK�v�H */
		/* free(outbuf); outbuf��free���ׂ��� exit�Ȃ̂łق��Ƃ� */
	}
#elif defined(GZIP)
	if(gzip_flag) {
		fflush(stdout);
		close(1);		/* gzip���I�������邽��close���� */
		waitpid(pid, NULL, 0);
	}
#endif
}

#ifdef	PUT_ETAG
/* �Ƃ肠����
   ETag: "���M����crc32-�S�̂̃T�C�Y-�S�̂̃��X��-���M���̃T�C�Y-���M���̃��X��-flag"
   ��%x�ŏo�͂���悤�ɂ��Ă݂��B
*/
typedef struct {
	unsigned long crc;
	int allsize;
	int allres;
	int size;
	int res;
	int flag;
		/*	���̂Ƃ���A
			((isbusytime) 		<< 0)
		  |	((nn_to > lineMax)	<< 1)
		*/
} etagval;

static void etag_put(const etagval *v, char *buff)
{
	sprintf(buff, "\"%lx-%x-%x-%x-%x-%x\"",
		v->crc, v->allsize, v->allres, v->size, v->res, v->flag);
}
static int etag_get(etagval *v, const char *s)
{
	return sscanf(s, "\"%lx-%x-%x-%x-%x-%x\"",
		&v->crc, &v->allsize, &v->allres, &v->size, &v->res, &v->flag) == 6;
}
static void etag_calc(etagval *v)
{
	int line = nn_st;
	int end = nn_to;
	
	if (end == 0 || end > lineMax)
		end = lineMax;
	if (line == 0)
		line = 1;
/*	if (line > lineMax)
		line = lineMax;
	���������ƁA�{���̏o�͔͈͂ƐH���Ⴂ���o��B
*/
	memset(v, 0, sizeof(*v));
	v->allsize = zz_fileSize;
	v->allres = lineMax;
	v->flag = (isbusytime << 0) | ((nn_to > lineMax) << 1);
	v->crc = crc32(0, NULL, 0);
	
	if (!is_nofirst()) {
		v->res++;
		v->size += BigLine[1] - BigLine[0];
		v->crc = crc32(v->crc, BigLine[0], BigLine[1] - BigLine[0]);
		if (line == 1)
			line++;
	}
	if (isbusytime) {
		if (end - line + 1 + !is_nofirst() > RES_NORMAL)
			end = line - 1 - !is_nofirst() + RES_NORMAL;
	}
	if (line <= end) {
		v->res += end - line + 1;
		v->size += BigLine[end] - BigLine[line-1];
		v->crc = crc32(v->crc, BigLine[line-1], BigLine[end] - BigLine[line-1]);
	}
}

int need_etag(int st, int to, int ls)
{
	const char *env;
	
	if (BadAccess())	/* �u�Ȃ񂩕ςł��v�̏ꍇ��dat�̓ǂ݂��݂�����邽�� */
		return false;
	
	env = getenv("SERVER_PROTOCOL");
	if (!env || !*env)
		return false;
	if (strstr(env, "HTTP/1.1") == NULL)
		return false;
	/* �����ɂ́AIf-None-Match��t�����Ȃ�UA�̃��X�g
	   (���͕t������UA�̃��X�g)��u��
	   ���Ӗ��ȏꍇ��false��Ԃ��̂��]�܂����B */
	
	/* to=nn������ꍇ���� */
	if (!to)
		return false;
	return true;
}

void create_etag(char *buff)
{
	etagval val;
	etag_calc(&val);
	etag_put(&val, buff);
}

int match_etag(const char *etag)
{
	etagval val, qry;
	const char *oldtag;
	
	/* CHUNK�̕ω������l�����A�ǂ��ŕs����o�邩���킩��Ȃ��̂�
	   ���ʁAbusytime�ȊO�͐V�������̂�Ԃ� */
	if (!isbusytime)
		return false;
	oldtag = getenv("HTTP_IF_NONE_MATCH");
	if (!oldtag || !*oldtag)
		return false;
	if (!etag_get(&val, etag) || !etag_get(&qry, oldtag))
		return false;
	
	if (val.crc != qry.crc || val.res != qry.res || val.size != qry.size)
		return false;
	
	/* �ȉ��ŁA�\���͈͊O�ɒǉ����X���������ꍇ��
	   �X�V���ׂ����ǂ��������߂� */
	
	/* �ǉ���100���X�ȓ��Ȃ�A����Ƃ݂Ȃ��Ă悢�E�E�Ǝv�� */
	if (val.allres < qry.allres + 100)
		return true;
	
	/* �L���b�V����busytime�O���̂Ȃ�΁A�������D�悳����ׂ��E�E�Ǝv�� */
	if ((qry.flag & (1 << 0)) == 0)
		return true;
	
	/* �\���͈͂������ꍇ�́ACHUNK���͋C�ɂ��Ȃ��Ă悢�E�E�Ǝv�� */
	if (val.res < 40)
		return true;
	
	/* �X���̎������ߕt������A�x������\�����ׂ��E�E�Ǝv�� */
	if (val.allres >= RES_YELLOW)
		return false;
	
	/* ���̑��A��������Ƃ肠�����X�V�����ɓ���񍐂�҂��Ă݂悤�E�E�Ǝv�� */
	return true;
}
#endif	/* PUT_ETAG */

/****************************************************************/
/****************************************************************/
void header_base_out(void)
{
	if ((path_depth < 3 || need_basehref) && zz_server_name && zz_script_name) {
#ifdef READ_KAKO
		if (read_kako[0]) {
			pPrintf(pStdout, R2CH_HTML_BASE_DEFINE, zz_server_name, zz_script_name, zz_bs, read_kako);
			path_depth = 4;
		} else
#endif
		{
			pPrintf(pStdout, R2CH_HTML_BASE_DEFINE, zz_server_name, zz_script_name, zz_bs, zz_ky);
			path_depth = 3;
		}
	}
}

#ifdef	REFERDRES_SIMPLE
int can_simplehtml(void)
{
	char buff[128];
	const char *p;
	const char *ref;
	const char * cginame = zz_script_name;
	static const char indexname[] = "index.htm";
	
	if (!isbusytime)
		return false;
	if (!nn_st || !nn_to || is_imode())
		return false;
	if (nn_st > nn_to || nn_to > lineMax)
		return false;
#if	1
	/* �Ƃ肠�����A�����N��̃��X���P�̏ꍇ���� */
	if (nn_st != nn_to || !is_nofirst())
		return false;
#else
	if (!(nn_st + 10 <= nn_to))
		return false;
#endif
	ref = getenv("HTTP_REFERER");
	if (!ref || !*ref)
		return false;

	p = strstr(ref, cginame);
	if (p) {
		p += strlen(cginame);
		if ( *p == '?' ) {
			char bbs[sizeof(zz_bs)];
			char key[sizeof(zz_ky)];
			const char * query_string = p+1;
#if	0
			/* �ꕔ��REQUEST_URI��REFERER�ɐݒ肳���炵���̂� */
			if (strcmp(zz_query_string, p + 1) == 0)
				return false;
#endif
			bbs[0] = key[0] = '\0';
			GetString(query_string, bbs, sizeof(bbs), "bbs");
			GetString(query_string, key, sizeof(key), "key");
			return (strcmp(zz_bs, bbs) == 0) && (strcmp(zz_ky, key) == 0);
		}
		sprintf(buff, "/%.50s/%.50s/", zz_bs, zz_ky);
		if (!strncmp(p, buff, strlen(buff)))
			return true;
	}

	sprintf(buff, "/%.50s/", zz_bs);
	p = strstr(ref, buff);
	if (p) {
		p += strlen(buff);
		if (*p == '\0')
			return true;
		if (strncmp(p, indexname, sizeof(indexname)-1) == 0)
			return true;
	}

	return false;
}

int out_simplehtml(void)
{
	int n = nn_st;
	
	/* html_head() */
	pPrintf(pStdout, R2CH_HTML_HEADER_0);
	header_base_out();
	pPrintf(pStdout, R2CH_SIMPLE_HTML_HEADER_1("%s", ""), zz_title);
	pPrintf(pStdout, R2CH_HTML_HEADER_2("%s"), zz_title);
	
	out_resN++;	/* �w�b�_�o�͂�}�~ */
	if (!is_nofirst()) {
		out_html(0, 0, 1);
		if (n == 1)
			n++;
	}
	for ( ; n <= nn_to; ++n) {
		out_html(0, n-1, n);
	}
	
	/* html_foot() */
	/* i-mode���͗��Ȃ��͂������A��������悤�ɂ���Ȃ�PREFOOTER��imode���͏o���Ȃ����� */
	pPrintf(pStdout, R2CH_HTML_PREFOOTER R2CH_HTML_FOOTER);
	return 0;
}
#endif	/* REFERDRES_SIMPLE */

/* ���`�� /bbs/kako/100/1000888777.*�ɑΉ��B doko��path���쐬���� */
static int find_old_kakodir(char *doko, const char *key, const char *ext)
{
	sprintf(doko, KAKO_DIR "%.3s/%.50s.%s", zz_bs, key, key, ext);
	return access(doko, 00) == 0;
}

static int find_kakodir(char *doko, const char *key, const char *ext)
{
	static char soko[256];
	if (soko[0] == '\0')
		kako_dirname(soko, key);
	sprintf(doko, KAKO_DIR "%s/%.50s.%s", zz_bs, soko, key, ext);
	return access(doko, 00) == 0 || find_old_kakodir(doko, key, ext);
}
static int find_tempdir(char *doko, const char *key, const char *ext)
{
	sprintf(doko, TEMP_DIR "%.50s.%s", zz_bs, key, ext);
	return access(doko, 00) == 0;
}

static void create_fname(char *fname, const char *bbs, const char *key)
{
	/* �v���O�����~�X�ɂ��I�[�o�[�t���[�̉\���������Ă��� */
	zz_bs[60] = zz_ky[60] = '\0';
#ifdef READ_KAKO
	if (read_kako[0] == 'k') {
		find_kakodir(fname, key, "dat");
# ifdef READ_TEMP
	} else if (read_kako[0] == 't') {
		find_tempdir(fname, key, "dat");
# endif
	} else
#endif
		sprintf(fname, DAT_DIR "%.256s.dat", bbs, key);

#ifdef	AUTO_KAKO
	if (zz_ky[0] && access(fname, 00) != 0) {
		/* zz_ky�`�F�b�N�́Asubject.txt�擾�������O���邽�� */
		char buff[1024];
		const char *keybase = LastChar(key, '/');
		int mode = AUTO_KAKO_MODE;
#if	1
		/* ���G���ԑшȊO�̂݁Atemp/,kako/�̉{����������ꍇ */
		if (isbusytime)
			mode = 0;
#endif
#ifdef	RAWOUT
		if (rawmode)
			mode = 2;	/* everywhere */
#endif
		if (mode >= 2 && find_kakodir(buff, keybase, "dat")) {
			zz_dat_where = 2;
		} else if (mode >= 1 && find_tempdir(buff, keybase, "dat")) {
			zz_dat_where = 1;
		}
		if (zz_dat_where)
			strcpy(fname, buff);
	}
#endif	/* AUTO_KAKO */

#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif

#ifdef	RAWOUT
	/* �X���ꗗ�����ɐ������[�h */
	if (1 <= path_depth && path_depth < 3
#ifndef USE_INDEX
	&& rawmode
	/* rawmode�Ɍ���Asubject.txt��Ԃ����Ƃ��\�Ƃ���B
	   GZIP���p�Ńg���t�B�b�N�팸��_���B

	   �Ƃ���� raw�̃p�����[�^�́A�ʏ햖���ǉ������s���Ȃ����Ƃ�O��Ƃ����d�g��
	   �ł��邽�߁A����S�̂��X�V����� subject.txt�ɂ͓K�؂ł͂Ȃ��B�]���Č����
	   raw=0.0 �Ɍ��肵�Ďg���ׂ��ł���B
	   subject.txt��ǂݏo�����[�h�̂Ƃ��ɂ́Araw�̃p�����[�^���[����ύX���A
	   �擪����w�萔�݂̂��擾�\�ɂ���ΗL�p�ł͂Ȃ���?
	 */ 
#endif
	) {
		sprintf(fname, "../%.256s/subject.txt", zz_bs);
#ifdef	USE_MMAP
		zz_mmap_mode = MAP_PRIVATE;
#endif
#ifdef	RAWOUT_PARTIAL
		if (zz_ls[0] && !zz_to[0])
			strcpy(zz_to, zz_ls), zz_ls[0] = zz_st[0] = '\0';
#endif
	}
#endif
}

/****************************************************************/
/*	MAIN							*/
/****************************************************************/
int main(void)
{
#ifdef GZIP
#ifndef ZLIB
	int pipefds[2];
	int whitespace = 2048;
#endif
#endif
#ifdef USE_INDEX
	DATINDEX_OBJ dat;
#endif
	char fname[1024];

	int st, to, ls;

#ifdef ZLIB
	pStdout = (gzFile) stdout;
#endif
	zz_GetEnv();

	/* st, to, ls�́A���̃��x���ł������Ă��� */
	st = atoi(zz_st);
	to = atoi(zz_to);
	ls = atoi(zz_ls);

	if (st < 0)
		st = 0;
	if (to < 0)
		to = 0;
	if (ls < 0)
		ls = 0;
	if (is_imode()) {	/* imode */
		if (!st && !to && !ls)
			ls = RES_IMODE;
	}
	/* �����w�肳�ꂽ����ls��D�� */
	if (ls) {
		st = to = 0;
	}
	if (st == 1 || (to && st<=1))	/* 1��\������̂�nofirst=false */
		zz_nf[0] = '\0';

	if (!is_nofirst() && ls > 0) {
		ls--;
		if(ls == 0) {
			ls = 1;
			set_nofirst_true();
		}
	}


#ifdef USE_INDEX
	/* ������index��ǂݍ���ł���
	   ���͂��łɁA.dat���}�b�s���O���ꂿ����Ă�̂ŁA
	   �߂��E�`��BigBuffer�͗p�ς݂ɂȂ��Ă��܂��� */
	if (nn_ky && !datindex_open(&dat, zz_bs, nn_ky)) {
		printf("Content-Type: text/plain\n\n%s/%s/%ld/", zz_bs, zz_ky, nn_ky);
		puts("error");
		return 0;
	}
#endif

#ifdef RAWOUT
	if(rawmode)
		/* puts("Content-Type: application/octet-stream"); */
		/* ���݂�.dat�� MIME type �ɍ��킹��D�e�L�X�g�f�[�^����... */
		puts("Content-Type: text/plain");
	else
#endif
		puts("Content-Type: text/html");

	create_fname(fname, zz_bs, zz_ky);
	zz_fileLastmod = getFileLastmod(fname);
#ifdef USE_INDEX
	/* �����d�l: �e��p�����[�^��������
	   �݂��ɖ�������悤�Ȑݒ�́A
	   �󂯓����ׂ��ł͂Ȃ� */
	if (st == 0 && to == 0 && ls) {
		to = dat.linenum;
		st = to - ls + 1;
		ls = 0;
	}

	/* ��������ƁA���΂炭 digest ���g���Ȃ��Ȃ� */
	zz_fileSize = dat.dat_stat.st_size;
	if (nn_ky)
		zz_fileLastmod = datindex_lastmod(&dat,
						  !is_nofirst(),
						  st,
						  to);
#endif
	get_lastmod_str(lastmod_str, zz_fileLastmod);
#ifdef PREVENTRELOAD
	if (zz_http_if_modified_since) {
		time_t modtime = ap_parseHTTPdate(zz_http_if_modified_since);
		/* �Ō�ɕύX���Ă��� FORCE_304_TIME �b�Ԃ͋��� 304
		 * (zz_http_if_modified_since��ɂ���Ɖi�v�ɋL�������
		 *  �Ȃ�����)
		 */
		if(modtime != BAD_DATE && zz_fileLastmod != -1
		   && (modtime + FORCE_304_TIME >= t_now
		       || modtime >= zz_fileLastmod))
#else
	if (zz_http_if_modified_since
	    && strstr(zz_http_if_modified_since, lastmod_str))
#endif
	{
		puts("Status: 304 Not Modified\n");
		return 0;
	}
#ifdef PREVENTRELOAD
	}
#endif

#ifdef	RAWOUT
	/* ���̕����́A���If-Modified-Since��������
	   �t�@�C���T�C�Y���x�̃��N�G�X�g�𑗂��ė���A
	   ������`����ɂ�镉�ב����ւ̑΍�	*/
	if (rawmode) {
		/* �����܂łŊ���zz_fileSize��(�t�@�C�����������)�ݒ肳��Ă��� */
		if (zz_fileSize && zz_fileSize == raw_lastsize) {
			if (!zz_http_if_modified_since) {	/* NULL����ˁHKARA���ꂽ��ς��Ă� */
				/* �񈳏k�ŕԂ� */
				char buff[120];
				printf("Last-Modified: %s\n", lastmod_str);
				printf("Content-Length: %d\n""\n""%s", 
					sprintf(buff, "+OK 0/%dK\n", MAX_FILESIZE / 1024), buff);
				return 0;
			}
		}
#ifdef	Katjusha_DLL_REPLY
		if (zz_katjusha_raw && zz_fileSize) {
			if (zz_fileSize < raw_lastsize) {
				/* ������html_error()���ĂԂƔ񈳏k�̃e�L�X�g��Ԃ����A�\��Ȃ��͂��B
				   Content-Encoding��LastModified���o�͂��Ȃ��B*/
				char buff[120];
				printf("Content-Length: %d\n""\n""%s",
					sprintf(buff, "-ERR %s\n", ERRORMES_ABORNED), buff);
				/*html_error(ERROR_ABORNED);*/
				return 0;
			}
		}
#endif
	}
#endif
	/*  �I�������o�^ */
	atexit(atexitfunc);
#ifdef	PUT_ETAG
	if (need_etag(st, to, ls)) {
		char etag[60];
		dat_read(fname, st, to, ls);
		create_etag(etag);
		if (match_etag(etag)) {
			puts("Status: 304 Not Modified\n");
			return 0;
		}
		printf("ETag: %s\n", etag);
	}
#endif

/* �����܂łŁA304 ��Ԃ��\�������鏈���͂����܂� */

#ifdef	GZIP
#ifdef	CHECK_MOD_GZIP
	if (mod_gzip_exist) {
		mod_gzip_exist += gzip_flag;
#ifdef NN4_LM_WORKAROUND
		if (!(!strncmp(zz_http_user_agent, "Mozilla/4.", 10)
		    && !strstr(zz_http_user_agent, "compatible")))
#endif
			gzip_flag = compress_none;
	}
#endif
#if	!defined(ZLIB)
	printf("%s", content_encoding[gzip_flag]);
#endif
#endif	/* GZIP */
	zz_init_parent_link();
	zz_init_cgi_path();

/*  Get Last-Modified Date */
	if (zz_fileLastmod != -1)	/* getFileLastmod �̃G���[����-1���Ԃ� */
		printf("Last-Modified: %s\n", lastmod_str);

#ifdef ZLIB
	if ( is_head() || gzip_flag == compress_none )
#endif
	{
		putchar('\n');
	}
	/* �ʏ킱���Ńw�b�_�͏I��肾���A gzip_flag ���ɂ͂܂��w�b�_������ */
#ifdef DEBUG
	sleep(1);
#endif
	fflush(stdout);
#ifdef ZLIB
	if ( is_head() || gzip_flag == compress_none )
#endif
	{
		/* HEAD���N�G�X�g�Ȃ炱���ŏI�� */
		if ( is_head() ) {
			return 0;
		}
	}

#ifdef GZIP
	if (gzip_flag) {
#ifndef ZLIB
		pipe(pipefds);
		if ((pid = fork()) == 0) {
			/* child  */
			dup2(pipefds[0], 0);
			close(pipefds[0]);
			close(pipefds[1]);
			execl("/bin/gzip", "gzip", "-cf9", NULL);
			pPrintf(pStdout, "Error can't exec\n");
			exit(1);
		}
		/* parent */
		dup2(pipefds[1], 1);
		close(pipefds[0]);
		close(pipefds[1]);
		fflush(stdout);
		while (whitespace--)
			putchar(' ');
		fflush(stdout);
#else
		fflush(stdout);

		/*  prepare zlib */
		/* ����(0,"w")��zlib/gzio.c�œ��ʈ���
		   ����stdout��ݒ肵�Aclose���Ȃ�
		  �i2ch�o�[�W�����̓Ǝ��d�l �ʏ�stdin��"w"���Ȃ�����j */
		pStdout = gzdopen(0, "wb9");

		pPrintf = gzprintf;
		/* gzdopen()��"wb9"���w�肵���̂ŕs�v */
		/* gzsetparams(pStdout, Z_BEST_COMPRESSION,
			Z_DEFAULT_STRATEGY); */
#endif /* ZLIB */
	}
#endif /* GZIP */

	setvbuf(stdout, NULL, _IOFBF, 64 * 1024);
	logOut("");
#ifdef	RAWOUT_MULTI
	if (rawmode && !*zz_bs && !*zz_ky && !raw_lastnum && !raw_lastsize && !path_depth) {
		/* path_depth���P�ȏゾ�ƁAcreate_fname��subject�擾���[�h�ɂȂ��Ă��܂� */
		/* zz_bs�̏����l�̐ݒ���l�����邪�ALast-Modified�̏o�͂��� */
		if (rawout_multi())
			return 0;	/* �P�ł��������Ă�����A�����ŏI��� */
	}
#endif

	dat_read(fname, st, to, ls);

#ifdef RAWOUT
	if (rawmode) {
		dat_out_raw();
		return 0;
	}
#endif
#ifdef USE_INDEX
	if (path_depth == 2) {
		if (zz_ky[0] == '-')
			dat_out_subback();	/* �X���ꗗ */
		else
			dat_out_index();	/* �_�C�W�F�X�g */
		return 0;
	}
#endif
#ifdef READ_KAKO
	if (path_depth && read_kako[0]) {
		if (path_depth!=4) {
			html_error(ERROR_NOT_FOUND);
			return 0;
		}
	} else
#endif
	if (path_depth && path_depth!=3) {
		html_error(ERROR_NOT_FOUND);
		return 0;
	}
#ifdef	REFERDRES_SIMPLE
	if (can_simplehtml())
		out_simplehtml();
	else
#endif
		dat_out(0);
	return 0;
}

#if	defined(READ_KAKO) || defined(READ_TEMP)
/* �ߋ����O�ւ̃����N�쐬
 *
 * path_depth, zz_ky�����������Ă��܂��̂Œ���
 */
const char * create_kako_link(const char * dir_type, const char * key)
{
	static char result[256];
	int needs_close_quote = false;
	char *wp = result;
	const char *p;

	*wp++ = '\"'; /* "�ň͂� */

	path_depth = 3;
	
	if (path_depth) {
		wp += sprintf(wp, "%.40s", zz_script_name );
		wp += sprintf(wp, "/%.40s/%.8s/%.40s/", zz_bs, dir_type, key);
	} else
		wp += sprintf(wp, "%.40s", zz_cgi_path);

	sprintf(zz_ky,"%.8s/%.40s", dir_type, key);
#ifdef READ_KAKO_THROUGH
	p = create_link(atoi(zz_st), atoi(zz_to), atoi(zz_ls), is_nofirst(), 0);
#else
	p = create_link(0, 0, 0, 0, 0);
#endif
	if (*p == '\"')
		++p;		/* �S�̂�"�ň͂ނ��߁A"���t���Ă���Ύ�菜�� */
	else
		needs_close_quote = true;	/* path�`���̍Ō�ɂ�"�� */

	if ( p[0]=='.' && p[1]=='/' )
		p += 2;	/* "./"�͎ז� */

	wp += sprintf(wp, "%.80s", p);

	if ( needs_close_quote )
		*wp++ = '\"'; /* "�ň͂� */
	*wp = '\0';
	return result;
}
#endif
/****************************************************************/
/*	ERROR END(html_error)					*/
/* isshowcode��extinfo��RAWOUT�������g��Ȃ� */
/****************************************************************/
int print_error(enum html_error_t errorcode, int isshowcode, const char *extinfo)
{
	char tmp[256];
	char doko[256];
	char comcom[256];
	const char * mes;
	switch ( errorcode ) {
	case ERROR_TOO_HUGE:
		mes = ERRORMES_TOO_HUGE;
		break;
	case ERROR_NOT_FOUND:
		mes = ERRORMES_NOT_FOUND;
		break;
	case ERROR_NO_MEMORY:
		mes = ERRORMES_NO_MEMORY;
		break;
	case ERROR_MAINTENANCE:
		mes = ERRORMES_MAINTENANCE;
		break;
	case ERROR_LOGOUT:
		mes = ERRORMES_LOGOUT;
		break;
#ifdef	Katjusha_DLL_REPLY
	case ERROR_ABORNED:
		mes = ERRORMES_ABORNED;
		break;
#endif
	default:
		mes = "";
	}

	*tmp = '\0';
	strcpy(tmp, LastChar(zz_ky, '/'));
#ifdef RAWOUT
	if(rawmode) {
		/* -ERR (message)�̓G���[�B */
		char str[512];
		int retcode = (errorcode + 1) * 100;
		if (errorcode == ERROR_NOT_FOUND) {
			if (find_kakodir(doko, tmp, "dat") || find_kakodir(doko, tmp, "dat.gz")) {
				sprintf(str, ERRORMES_DAT_FOUND " %s", doko);
				mes = str;
				retcode += 1;
			} else if (find_tempdir(doko, tmp, "dat")) {
				mes = ERRORMES_TEMP_FOUND;
				retcode += 2;
			}
		}
		if (isshowcode)
			pPrintf(pStdout, "-ERR %d %s%s\n", retcode, mes, extinfo);
		else
			pPrintf(pStdout, "-ERR %s\n", mes);
		return 0;
	}
#endif
	
	*comcom = '\0';
	if (errorcode == ERROR_LOGOUT) {
		sprintf(comcom, R2CH_HTML_ERROR_ADMIN);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_1, mes, mes, mes, comcom);

	if (strstr(zz_http_user_agent, "Katjusha")) {
		pPrintf(pStdout, R2CH_HTML_ERROR_2);
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_3);
	html_bannerNew();
	pPrintf(pStdout, R2CH_HTML_ERROR_4);

	if (errorcode == ERROR_NOT_FOUND) {
		if (find_kakodir(doko, tmp, "html")) {
			/* �ߋ����O�q�ɂ�html���� */
			pPrintf(pStdout, R2CH_HTML_ERROR_5_HTML, 
				zz_cgi_path, doko, tmp);
		} else if (find_kakodir(doko, tmp, "dat")) {
			/* �ߋ����O�q�ɂ�dat���� */
#ifdef READ_KAKO
 			pPrintf(pStdout, R2CH_HTML_ERROR_5_DAT("%s","%s"),
				create_kako_link("kako", tmp), tmp);
#else
			pPrintf(pStdout, R2CH_HTML_ERROR_5_DAT,
				zz_cgi_path, doko, tmp);
#endif
		} else if (find_tempdir(doko, tmp, "dat")) {
#ifdef READ_TEMP
			pPrintf(pStdout, R2CH_HTML_ERROR_5_TEMP("%s","%s"),
				create_kako_link("kako",tmp), tmp);
#else
			pPrintf(pStdout, R2CH_HTML_ERROR_5_TEMP,
				tmp);
#endif
		} else {
			pPrintf(pStdout, R2CH_HTML_ERROR_5_NONE,
				zz_cgi_path, zz_bs);
		}
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_6);

	return 0;
}
void html_error(enum html_error_t errorcode)
{
	print_error(errorcode, false, "");
	exit(0);
}
#if 0
/****************************************************************/
/*	HTML BANNER						*/
/****************************************************************/
void html_banner(void)
{
	pPrintf(pStdout, R2CH_HTML_BANNER);
}

/****************************************************************/
/*	ERROR END(html_error999)				*/
/****************************************************************/
int html_error999(char const *mes)
{
	char zz_soko[256];	/* �q�ɔԍ�(3������) */
	char tmp[256];		/* ?�X���ԍ�(9������) */
	char tmp_time[16];	/* ���� "hh:mm:ss" */
	*tmp = '\0';
#ifdef RAWOUT
	if(rawmode) {
		/* -ERR (message)�̓G���[�B */
		pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
	}
#endif
	strcpy(tmp, LastChar(zz_ky, '/'));
	kako_dirname(zz_soko, tmp);
	sprintf(tmp_time, "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min,
		tm_now.tm_sec);

	pPrintf(pStdout, R2CH_HTML_ERROR_999_1,
		mes, zz_bs, zz_ky, zz_ls, zz_st, zz_to, zz_nf, zz_fileSize,
		lineMax, tmp_time, zz_bs, zz_soko, tmp, tmp);
	html_banner();
	pPrintf(pStdout, R2CH_HTML_ERROR_999_2);

	exit(0);
}
#endif
/****************************************************************/
/*								*/
/****************************************************************/
#define GETSTRING_LINE_DELIM '&'
#define GETSTRING_VALUE_DELIM '='
#define MAX_QUERY_STRING 200
/* �d�l�ύX
   �߂�l�Ƃ��āAdst�ł͂Ȃ�(�g���Ă��Ȃ��̂�)
   line���̎��̈ʒu��Ԃ��悤�ɂ����B
   tgt��������Ȃ�����NULL��Ԃ��B */
char const *GetString(char const *line, char *dst, size_t dat_size, char const *tgt)
{
	int	i;

	char const *delim_ptr;
	char const *value_ptr;
#ifndef GSTR2
	int tgt_len = strlen(tgt);
#endif
	int line_len;
	int value_len;
	int value_start;

	for(i=0;i<MAX_QUERY_STRING;i++)
	{
		/* �s���������� */
		delim_ptr = strchr( line, GETSTRING_LINE_DELIM );
		if ( delim_ptr )
			line_len = delim_ptr - line;
		else
			line_len = strlen(line);

		/* '='�������� */
		value_ptr = memchr( line, GETSTRING_VALUE_DELIM, line_len );
		if ( value_ptr ) {
			value_start = value_ptr + 1 - line;
#ifdef GSTR2
			/* �ŏ��̈ꕶ���̈�v */
			if ( *line == *tgt )
#else
			/* ���S��v */
			if ( tgt_len == (value_start-1) && !memcmp(line, tgt, tgt_len) )
#endif
			{
				/* �l�����̊J�n�ʒu�ƒ��� */
				value_len = line_len - value_start;

				/* �������ۂ߂� */
				if ( value_len >= dat_size )
					value_len = dat_size - 1;

				/* �l���R�s�[ */
				memcpy( dst, line + value_start, value_len );
				dst[value_len] = '\0';

				return line + line_len + (delim_ptr != NULL);	/* skip delim */
			}
		}

		if ( !delim_ptr )
			break;

		line = delim_ptr + 1;	/* skip delim */
	}
	return	NULL;
}
/****************************************************************/
/*								*/
/****************************************************************/
int IsBusy2ch(void)
{
	if (tm_now.tm_hour < LIMIT_AM || LIMIT_PM <= tm_now.tm_hour)
		return 1;
	return 0;
}
/****************************************************************/
/*								*/
/****************************************************************/
 /* src���̈�Ԗ����̃f���~�^������ c �̎��̕����ʒu�𓾂�
  */
char const *LastChar(char const *src, char c)
{
	char const *p;
	p = strrchr(src, c);

	if (p == NULL)
		return src;
	return (p + 1);
}

#ifdef	CHUNK_ANCHOR
/* first-last�܂ł�CHUNKED anchor��\��
   first��last�̓��X�ԍ��Bfirst��0�͓n���ȁ[ */
static void html_thread_anchor(int first, int last)
{
	int line = ((first - 1)/ CHUNK_NUM) * CHUNK_NUM + 1;
	if (first <= last) {
#ifdef	CHUNKED_ANCHOR_WITH_FORM
		pPrintf(pStdout, CHUNKED_ANCHOR_SELECT_HEAD("%s", "%s"),
			zz_bs, zz_ky);
		for ( ; line <= last; line += CHUNK_NUM) {
			pPrintf(pStdout, CHUNKED_ANCHOR_SELECT_STARTNUM("%d"),
			line);
		}
		pPrintf(pStdout, CHUNKED_ANCHOR_SELECT_TAIL);
#else
		for ( ; line <= last; line += CHUNK_NUM) {
			pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR("%s", "%d"),
				create_link(line, 
					line + CHUNK_NUM - 1, 
					0,0,0),
				line);
		}
#endif
	}
}
#else
#define	html_thread_anchor(first, last)		/* (void)0   nothing */
#endif	/* SEPARATE_CHUNK_ANCHOR */

/* �ŏ��ƍŌ�ɕ\������郌�X�ԍ���Ԃ�(���X�P������)
   isprinted�Ɠ���������B
*/
#if defined(SEPARATE_CHUNK_ANCHOR) || defined(PREV_NEXT_ANCHOR)
static int calc_first_line(void)
{
	if (nn_st)
		return nn_st;
	if (nn_ls)
		return lineMax - nn_ls + 1;
	return 1;
}
static int calc_last_line(void)
{
	int line = lineMax;

	if (nn_to && nn_to < lineMax)
		line = nn_to;
	/* 1-�Ōv�Z�Ԉ���Ă邪�A���̕����s�������� */
	if (is_imode()) {
		int imode_last = first_line + RES_IMODE - 1 + is_nofirst();
		if (imode_last < line)
			line = imode_last;
	} else if (isbusytime) {
		int busy_last = first_line + RES_NORMAL - 1 + is_nofirst();
		if (busy_last < line)
			line = busy_last;
	}
	return line;
}

void calc_first_last(void)
{
	/* �K�� first_line ���Ɍv�Z���� */
	first_line = calc_first_line();
	last_line = calc_last_line();
}
#else
#define calc_first_last()
#endif


/****************************************************************/
/*	HTML HEADER						*/
/****************************************************************/
void html_head(int level, char const *title, int line)
{
#ifdef USE_INDEX
	if (level) {
		pPrintf(pStdout, R2CH_HTML_DIGEST_HEADER_2("%s"),
			title);
		/* ���ꂾ���o�͂��Ă��ǂ� */
		return;
	}
#endif

	pPrintf(pStdout, R2CH_HTML_HEADER_0);
	header_base_out();
	zz_init_parent_link();
	zz_init_cgi_path();
	calc_first_last();

	if (!is_imode()) {	/* no imode       */
		pPrintf(pStdout, R2CH_HTML_HEADER_1("%s", "%s"),
			title, zz_parent_link);

	/* ALL_ANCHOR �͏�ɐ����ɂ���
	   �������ACHUNK_ANCHOR�������ŁA����isbusytime�ɂ͕\�����Ȃ� */
#if defined(CHUNK_ANCHOR) || defined(PREV_NEXT_ANCHOR)
		if (!isbusytime)
#endif
			pPrintf(pStdout, R2CH_HTML_ALL_ANCHOR("%s"),
				create_link(0,0,0,0,0) );

#if defined(PREV_NEXT_ANCHOR) && !defined(CHUNK_ANCHOR)
		pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR("%s", "1"),
			create_link(1,CHUNK_NUM,0,0,0) );
		if (first_line>1) {
			pPrintf(pStdout, R2CH_HTML_PREV("%s", "%d"),
				create_link((first_line<=CHUNK_NUM ? 1 : first_line-CHUNK_NUM),
					first_line-1,
					0,0,0),
				CHUNK_NUM );
		}
		pPrintf(pStdout, R2CH_HTML_NEXT("%s", "%d"),
			create_link(last_line+1,
				last_line+CHUNK_NUM,0,0,0),
			CHUNK_NUM );
#endif
#ifdef	SEPARATE_CHUNK_ANCHOR
		html_thread_anchor(1, first_line-1);
#else
		html_thread_anchor(1, lineMax);
#endif

		/* LATEST_ANCHOR�͏�� */
		pPrintf(pStdout, R2CH_HTML_LATEST_ANCHOR("%s", "%d"),
			create_link(0,0, LATEST_NUM, 0,0),
			LATEST_NUM);
	} else {
		pPrintf(pStdout, R2CH_HTML_IMODE_HEADER_1("%s", "%s", "%s"),
			title,
			zz_parent_link,
			create_link(1,RES_IMODE, 0,0,0) );
		pPrintf(pStdout, R2CH_HTML_IMODE_HEADER_2("%s", "%d"),
			create_link(0, 0, 0 /*RES_IMODE*/, 1,0),	/* imode�̓X����ls=10���� */
			RES_IMODE);
	}

	if (line > RES_RED) {
		pPrintf(pStdout, R2CH_HTML_HEADER_RED("%d"), RES_RED);
	} else if (line > RES_REDZONE) {
		pPrintf(pStdout, R2CH_HTML_HEADER_REDZONE("%d", "%d"),
			RES_REDZONE, RES_RED);
	} else if (line > RES_YELLOW) {
		pPrintf(pStdout, R2CH_HTML_HEADER_YELLOW("%d", "%d"),
			RES_YELLOW, RES_RED);
	}

#ifdef CAUTION_FILESIZE 
	if (line > RES_RED )
		;
	else
	if (zz_fileSize > MAX_FILESIZE - CAUTION_FILESIZE * 1024) { 
		pPrintf(pStdout, R2CH_HTML_HEADER_SIZE_REDZONE("%dKB", "%dKB", ""),
			MAX_FILESIZE/1024 - CAUTION_FILESIZE, MAX_FILESIZE/1024); 
	} 
# ifdef MAX_FILESIZE_BUSY 
	else if (zz_fileSize > MAX_FILESIZE_BUSY - CAUTION_FILESIZE * 1024) { 
		pPrintf(pStdout, R2CH_HTML_HEADER_SIZE_REDZONE_TIME("%dKB", "%dKB"), 
			MAX_FILESIZE_BUSY/1024 - CAUTION_FILESIZE, MAX_FILESIZE_BUSY/1024 ); 
	} 
# endif 
#endif 

	if (is_imode())
		pPrintf(pStdout, R2CH_HTML_HEADER_2_I("%s"), title);
	else
		pPrintf(pStdout, R2CH_HTML_HEADER_2("%s"), title);
}

/****************************************************************/
/*	RELOAD						        */
/****************************************************************/
#ifdef RELOADLINK
void html_reload(int startline)
{
	if (is_imode())	/*  imode */
#ifdef PREV_NEXT_ANCHOR
		return;
#else
		/* PREV_NEXT���@�\���s */
		pPrintf(pStdout, R2CH_HTML_RELOAD_I("%s"),
			create_link(startline,0, 0, 1,0) );
#endif
	else {
#ifdef PREV_NEXT_ANCHOR
		if (last_line<lineMax) {
			if (isbusytime) return;	/* ���G���͎�100�ɂ܂����� */
			pPrintf(pStdout, R2CH_HTML_AFTER("%s"),
				create_link(last_line+1,0, 0, 0,0) );

		} else
#endif
		{
			pPrintf(pStdout, R2CH_HTML_RELOAD("%s"),
				create_link(startline,0, 0, 0,0) );
		}
	}
}
#endif
/****************************************************************/
/*	HTML FOOTER						*/
/****************************************************************/
static void html_foot(int level, int line, int stopped)
{
#ifdef PREV_NEXT_ANCHOR
	int nchunk;
#endif
	/* out_resN = 0;	�_�C�W�F�X�g�p�ɍď����� */
	if (is_imode()) {
		html_foot_im(line,stopped);
		return;
	}
#if defined(PREV_NEXT_ANCHOR) || defined(RELOADLINK)
	pPrintf(pStdout, "<hr>");
#endif
#ifdef PREV_NEXT_ANCHOR
	if (!isbusytime) {
		pPrintf(pStdout, R2CH_HTML_RETURN_BOARD("%s"),
			zz_parent_link);
		pPrintf(pStdout, R2CH_HTML_ALL_ANCHOR("%s"),
			create_link(0,0,0,0,0) );
	}

#ifndef RELOADLINK
	pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR("%s", "1"),
		create_link(1,CHUNK_NUM,0,0,0) );
#endif
	if (!isbusytime && first_line>1) {
		pPrintf(pStdout, R2CH_HTML_PREV("%s", "%d"),
			create_link((first_line<=CHUNK_NUM ? 1 : first_line-CHUNK_NUM),
				first_line-1, 0,0,0),
			CHUNK_NUM );
	}
	if (isbusytime && need_tail_comment)
		nchunk = RES_NORMAL;
	else
		nchunk = CHUNK_NUM;
#ifdef RELOADLINK
	if (!isbusytime || last_line<lineMax) {
#else
	if (last_line < lineMax) {
#endif
		pPrintf(pStdout, R2CH_HTML_NEXT("%s", "%d"),
			create_link(last_line+1,
				last_line+nchunk, 0,0,0),
			nchunk );
#ifndef RELOADLINK
	} else {
		pPrintf(pStdout, R2CH_HTML_NEW("%s"),
			create_link(last_line, 0, 0,0,0) );
	}
#endif
#ifndef SEPARATE_CHUNK_ANCHOR
	pPrintf(pStdout, R2CH_HTML_LATEST_ANCHOR("%s", "%d"),
		create_link(0,0, LATEST_NUM, 0,0),
		LATEST_NUM);
#endif
	if (isbusytime && need_tail_comment)
		pPrintf(pStdout, R2CH_HTML_TAIL_SIMPLE("%02d:00", "%02d:00"),
			LIMIT_PM - 12, LIMIT_AM);
#ifdef RELOADLINK
	}
#endif
#endif

#ifdef	SEPARATE_CHUNK_ANCHOR
#if !defined(RELOADLINK) && !defined(PREV_NEXT_ANCHOR)
	pPrintf(pStdout, "<hr>");
#endif
	if (last_line < lineMax) {
		/* RELOADLINK�̕\�������̋t�Ȃ񂾂��� */
		html_thread_anchor(last_line + 1, lineMax - LATEST_NUM);
		if (!(isbusytime && out_resN > RES_NORMAL)) {
			/* �ŐV���Xnn�����Ԃ�̂ŋ���̍�
			   LATEST_ANCHOR�𐶂��ɂ��āA�Ȃ��������Ɏ����Ă��Ă���̂�
			   out_html���́@R2CH_HTML_TAIL���C������ق���
			   �����̗���Ƃ��Ă͖]�܂������A
			   �u���G����CHUNK_ANCHOR���\���ɂ���v���̏ꍇ�ɂ�
			   �ďC�����K�v�Ȃ̂ŕۗ� */
			/* LATEST_ANCHOR����ɐ����ɂ��� */
			pPrintf(pStdout, R2CH_HTML_LATEST_ANCHOR("%s", "%d"),
				create_link(0,0, LATEST_NUM, 0,0),
				LATEST_NUM);
		}
	}
#endif
	if (line <= RES_RED && !stopped) {
		pPrintf(pStdout, R2CH_HTML_FORM("%s", "%s", "%s", "%ld"),
			zz_cgi_path,
			zz_bs, zz_ky, currentTime);
	}

#ifdef USE_INDEX
	if (level)
		pPrintf(pStdout, R2CH_HTML_DIGEST_FOOTER);
	else
#endif
		pPrintf(pStdout, R2CH_HTML_FOOTER);
}
/****************************************************************/
/*	HTML FOOTER(i-MODE)					*/
/****************************************************************/
void html_foot_im(int line, int stopped)
{
#ifdef PREV_NEXT_ANCHOR
	if (last_line < lineMax) {
		pPrintf(pStdout, R2CH_HTML_NEXT("%s", "%d"),
			create_link(last_line+1,
				last_line+RES_IMODE, 0,1,0),
			RES_IMODE );
	} else {
		pPrintf(pStdout, R2CH_HTML_NEW_I("%s"),
			create_link(last_line,
				last_line+RES_IMODE-1, 0,1,0) );
	}
	if ( first_line>1) {
		pPrintf(pStdout, R2CH_HTML_PREV("%s", "%d"),
			create_link((first_line<=RES_IMODE ? 1 : first_line-RES_IMODE),
				first_line-1, 0,1,0),
			RES_IMODE );
	}
	pPrintf(pStdout, R2CH_HTML_IMODE_TAIL2("%s", "%d"),
		create_link(0, 0, 0 /*RES_IMODE*/, 1,0),	/* imode�̓X����ls=10���� */
		RES_IMODE);
#endif
	if (line <= RES_RED && !stopped ) {
		pPrintf(pStdout, R2CH_HTML_FORM_IMODE("%s", "%s", "%s", "%ld"),
			zz_cgi_path,
			zz_bs, zz_ky, currentTime); 
	}
	pPrintf(pStdout, R2CH_HTML_FOOTER_IMODE);
}

/****************************************************************/
/*	�ߋ����Opath����					*/
/****************************************************************/
void kako_dirname(char *buf, const char *key)
{
	int tm = atoi(key) / 100000;
	if (tm<10000) {
	    /*  9aabbbbbb -> 9aa */
	    sprintf(buf, "%03d", tm / 10);
	} else {
	    /* 1aaabccccc -> 1aaa/1aaab */
	    sprintf(buf, "%d/%d", tm / 10, tm);
	}
}

/****************************************************************/
/*	END OF THIS FILE					*/
/****************************************************************/
