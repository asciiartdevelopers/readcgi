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
# include	<zlib.h>
#endif

#if defined(GZIP) && !defined(ZLIB)
#include        <sys/wait.h>
static int pid;
#endif

#include	"r2chhtml.h"
#ifdef PREVENTRELOAD
# ifndef FORCE_304_TIME
#  define FORCE_304_TIME  30    /* �b�Ŏw�� */
# endif
# include        "util_date.h" /* from Apache 1.3.20 */
#endif

#if	(defined(CHUNK_ANCHOR) && CHUNK_NUM > RES_NORMAL) 
# error "Too large CHUNK_NUM!!"
#endif

/* CHUNK_ANCHOR �̃R�[�h�Ɉˑ����Ă��� */
#if defined(SEPARATE_CHUNK_ANCHOR) || defined(CHUNKED_ANCHOR_WITH_FORM)
# define CHUNK_ANCHOR
#endif

#if defined(ALWAYS_PATH) && !defined(USE_PATH)
# undef ALWAYS_PATH /* USE_PATH����`����Ă��Ȃ����ALWAYS_PATH�͖��� */
#endif

/* ��TERI�^�C�v��','���u������Ċi�[����镶���� */
#define COMMA_SUBSTITUTE "\x81\x97\x81\x4d" /* "���M" */
#define COMMA_SUBSTITUTE_FIRSTCHAR 0x81
#define COMMA_SUBSTITUTE_LEN 4

int zz_head_request; /* !0 = HEAD request */
char const *zz_remote_addr;
char const *zz_remote_host;
char const *zz_http_referer;
#ifdef ALWAYS_PATH
char const *zz_server_name;
char const *zz_script_name;
#endif

#ifdef USE_PATH
char const *zz_path_info;
/* 0 �̂Ƃ��́Apath�͓K�p����Ă��Ȃ�
   read.cgi/tech/998845501/ �̂Ƃ��́A3�ɂȂ� */
int path_depth;
#endif
char const *zz_query_string;
char *zz_temp;
char const *zz_http_user_agent;
char const *zz_http_language;
#ifdef GZIP
char const *zz_http_encoding;
enum compress_type_t {
	compress_none,
	compress_gzip,
	compress_x_gzip,
} gzip_flag;
#endif
#ifdef CHECK_MOD_GZIP
char const *zz_server_software;
#endif

char const *zz_http_if_modified_since;
time_t zz_fileLastmod;
char lastmod_str[1024];

#ifdef USE_MMAP
static int zz_mmap_fd;
static size_t zz_mmap_size;
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
int nn_st, nn_to, nn_ls;
char *BigBuffer = NULL;
char const *BigLine[RES_RED + 16];

#define is_imode() (*zz_im == 't')
#define is_nofirst() (*zz_nf == 't')
#define is_head() (zz_head_request != 0)

char *KARA = "";
int zz_fileSize = 0;
int lineMax = -1;
int out_resN = 0;

time_t t_now;
struct tm tm_now;
long currentTime;
int isbusytime = 0;

char const *LastChar(char const *src, char c);
char *GetString(char const *line, char *dst, size_t dst_size, char const *tgt);
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
#ifdef	TYPE_TERI
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
#ifdef	TYPE_TERI
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
	____3_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E0-E7 */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  E8-EF */
	___23_,___23_,___23_,___23_,___23_,___23_,___23_,___23_, /*  F0-F7 */
	___23_,___23_,___23_,___23_,___23_,______,______,______, /*  F8-FF */
};

typedef struct { /*  class... */
	char **buffers; /* csv�̗v�f */
	int rest; /* �c��̃o�b�t�@�T�C�Y�E�E�����ɂ͔��肵�ĂȂ��̂ŁA���o�C�g�͗]�T���~���� */
	int isTeri;
} ressplitter;

/* read.cgi�Ăяo����LINK��쐬 */
/* ��� pPrintf() �ň�x�����g���Ă͂����Ȃ� */
/* st,to,ls,nf�́A���ꂼ��̌Ăѐ�Bnf=1��nofirst=true
** �g��Ȃ����̂́A0�ɂ��ČĂ�
** sst�́ACHUNK_LINK�̏ꍇ��#�ԍ�
*/
const char *create_link(int st, int to, int ls, int nf, int sst)
{
	static char url_expr[128];
	static char *url_p = NULL;
	char *p;

#ifdef USE_PATH
	if (path_depth) {
		p = url_expr;
#ifdef USE_INDEX
		if (path_depth == 2) {
			p += sprintf(p, "%.40s/", zz_ky );
		}
#endif
		if (ls) {
			p += sprintf(p,"l%d",ls);
		} else if (to==0) {
			if (st<=1) {
#ifdef USE_INDEX
				if(path_depth!=2)
#endif
					p += sprintf(p, "./" ); /* �S�� */
			} else
				p += sprintf(p,"%d-",st);
			
		} else {
			if ( st != to ) {
				if ( st > 1 )
					p += sprintf(p, "%d", st); /* �n�_ */
				*p++ = '-';
			}
			p += sprintf(p, "%d", to); /* �I�_ */
		}
		if (nf && (st!=to || ls))	/* �P�_��'n'�s�v */
			*p++ = 'n';
		if (is_imode())
			*p++ = 'i';
#ifdef CREATE_NAME_ANCHOR
		if (sst && sst!=st) {
			p += sprintf(p,"#%d",sst);
		}
#endif
	} else
#endif	/* USE_PATH */
	{
		if (url_p==NULL) {	/* ��x������� key�͒��߂� */
			url_p = url_expr;
			url_p += sprintf(url_p, "\"" CGINAME "?bbs=%.20s&key=%.40s", zz_bs, zz_ky);
		}
		p = url_p;
		if (ls) {
			p += sprintf(p,"&ls=%d",ls);
		} else {
			if ( st > 1 )
				p += sprintf(p, "&st=%d", st);
			if ( to )
				p += sprintf(p, "&to=%d", to);
		}
		if (nf && (st>1 || ls) ) {		/* 1���܂ނƂ��͕s�v */
			p += sprintf(p, NO_FIRST );
		}
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
	*p = '\0';
	return url_expr;
}

/* �f���ɖ߂��LINK��쐬 */
void zz_init_parent_link(void)
{
	char * p = zz_parent_link;

#ifdef USE_PATH
#ifdef USE_INDEX
	if (path_depth==2)
		p += sprintf(p,"../");
	else
#endif
	if (path_depth)
		p += sprintf(p,"../../../../%.20s/",zz_bs);
	else
#endif
		p += sprintf(p,"../%.20s/",zz_bs);
	if (is_imode() ) {
		strcpy(p,"i/");
		return;
	}
#ifdef CHECK_MOD_GZIP
	if (zz_server_software && strstr(zz_server_software,"mod_gzip/")!=NULL)
		return;
#endif
#ifdef GZIP
	if (gzip_flag)
		strcpy(p,"index.htm");
#endif
}

/* bbs.cgi��LINK��쐬 */
void zz_init_cgi_path(void)
{
	char * p = zz_cgi_path;
#ifdef USE_PATH
#ifdef USE_INDEX
	if (path_depth==2)
		p += sprintf(p,"../../");
	else
#endif
	if (path_depth)
		p += sprintf(p,"../../../");
	else
#endif
		*p = '\0';
}

/*
  ������
  toparray �|�C���^�z��̃A�h���X
  buff �R�s�[��̃o�b�t�@�̐擪
  bufsize �����ɂ͔��肵�ĂȂ��̂ŁA���o�C�g�͗]�T���~����
  */

void ressplitter_init(ressplitter *This, char **toparray, char *buff, int bufsize/*, bool isteri*/)
{
	This->buffers = toparray;
	This->rest = bufsize;
	This->isTeri = true; /* ���X�P��strstr("<>")�������ʂ�ݒ肷�ׂ� */
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
	int n;
	int f_processed = 0;

	/* ���ʒu��T�� */
	n = strcspn(*sp, ">");
	if (n == 0)
		return 0;
	s += n + 1;	/* �܂�d�͐i�߂Ȃ��ł��� */
	if (!memcmp(s, "&gt;&gt;", 8)) {
		char const * copy_start = s + 8; /* &gt;&gt;���X�L�b�v */
		int copy_len;
		int st, to;
		int mst, mto;
		char buf[8];
		s += 8;
		/* 1�Ԗڂ̐������������� */
		n = strspn(s, "0123456789");
		if (n == 0)
			return 0;
		if (n > sizeof(buf) - 1)
			n = sizeof(buf) - 1;
		strncpy(buf, s, n);
		s += n;
		buf[n] = 0; /* sentinel */
		st = to = atoi(buf);
		f_processed = 1;
		/* 2�Ԗڂ̐���������Ȃ�A�������� */
		if (s[0] == '-') {
			n = strspn(++s, "0123456789");
			if (n == 0)
				return 0;
			if (n > sizeof(buf) - 1)
				n = sizeof(buf) - 1;
			strncpy(buf, s, n);
			s += n;
			buf[n] = 0; /* sentinel */
			to = atoi(buf);
		}
		/* </a>������͂��Ȃ̂ŒT���A�̂Ă� */
		s = strstr(s, "</a>");
		if (!s)
			return 0;
		copy_len = s - copy_start;
		s += 4; /* strlen("</a>") */

		if (is_imode()
		|| (st == 0 || to == 0 || st > lineMax || to > lineMax)
#if	1	/* #ifndef USE_PATH */
		/* #ifndef �𐶂����΁Aisbusytime�ł�a name=�ւ̃����N����*/
		/* USE_PATH�łȂ��Ă��Aa name=��anchor�����悤�ɂȂ���??	*/
		|| (istagcut && isprinted(st) && isprinted(to))
#endif
		)
		{
			// �^�O���� /
		} else
		{
			copy_len = s - copy_start;
#ifdef CREATE_NAME_ANCHOR
			/* �V�����\�����u�`���� */
			if (isprinted(st) && isprinted(to))
			{
				d += sprintf(d,
					     "<a href=#%u>", 
					     st);
			} else
#endif
			{
				/* chunk�d�l�𐶂������߂�kludge�͈ȉ��ɁB */
#if defined(CHUNK_ANCHOR) && defined(CREATE_NAME_ANCHOR) && defined(USE_CHUNK_LINK)
				mst = (st - 1) / CHUNK_NUM;
				mto = (to - 1) / CHUNK_NUM;

				if (mst == mto && (st != 1 || to != 1) ) {
					/* chunk�͈� */
					mst = 1 + CHUNK_NUM * mst;
					mto = CHUNK_NUM * (mto + 1);
				} else 
#endif
				{
					/* chunk���܂��������Ȃ̂ŁA�ŏ��P�ʂ��B*/
					mst = st;
					mto = to;
				}

				d += sprintf(d, "<a href=%s " TARGET_BLANK ">", 
#if defined(CHUNK_ANCHOR) && defined(CREATE_NAME_ANCHOR) && defined(USE_CHUNK_LINK)
					create_link(mst, mto, 0, 0, st)
#else
					create_link(mst, mto, 0, 1, st)
#endif
					);
			}
		}

		/* "&gt;&gt;"�� >> �ɒu�������A�Â�.."</a>"���ێʂ� */
		*d++ = '>';
		*d++ = '>';
		memcpy( d, copy_start, copy_len );
		d += copy_len;
	}

	/* ���Ƃ��܂� */
	if (f_processed) {
		/* �|�C���^���i�񂾂͂� */
		*sp = s;
	} else {
		/* �P���ɑ��������� */
		s = *sp;
		n = strcspn(s, ">");
		if (n == 0)
			return 0;
		n++;
		if (!istagcut) {
			memcpy(d, s, n);
			d += n;
		}
		s += n;
		*sp = s;
	}
	*dp = d;
	return 1;
}
/*
	p�́A"http://"��':'���w���Ă���
	�����������̂ڂ邩��Ԃ��B0�Ȃ�non-match
	�S�����O(http�̏ꍇ)����X�L�������Ă���̂ŁA
	���S���m�F���Ă���Ă�
*/
int isurltop(const char *p)
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
int geturltaillen(const char *p)
{
	const char *top = p;
	int len = 0;
	while (!hrefStop(*p)) {
		if (*p == '&') {
			/* &quot;�ň͂܂ꂽURL�̖������� */
			if (strncmp(p, "&quot;", 6) == 0)
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
int urlcopy(char *bufp, const char *url, int urllen)
{
	return sprintf(bufp,
		"<a href=\"%.*s\" " TARGET_BLANK ">%.*s</a>", 
		/* �������ɂ��Ă����[�[�[���������k���ʂ����҂����� */
		/* "<A HREF=\"%.*s\" TARGET=\"_blank\">%.*s</A>", */
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
	}
	return true;
}

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
#ifdef NORMAL_TAGCUT
	int istagcut = (resnumber > 1) || is_imode();
#else
	int istagcut = (LINKTAGCUT && isbusytime && resnumber > 1) || is_imode();
#endif
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
					if (bufp == *This->buffers) /* ���O�������p�󔒂̏ꍇ */
						*bufp++ = ' ';
					p += 3;
					goto Teri_Break;
				}
				if (memcmp(p, " <br> ", 6) == 0) {
					if (bufp != *This->buffers && isSJIS1(*(bufp-1))) {
						*bufp++ = ' ';
					}
					memcpy(bufp, "<br>", 4);
					p += 6;
					bufp += 4;
					continue;
				}
				break;
			case '<': /*  �X���� */
				if (*(p+1) == '>') {
					/* This->isTeri = true; */
					p += 2;
					goto Teri_Break;
				}
#ifdef	RAWOUT
				/* rawmode != 0 �̎��͌Ă΂�Ă��Ȃ��悤�Ɏv�����ǁE�E?? */
				if (rawmode)
					break;
#endif
				if (resnumber && p[1] == 'a' && isspace(p[2])) {
					char *tdp = bufp;
					char const *tsp = p;
					if (!rewrite_href(&tdp, &tsp, istagcut))
						goto Break;
					bufrest -= tdp - bufp;
					bufp = tdp;
					p = tsp;
					continue;
				}
				break;
			case '&':
				if (memcmp(p+1, "amp", 3) == 0) {
					if (*(p + 4) != ';')
						p += 4 - 1;
				}
				/* &MAIL->&amp;MAIL�̕ϊ��́Adat���ǂ݂̂�����`���Ⴊ�Ή����Ȃ��Ɩ���
				   �����ϊ�����Ȃ�bbs.cgi */
				break;
			case ':':
#ifndef NO_LINK_URL_BUSY
				if (resnumber) {
#else
				if (resnumber && !istagcut) {
					/* url�̃����N��(���ԑтɂ����)�p�~����Ȃ� */
#endif
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
#ifndef TYPE_TERI
			case COMMA_SUBSTITUTE_FIRSTCHAR: /*  *"��"(8197)  "�M"(814d) */
				/* if (!This->isTeri) { */
				if (memcmp(p, COMMA_SUBSTITUTE, COMMA_SUBSTITUTE_LEN) == 0) {
					ch = ',';
					p += 4 - 1;
				}
				/* } */
				break;
			case ',':
				/* if (!This->isTeri) { */
				p++;
				goto Break;
				/* } */
				/* break; */
#endif
			case '\0':
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
	*bufp++ = '\0';
	This->rest -= bufp - *This->buffers;
	*++This->buffers = bufp;
	
	/* ��؂薖�̋󔒂��폜 */
	if (*p == ' ')
		++p;
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
}


/****************************************************************/
/*	BadAccess						*/
/* ����=0, �s����=1��Ԃ�                                       */
/****************************************************************/
int BadAccess(void)
{
	char *agent_kick[] = {
#ifdef	Katjusha_Beta_kisei
		"Katjusha",
#endif
		"WebFetch",
		"origin",
		"Nozilla",
		"WWWD",
	};
	int i;

	if ( is_head() )
		return 0;
#if defined(GZIP) && defined(RAWOUT)
	if ( rawmode )
		return !gzip_flag;
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
int logOut(char *txt)
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
	char *s[20];
	char p[SIZE_BUF];

	if (out_resN)
		return 0;
	splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
	if (!*p)
		return 1; 
	html_head(level, s[4], lineMax);
	out_resN++;
	return 0;
}
/****************************************************************/
/*	Get file size(out_html)					*/
/****************************************************************/
static int out_html(int level, int line, int lineNo)
{
	char *s[20];
	char *r0, *r1, *r3, *r4;
	char p[SIZE_BUF];

#ifdef	CREATE_NAME_ANCHOR
#define	LineNo_			lineNo, lineNo
#else
#define	LineNo_			lineNo
#endif

/*printf("line=%d[%s]<P>\n",line,BigLine[line]);return 0;*/

	if (!out_resN) {	/* Can I write header ?   */
		splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
		if (!*p)
			return 1; 
		r4 = s[4];
		html_head(level, r4, lineMax);
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
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL("%s", "%d"),
				create_link(lineNo, 
					lineNo + RES_IMODE, 0,0,0),
				RES_IMODE);
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL2("%s", "%d"),
				create_link(0, 0, RES_IMODE, 1,0),
				RES_IMODE);
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
	/* �����񍐂��ꂽ�ŏI���X�ԍ�����уT�C�Y����v���Ă��Ȃ�
	   ��΁A�ŏ��̍s�ɂ��̎|������ */
	/* raw_lastsize > 0 �ɂ����nnn.0�ł��ځ[�񌟏o�𖳌��ɂł��邪
	   �T�[�o�[�ō폜�������̂̓N���C�A���g�ł��폜�����ׂ��ł��� */
	if(raw_lastnum > 0
		&& raw_lastsize >= 0
		&& !(raw_lastnum <= lineMax
		 && BigLine[raw_lastnum] - BigBuffer == raw_lastsize)) {
		pPrintf(pStdout, "-INCR");
		/* �S���𑗐M����悤�ɕύX */
	} else {
		pPrintf(pStdout, "+OK");
		/* �������M�p�ɐ擪��ݒ� */
		begin = BigLine[raw_lastnum];
	}
	pPrintf(pStdout, " %d/%dK\n", end - begin, MAX_FILESIZE / 1024);
	/* raw_lastnum ����S���𑗐M���� */
#ifdef ZLIB
	if (gzip_flag)
		gzwrite(pStdout, (const voidp)begin, end - begin);
	else
#endif
		fwrite(begin, 1, end - begin, pStdout);

	return 1;
}
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
	char *s[20]; 
	char p[SIZE_BUF]; 

	if (!isdigit(*zz_ky)) {
		threadStopped = 1;
		/* �ߋ����O��FORM��RELOADLINK����\���ɂ��邽�� */
	}
	for (line = 0; line < lineMax; line++) { 
		int lineNo = line + 1; 
		if (!isprinted(lineNo)) 
			continue; 
		if (out_html(level, line, lineNo)) { 
			line++; 
			break; /* ��0���Ԃ�̂́A�G���[����imode��Max�ɒB������ */ 
		} 
		if (lineNo==1 && is_imode() && nn_st<=1 && nn_ls==0) 
			++out_resN; 
	} 
	out_html1(level); /* ���X���P���\������Ă��Ȃ����Ƀ��X�P��\������ */ 

	splitting_copy(s, p, BigLine[lineMax-1], sizeof(p) - 20, lineMax-1);
	if (!*p)
		return 1; 
	if( s[2]!=0 && (strstr( s[2], "�X�g�b�p�[" ) || strstr( s[2], "��~" )) ) threadStopped=1;

	if ( !is_imode() )
		pPrintf(pStdout, R2CH_HTML_PREFOOTER);
#ifdef RELOADLINK
	if (
#ifdef USE_INDEX
	    !level && 
#endif
	    lineMax == line && lineMax <= RES_RED && !threadStopped) {
	
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
	zz_fileSize = getFileSize(fname);

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
	BigBuffer = mmap(NULL,
			 zz_mmap_size = zz_fileSize + 0x10000,
			 PROT_READ | PROT_WRITE,
			 MAP_PRIVATE,
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

	lineMax = getLineMax();
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
	char *p = BigBuffer;
	char *p1;

	if (!p)
		return -8;

	p1 = BigBuffer + zz_fileSize;	/* p1 = �Ō��\n�̎��̃|�C���^ */
	while (p < p1 && *(p1-1) != '\n')	/* �Ō�̍s����T�� �����dat�Ȃ��薳�� */
		p1--;
	if (p1 - p < 10)	/* �K�������ǁA���͏o�Ȃ��͂� */
		return -8;
	do {
		BigLine[line] = p;
		if (line > RES_RED)
			break;
		++line;
		p = (char *)memchr(p, '\n', p1-p) + 1;
	} while(p != p1);
	
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
time_t getFileLastmod(char *file)
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
#ifdef USE_PATH
static int get_path_info(char const *path_info)
{
	char const *s = path_info;
	int n;

	path_depth = 0;
	/* PATH_INFO �́A'/' �Ŏn�܂��Ă�悤�ȋC�������� */
	if (s[0] != '/')
		return 0;

	path_depth++;

	/* ��������PATH_INFO�͕|���̂Ŏ󂯕t���Ȃ� */
	if (strlen(s) >= 256)
		return 0;

	/* PATH_INFO�𑖍��B�{�[�h���Ȃǂ𔲂��o���B
	   ���܂��܂ȗ��R�ɂ��A'/'�̗L�����`�F�b�N���Ă��� */
	n = strcspn(++s, "/");
	if (n == 0)
		return 0;
	/* �� */
	strncpy(zz_bs, s, n);
	s += n;
	if (s[0] != '/') {
		/* XXX �����ŉ������������Bhehehe */
		return 0;
	}
	path_depth++;
	/* ���c */
	n = strcspn(++s, "/");
	if (n == 0)
		return 0;
	strncpy(zz_ky, s, n);	/* �p�����[�^��������Ȃ��̂Ŏ�荞�� */
	if (n == 0 || s[n] != '/')
		return 0;
	path_depth++;
	/* �X�� */
	s += n + 1;

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
			strcpy(zz_im,"true");
			s++;
		} else if (*s == '.') {
			strcpy(zz_nf,"false");
			s++;
		} else if (*s == 'n') {
			strcpy(zz_nf,"true");
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
				strcpy(zz_nf,"true");
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
		} else {
			/* �K�肳��ĂȂ������������̂ŕ]������߂� */
			break;
		}
	}

	if (zz_to[0] == '-') {
		/* �͈͎w��͂Ȃ��̂ŁA
		   �P�_�|�C���g�ƌ��Ȃ� */
		strcpy(zz_to, zz_st);
		if ( zz_st[0] == '-' ) {
			/* st�̎w����Ȃ����� */
			zz_to[0] = '\0';
		} else {
			/* �P�_�́Anofirst=true��default�� */
			if (!zz_nf[0]) {
				strcpy(zz_nf,"true");
			}
		}
	}
	if (zz_st[0] == '-') {
		/* st���Ȃ�����1���� */
		zz_st[0] = '\0';
	}
	if (zz_ls[0]) {
		/* ls��D�� */
		zz_st[0] = zz_to[0] = '\0';
	}

	/* �����͊����������̂Ƃ݂Ȃ� */
	return 1;
}
#endif

/****************************************************************/
/*	SETTING_R.TXT�̓ǂ݂���					*/
/****************************************************************/
#ifdef	USE_SETTING_FILE
void readSettingFile(const char *bbsname)
{
	char fname[1024];
	int fd;
	
	sprintf(fname, "../%.256s/%s", bbsname, SETTING_FILE_NAME);
	fd = open(fname, O_RDONLY);
	if (fd >= 0) {
		/* SETTING_R.TXT��ǂ� */
		char const *cptr;
		char const *endp;
		struct stat st;
#ifdef	USE_MMAP
		void *mmptr;
		fstat(fd, &st);
		mmptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
#else
		/* �܂��ݒ�t�@�C����8k�ȏ�����Ȃ���΂����Ƃ������Ƃ� */
		char mmptr[8192];
		st.st_size = read(fd, mmptr, 8192);
#endif
		for (cptr = mmptr, endp = cptr + st.st_size - 1;
		     cptr < endp && *endp != '\n'; endp--)
			;
		for ( ; cptr && cptr < endp;
		      cptr = memchr(cptr, '\n', endp - cptr), cptr?cptr++:0) {
			if (*cptr != '\n' && *cptr != '#' && *cptr != ';') {
				int i;
				for (i = 0; i < sizeof(SettingParam)
					     / sizeof(SettingParam[0]); i++) {
					int len = strlen(SettingParam[i].str);
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
#ifdef	USE_MMAP
#ifdef  EXPLICIT_RELEASE
		munmap(mmptr, st.st_size);
		close(fd);
#endif
#else
		/* mmptr == automatic variable... */
		/* free(mmptr); */
		close(fd);
#endif  /* USE_MMAP */
	}
}
#endif	/*	USE_SETTING_FILE	*/

/****************************************************************/
/*	GET Env							*/
/****************************************************************/
void zz_GetEnv(void)
{
	char const * request_method;
	currentTime = (long) time(&t_now);
	putenv("TZ=JST-9");
	tzset();
	tm_now = *localtime(&t_now);

	request_method = getenv("REQUEST_METHOD");
	zz_head_request = request_method && (strcmp(request_method, "HEAD") == 0);
	zz_remote_addr = getenv("REMOTE_ADDR");
	zz_remote_host = getenv("REMOTE_HOST");
	zz_http_referer = getenv("HTTP_REFERER");
#ifdef ALWAYS_PATH
	zz_server_name = getenv("HTTP_HOST");
	if (!zz_server_name)
		zz_server_name = getenv("SERVER_NAME");
	zz_script_name = getenv("SCRIPT_NAME");
#endif

#ifdef USE_PATH
	zz_path_info = getenv("PATH_INFO");
#endif
	zz_query_string = getenv("QUERY_STRING");
	zz_temp = getenv("REMOTE_USER");
	zz_http_user_agent = getenv("HTTP_USER_AGENT");
	zz_http_language = getenv("HTTP_ACCEPT_LANGUAGE");
#ifdef GZIP
	zz_http_encoding = getenv("HTTP_ACCEPT_ENCODING");
#endif
#ifdef CHECK_MOD_GZIP
	zz_server_software = getenv("SERVER_SOFTWARE");
#endif
	zz_http_if_modified_since = getenv("HTTP_IF_MODIFIED_SINCE");

	if (!zz_remote_addr)
		zz_remote_addr = KARA;
	if (!zz_remote_host)
		zz_remote_host = KARA;
	if (!zz_http_referer)
		zz_http_referer = KARA;
#ifdef USE_PATH
	if (!zz_path_info)
		zz_path_info = "";	/* XXX KARA���g���񂷂͕̂|�� */
#endif
	if (!zz_query_string)
		zz_query_string = KARA;
	if (!zz_temp)
		zz_temp = KARA;
	if (!zz_http_user_agent)
		zz_http_user_agent = KARA;
	if (!zz_http_language)
		zz_http_language = KARA;

#ifdef USE_PATH
	zz_bs[0] = zz_ky[0] = zz_ls[0] = zz_st[0] = '\0';
	zz_to[0] = zz_nf[0] = zz_im[0] = '\0';
	if (!get_path_info(zz_path_info)) {
		/* ����ȍ~�Apath ���t�^����Ă��邩�ǂ�����
		   ����� zz_path_info �̃e�X�g�ōs���Ă��� */
		zz_path_info = NULL;
	}
#endif
	GetString(zz_query_string, zz_bs, sizeof(zz_bs), "bbs");
	GetString(zz_query_string, zz_ky, sizeof(zz_ky), "key");
	GetString(zz_query_string, zz_ls, sizeof(zz_ls), "ls");
	GetString(zz_query_string, zz_st, sizeof(zz_st), "st");
	GetString(zz_query_string, zz_to, sizeof(zz_to), "to");
	GetString(zz_query_string, zz_nf, sizeof(zz_nf), "nofirst");
	GetString(zz_query_string, zz_im, sizeof(zz_im), "imode");
#ifdef RAWOUT
	zz_rw[0] = '\0';
	GetString(zz_query_string, zz_rw, sizeof(zz_rw), "raw");
#endif
	/* zz_ky �͒P�Ȃ�32�r�b�g���l�Ȃ̂ŁA
	   �ȍ~�A�����ł�������悤�ɂ��Ă��� */
	nn_ky = atoi(zz_ky);
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
		if ( gzip_flag == compress_x_gzip ) {
			puts("Content-Encoding: x-gzip");
		} else {
			puts("Content-Encoding: gzip");
		}
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
	if (BadAccess())	/* �u�Ȃ񂩕ςł��v�̏ꍇ��dat�̓ǂ݂��݂�����邽�� */
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

#ifdef	REFERDRES_SIMPLE
int can_simplehtml(void)
{
	char buff[1024];
	const char *p;
	const char *ref;
	static const char cginame[] = "/test/" CGINAME "?";
	static const char indexname[] = "index.htm";
	
	if (!isbusytime)
		return false;
	if (!nn_st || !nn_to || is_imode())
		return false;
	if (nn_st > nn_to || nn_to > lineMax)
		return false;
	/* �Ƃ肠�����A�����N��̃��X���P�̏ꍇ���� */
	if (nn_st != nn_to || !is_nofirst())
		return false;
	/*if (!(nn_st + 10 <= nn_to))
		return false; */
	ref = getenv("HTTP_REFERER");
	if (!ref || !*ref)
		return false;
	
	sprintf(buff, "/%.50s/", zz_bs);
	p = strstr(ref, buff);
	if (p) {
		p += strlen(buff);
		if (*p == '\0')
			return true;
		if (strncmp(p, indexname, sizeof(indexname)-1) == 0)
			return true;
	}
	p = strstr(ref, cginame);
	if (p) {
		char bbs[sizeof(zz_bs)];
		char key[sizeof(zz_ky)];
		const char * query_string = p + sizeof(cginame)-1;
		GetString(query_string, bbs, sizeof(bbs), "bbs");
		GetString(query_string, key, sizeof(key), "key");
		return (strcmp(zz_bs, bbs) == 0) && (strcmp(zz_ky, key) == 0);
	}
#ifdef	USE_PATH
	sprintf(buff, "/test/" CGINAME "/%.50s/%.50s/", zz_bs, zz_ky);
	if (strstr(ref, buff))
		return true;
#endif
	return false;
}

int out_simplehtml(void)
{
	char *s[20];
	char p[SIZE_BUF];
	int n = nn_st;
	
	/* html_head() */
	splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
	if (!*p)
		return 1;
	pPrintf(pStdout, R2CH_SIMPLE_HTML_HEADER_1("%s", ""), s[4]);
	pPrintf(pStdout, R2CH_HTML_HEADER_2("%s"), s[4]);
	
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

#if	('\xFF' != 0xFF)
#error	-funsigned-char required.
	/* ���̃V�X�e���ł́A-funsigned-char��v������ */
	if ((char)0xFF != (unsigned char)0xFF) {
		puts("Content-Type: text/plain\n\n"
		     "-funsigned-char required.");
		return 0;
	}
#endif

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
	if (st == 1 && to == 1)
		zz_nf[0] = '\0';
	if (is_imode()) {	/* imode */
		if (!st && !to && !ls)
			ls = RES_IMODE;
	}
	if (!is_nofirst() && ls > 0) {
		ls--;
		if(ls == 0) {
			ls = 1;
			strcpy(zz_nf, "true");
		}
	} else if (ls < 0)
		ls = 0;

	/* �����w�肳�ꂽ����ls��D�� */
	if (ls) {
		st = to = 0;
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

	sprintf(fname, DAT_DIR "%.256s.dat", zz_bs, zz_ky);
#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif
#ifdef USE_INDEX
	/* �X���ꗗ�����ɐ������[�h */
	if (1 <= path_depth && path_depth < 3) {
		sprintf(fname, "../%.256s/subject.txt", zz_bs);
		zz_fileLastmod = getFileLastmod(fname);
	}
#endif

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
		if(modtime != BAD_DATE
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

#ifdef GZIP
	if (zz_http_encoding && strstr(zz_http_encoding, "x-gzip")) {
		gzip_flag = compress_x_gzip;
	} else if (zz_http_encoding && strstr(zz_http_encoding, "gzip")) {
		gzip_flag = compress_gzip;
	} else {
		gzip_flag = compress_none;
	}
#ifndef ZLIB
	switch ( gzip_flag ) {
	case compress_x_gzip:
		puts("Content-Encoding: x-gzip");
		break;
	case compress_gzip:
		puts("Content-Encoding: gzip");
		break;
	}
#endif
#endif
	zz_init_parent_link();
	zz_init_cgi_path();

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

/*  Get Last-Modified Date */
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

	logOut("");

	dat_read(fname, st, to, ls);

#ifdef RAWOUT
	if (rawmode) {
		dat_out_raw();
		return 0;
	}
#endif
#ifdef USE_PATH
#ifdef USE_INDEX
	if (path_depth == 2) {
		if (zz_ky[0] == '-')
			dat_out_subback();	/* �X���ꗗ */
		else
			dat_out_index();	/* �_�C�W�F�X�g */
		return 0;
	}
#endif
	if (path_depth && path_depth!=3) {
		html_error(ERROR_NOT_FOUND);
		return 0;
	}
#endif
#ifdef	REFERDRES_SIMPLE
	if (can_simplehtml())
		out_simplehtml();
	else
#endif
		dat_out(0);
	return 0;
}
/****************************************************************/
/*	ERROR END(html_error)					*/
/****************************************************************/
void html_error(enum html_error_t errorcode)
{
	char zz_soko[256];
	char tmp[256];
	char doko[256];
	struct stat CountStat;
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
	default:
		mes = "";
	}

	*tmp = '\0';
	strcpy(tmp, LastChar(zz_ky, '/'));
	kako_dirname(zz_soko, tmp);
#ifdef RAWOUT
	if(rawmode) {
		/* -ERR (message)�̓G���[�B */
		if (errorcode == ERROR_NOT_FOUND) {
			sprintf(doko, KAKO_DIR "%.50s/%.50s.dat", zz_bs, zz_soko, tmp);
			if (!stat(doko, &CountStat)) {
				pPrintf(pStdout, "-ERR " ERRORMES_DAT_FOUND "\n", doko);
			} else {
				sprintf(doko, TEMP_DIR "%.50s.dat", zz_bs, tmp);
				if (!stat(doko, &CountStat)) {
					pPrintf(pStdout, "-ERR %s\n", ERRORMES_TEMP_FOUND);
				} else {
					pPrintf(pStdout, "-ERR %s\n", mes);
				}
			}
		} else
			pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
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
		sprintf(doko, KAKO_DIR "%.50s/%.50s.html", zz_bs,
			zz_soko, tmp);
		if (!stat(doko, &CountStat)) {
			pPrintf(pStdout, R2CH_HTML_ERROR_5_HTML, 
				zz_cgi_path, doko, tmp);
		} else {
			sprintf(doko, KAKO_DIR "%.50s/%.50s.dat",
				zz_bs, zz_soko, tmp);
			if (!stat(doko, &CountStat)) {
				pPrintf(pStdout, R2CH_HTML_ERROR_5_DAT,
					zz_cgi_path, doko, tmp);
			} else {
				sprintf(doko, TEMP_DIR "%.50s.dat",
					zz_bs, tmp);
				if (!stat(doko, &CountStat)) {
					pPrintf(pStdout, R2CH_HTML_ERROR_5_TEMP,
						tmp);
				} else {
					pPrintf(pStdout, R2CH_HTML_ERROR_5_NONE,
						zz_cgi_path, zz_bs);
				}
			}
		}
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_6);

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
int html_error999(char *mes)
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
char *GetString(char const *line, char *dst, size_t dat_size, char const *tgt)
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

				return dst;
			}
		}

		if ( !delim_ptr )
			break;

		line += line_len + 1; /* skip delim */
	}
#ifndef USE_PATH
	*dst = '\0';
#endif
	return	dst;
}
/****************************************************************/
/*								*/
/****************************************************************/
int whatKanji(char *str)
{
	int val = 0;
	unsigned char b1, b2, b3;

	b1 = *str++;
	b2 = *str++;
	b3 = *str;
	if (b1 == 0x1b) {
		if (b2 == '$' && b3 == 'B')
			return 16;
		if (b2 == '$' && b3 == '@')
			return 32;
		if (b2 == '(' && b3 == 'J')
			return 64;
		if (b2 == '(' && b3 == 'B')
			return 128;
		return 0;
	}
	if (b1 >= 0xa0 && b1 <= 0xdf)
		val |= 1;
	if (((b1 >= 0x81 && b1 <= 0x9f) || (b1 >= 0xe0 && b1 <= 0xfc))
	    && (b2 >= 0x40 && b2 <= 0xfc && b2 != 0x7f))
		val |= 2;
	if (b1 == 0x8e && (b2 >= 0xa0 && b2 <= 0xdf))
		val |= 4;
	if ((b1 >= 0xa1 && b1 <= 0xfe) && (b2 >= 0xa1 && b1 <= 0xfe))
		val |= 8;

	return val;
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

#ifdef	SEPARATE_CHUNK_ANCHOR
#ifndef	CHUNK_ANCHOR
#error	SEPARATE_CHUNK_ANCHOR needs CHUNK_ANCHOR
#endif
#endif
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
   imode���Ή�, isprinted�Ɠ���������B
*/
#if defined(SEPARATE_CHUNK_ANCHOR) || defined(PREV_NEXT_ANCHOR)
static int first_line()
{
	if (nn_st)
		return nn_st;
	if (nn_ls)
		return lineMax - nn_ls + 1;
	return 1;
}
static int last_line()
{
	/* html_foot���ĂԎ��ɍŏI�\���s��n���悤�ɂ���Ηv��Ȃ��񂾂��� */
	int line = lineMax;
	if (nn_to && nn_to < lineMax)
		line = nn_to;
	if (isbusytime) {
		int busy_last = first_line() + RES_NORMAL - 1 - is_nofirst();
		/* �ׂ����v�Z�Ԉ���Ă邩�� */
		if (busy_last < line)
			line = busy_last;
	}
	return line;
}
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
#ifdef ALWAYS_PATH
	if (path_depth < 3 && zz_server_name && zz_script_name) {
		pPrintf(pStdout, R2CH_HTML_BASE_DEFINE, zz_server_name, zz_script_name, zz_bs, zz_ky);
		path_depth = 3;
	}
#endif
	zz_init_parent_link();
	zz_init_cgi_path();

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
		if (first_line()>1) {
			pPrintf(pStdout, R2CH_HTML_PREV("%s", "%d"),
				create_link((first_line()<=CHUNK_NUM ? 1 : first_line()-CHUNK_NUM),
					first_line()-1,
					0,0,0),
				CHUNK_NUM );
		}
		pPrintf(pStdout, R2CH_HTML_NEXT("%s", "%d"),
			create_link(last_line()+1,
				last_line()+CHUNK_NUM,0,0,0),
			CHUNK_NUM );
#endif
#ifdef	SEPARATE_CHUNK_ANCHOR
		html_thread_anchor(1, first_line()-1);
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
			create_link(0,0, RES_IMODE, 1,0),
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
		pPrintf(pStdout, R2CH_HTML_HEADER_SIZE_REDZONE("%dKB", "%dKB", "���ԑтɂ���Ă�"), 
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
		pPrintf(pStdout, R2CH_HTML_RELOAD_I("%s"),
			create_link(startline,0, 0, 1,0) );
	else {
#ifdef PREV_NEXT_ANCHOR
		if (last_line()<lineMax) {
			if (isbusytime) return;	/* ���G���͎�100�ɂ܂����� */
			pPrintf(pStdout, R2CH_HTML_AFTER("%s"),
				create_link(last_line()+1,0, 0, 0,0) );

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
	if (!isbusytime && first_line()>1) {
		pPrintf(pStdout, R2CH_HTML_PREV("%s", "%d"),
			create_link((first_line()<=CHUNK_NUM ? 1 : first_line()-CHUNK_NUM),
				first_line()-1, 0,0,0),
			CHUNK_NUM );
	}
	if (isbusytime && need_tail_comment)
		nchunk = RES_NORMAL;
	else
		nchunk = CHUNK_NUM;
#ifdef RELOADLINK
	if (!isbusytime || last_line()<lineMax) {
#else
	if (last_line() < lineMax) {
#endif
		pPrintf(pStdout, R2CH_HTML_NEXT("%s", "%d"),
			create_link(last_line()+1,
				last_line()+nchunk, 0,0,0),
			nchunk );
#ifndef RELOADLINK
	} else {
		pPrintf(pStdout, R2CH_HTML_NEW("%s"),
			create_link(last_line(), 0, 0,0,0) );
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
	if (last_line() < lineMax) {
		/* RELOADLINK�̕\�������̋t�Ȃ񂾂��� */
		html_thread_anchor(last_line() + 1, lineMax - LATEST_NUM);
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
	int tm = atoi(key)/(1000*1000);
	if (tm<1000) {
	    /*  9aabbbbbb -> 9aa */
	    sprintf(buf, "%03d", tm);
	} else {
	    /* 1abbcccccc -> 1a/abb */
	    sprintf(buf, "%d/%03d", tm/100, tm%1000);
	}
}

/****************************************************************/
/*	END OF THIS FILE					*/
/****************************************************************/
