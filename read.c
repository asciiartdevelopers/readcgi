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

#include "digest.h"
#include "read.h"

#ifdef ZLIB
# ifndef GZIP
#  define GZIP			/* gzip�R���̃R�[�h���g�p����̂� */
# endif
# include	<zlib.h>
#endif

#if defined(GZIP) && !defined(ZLIB)
#include        <sys/wait.h>
#endif

#include	"r2chhtml.h"
#ifdef PREVENTRELOAD
# ifndef LASTMOD
#  define LASTMOD
# endif
# ifndef FORCE_304_TIME
#  define FORCE_304_TIME  30    /* �b�Ŏw�� */
# endif
# include        "util_date.h" /* from Apache 1.3.20 */
#endif

#if	(defined(CHUNK_ANCHOR) && CHUNK_NUM > RES_NORMAL) 
# error "Too large CHUNK_NUM!!"
#endif

char const *zz_remote_addr;
char const *zz_remote_host;
char const *zz_http_referer;
char const *zz_http_cookie;
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
int gzip_flag;
#endif

#ifdef LASTMOD
char const *zz_http_if_modified_since;
time_t zz_fileLastmod;
char lastmod_str[1024];
char expires_str[1024];
#endif

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
#ifdef RAWOUT
char zz_rw[1024];
#endif
int nn_st, nn_to, nn_ls;
char *BigBuffer = NULL;
char const *BigLine[RES_RED + 16];

#define is_imode() (*zz_im == 't')

char *KARA = "";
int zz_fileSize = 0;
int lineMax = -1;
int out_resN = 0;

time_t t_now;
struct tm tm_now;
long currentTime;
int isbusytime = 0;

char const *LastChar(char const *src, char c);
char *zz_GetString(char *dst, char const *tgt);
char *doReplace(char *des, char const *str0, char const *str1);
void html_foot_im(int,int);
void html_head(int level, char const *title, int line);
int res_split(char **s, char *p);
void someReplace(char const *src, char *des, char const *str0, char const *str1);
void hlinkReplace(char *src);
static void html_foot(int level, int line,int);
int getLineMax(void);
int IsBusy2ch(void);
int getFileSize(char const *file);
#ifndef CUTRESLINK
/*int res_split(char **s, char *p);*/
char *findSplitter(char *stt, int sp);
#endif
#ifdef RELOADLINK
void html_reload(int);
#endif
#ifdef RAWOUT
int rawmode;
int raw_lastnum, raw_lastsize; /* client�������Ă���f�[�^�̔ԍ��ƃT�C�Y */
#endif

#ifdef ZLIB
/*  zlib�Ή� */
gzFile pStdout; /*  = (gzFile) stdout; */
zz_printf_t pPrintf = (zz_printf_t) fprintf;
char *outbuf;
int outlen = 0;
int outalloc = 0;
#else
static int pid;
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
#ifdef	LATEST_ANCHOR
	int Latest_Num;
#endif
#ifdef	CUTRESLINK
	int LinkTagCut;
#endif
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
#ifdef	LATEST_ANCHOR
	LATEST_NUM,
#endif
#ifdef	CUTRESLINK
	LINKTAGCUT,
#endif
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
#ifdef	LATEST_ANCHOR
	{	"LATEST_NUM",	&Settings.Latest_Num,	},
#endif
#ifdef	CUTRESLINK
	{	"LINKTAGCUT",	&Settings.LinkTagCut	},
#endif
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
#ifdef	LATEST_ANCHOR
#undef	LATEST_NUM
#define	LATEST_NUM	Settings.Latest_Num
#endif
#ifdef	CUTRESLINK
#undef	LINKTAGCUT
#define	LINKTAGCUT	Settings.LinkTagCut
#endif
#endif	/*	USE_SETTING_FILE	*/

#ifdef CUTRESLINK
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
	char depth_expr[64];
	char *d = *dp;
	char const *s = *sp;
	int n;
	int f_processed = 0;

	if (path_depth == 2) {
		strncpy(depth_expr,
			zz_ky,
			sizeof(depth_expr) - 4);
		depth_expr[sizeof(depth_expr) - 4] = 0;
		strcat(depth_expr, "/");
	} else
		depth_expr[0] = 0;

	/* ���ʒu��T�� */
	n = strcspn(*sp, ">");
	if (n == 0)
		return 0;
	s += n + 1;	/* �܂�d�͐i�߂Ȃ��ł��� */
	if (!memcmp(s, "&gt;&gt;", 8)) {
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
		buf[sizeof(buf) - 1] = 0; /* sentinel */
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
			buf[sizeof(buf) - 1] = 0; /* sentinel */
			to = atoi(buf);
		}
		/* </a>������͂��Ȃ̂ŒT���A�̂Ă� */
		s = strstr(s, "</a>");
		if (!s)
			return 0;
		s += 4;

		/* chunk�d�l�𐶂������߂�kludge�͈ȉ��ɁB */
		mst = (st - 1) / CHUNK_NUM;
		mto = (to - 1) / CHUNK_NUM;

		if (mst == mto) {
			/* chunk�͈� */
			mst = 1 + CHUNK_NUM * mst;
			mto = CHUNK_NUM * (mto + 1);
		} else {
			/* chunk���܂��������Ȃ̂ŁA�ŏ��P�ʂ��B*/
			mst = st;
			mto = to;
		}

		/* �V�����\�����u�`���� */
		if (st < to)
			d += sprintf(d,
				     "<a href=\"%s%d-%d#%d\">&gt;&gt;%d-%d</a>",
				     depth_expr,
				     mst, mto, st,
				     st, to);
		else
			d += sprintf(d,
				     "<a href=\"%s%d-%d#%d\">&gt;&gt;%d</a>",
				     depth_expr,
				     mst, mto, st,
				     st);
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
			/*  cutWordOff(url, "&quot;");
				������Ƃ���Ă��邱�Ƃ��킩��Ȃ����A���̂܂ܓ�����Č�
				�ǂ����Ȃ�strnicmp()�̕����E�E�E
			*/
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
		"<A HREF=\"%.*s\" TARGET=\"_blank\">%.*s</A>", 
		urllen, url, urllen, url);
}

/*
	findSplitter�̑���
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
	int istagcut = (LINKTAGCUT && isbusytime && resnumber > 1) || rawmode || is_imode();
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
				if (resnumber) {
					if (path_depth
					    && p[1] == 'a'
					    && isspace(p[2])) {
						char *tdp = bufp;
						char const *tsp = p;
						if (!rewrite_href(&tdp, &tsp, istagcut))
							goto Break;
						bufp = tdp;
						p = tsp;
						continue;
					} else if (istagcut) {
						/* if (*(p+1) != 'b' || *(p+2) != 'r') { */
						/* <a ....> </a> ���T�p�[�����炵�� */
						if ((*(p+1) == 'a' && *(p+2) == ' ')
						    || (*(p+1) == '/' && *(p+2) == 'a')) {
							while (*p != '>') {
								if (*p == '\n')
									goto Break;
								++p;
							}
							++p;
							continue;
						}
					}
				}
				break;
			case '&':
				if (memcmp(p+1, "amp", 3) == 0) {
					if (*(p + 4) != ';')
						p += 4 - 1;
				}
				break;
			case ':':
#if	1
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
			case 0x81: /*  *"��"(8197) */
				/* if (!This->isTeri) { */
				if (memcmp(p+1, "\x97�M", 3) == 0) {
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

#endif

#ifdef COOKIE
#define FORM_MAXLEN 1024
char form_name[FORM_MAXLEN];
char form_mail[FORM_MAXLEN];

/*
 *  Unescape - %xx �����ɖ߂��āAHTML�`���ɕϊ�
 *  %00�͂��낢���肠�邩�獡�̂Ƃ����%20(' ')�ɒu��
 */

void Unescape(char *dst, const char *src, int dstlen, int srclen)
{
	while(dstlen > 1 && srclen > 0 && *src) {
		int c = 0;
		if(*src == '%') {
			int i;
			const char *tmp = ++src;
			srclen--;
			for(i = 0; i < 2 && srclen > 0; i++, src++, srclen--) {
				c *= 16;
				if(*src >= '0' && *src <= '9')
					c += *src - '0';
				else if(*src >= 'A' && *src <= 'F')
					c += *src - 'A' + 10;
				else if(*src >= 'a' && *src <= 'f')
					c += *src - 'a' + 10;
				else
					break;
			}
			if(c == 0) {
				if(tmp == src) {
					*(dst++) = '%';
					dstlen--;
					continue;
				} else {
					*(dst++) = ' ';
					dstlen--;
					continue;
				}
			}
		} else {
			c = *(src++);
			srclen--;
		}
#define SAFECPY(str, slen)        		\
		if(dstlen <= slen + 1)		\
			break;			\
		memcpy(dst, str, slen);		\
		dstlen -= slen;
		if(c == '\"') {
			SAFECPY("&quot;", 6);
		} else if(c == '&') {
			SAFECPY("&amp;", 5);
		} else if(c == '<') {
			SAFECPY("&lt;", 4);
		} else if(c == '>') {
			SAFECPY("&gt;", 4);
#undef SAFECPY
		} else {
			*dst++ = c;
			dstlen--;
		}
	}
	if(dstlen)
		*(dst++) = '\0';
}

/*
 *  SetFormName
 */

void SetFormName(void)
{
	/* HTTP_COOKIE= FROM=......;MAIL=......�Ƃ������`��
	 (&�ŋ�؂��zz_GetString���g����񂾂��ǂ�...) */
	char const *p = zz_http_cookie;
	strcpy(form_name, KARA);
	strcpy(form_mail, KARA);
	while(p && *p) {
		char *q = strchr(p, '='), *r;
		char *target;
		if(!q)
			break;
		if(q - p == 4 && !strncmp(p, "MAIL", 4)) {
			target = form_mail;
		} else if (q - p == 4 && !strncmp(p, "NAME", 4)) {
			target = form_name;
		} else {
			p = strchr(q, ';');
			if(p)
				p++;
			continue;
		}
		r = strchr(q, ';');
		if(!r) {
			r = q + strlen(q);
			p = NULL;
		} else
			p = r + 1;
		Unescape(target, q + 1, FORM_MAXLEN, r - q - 1);
	}
}
#endif

/****************************************************************/
/*	BadAccess						*/
/****************************************************************/
#ifdef NEWBA
int BadAccess(void)
{
	char *agent_kick[] = {
#ifdef	Katjusha_Beta_kisei
		"Kathusha",
#endif
		"WebFetch",
		"origin",
		"Nozilla",
		"WWWD",
	};
	int i;

	if (!*zz_http_user_agent && !*zz_http_language)
		return 1;

	for (i = 0; i < sizeof(agent_kick) / sizeof(char *); i++) {
		if (strstr(zz_http_user_agent, agent_kick[i]))
			return 1;
	}

	return 0;
}
#else
int BadAccess(void)
{
	if (strstr(zz_http_user_agent, "DreamPassport"))
		return 0;
	if (strstr(zz_http_user_agent, "DoCoMo"))
		return 0;
	if (strstr(zz_http_user_agent, "J-PHONE"))
		return 0;
	if (strstr(zz_http_user_agent, "ASTEL"))
		return 0;
	if (strstr(zz_http_user_agent, "[ja]"))
		return 0;
	if (strstr(zz_http_user_agent, "iCab"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "iBOX"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "WannaBe"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "Macintosh; I;"))
		return 0;	/* MAC          */
	if (strstr(zz_http_user_agent, "Mozilla/3.0N"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "CBBoard"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "PersonaWare"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "Sharp"))
		return 0;	/* small?       */
	if (strstr(zz_http_user_agent, "95"))
		return 0;	/* win95        */
	if (strstr(zz_http_user_agent, "NT 4.0"))
		return 0;	/* winNT        */
	if (strstr(zz_http_user_agent, "WinNT"))
		return 0;	/* winNT        */

	if (!*zz_http_user_agent && !*zz_http_language)
		return 1;
/*	if(!*zz_http_user_agent)			return 1;*/
/*	if(!*zz_http_language)				return 1;*/
#ifdef	Katjusha_Beta_kisei
	if (strstr(zz_http_user_agent, "Katjusha"))
		return 1;
#endif
	if (strstr(zz_http_user_agent, "WebFetch"))
		return 1;
	if (strstr(zz_http_user_agent, "origin"))
		return 1;
	if (strstr(zz_http_user_agent, "Nozilla"))
		return 1;
	if (strstr(zz_http_user_agent, "WWWD"))
		return 1;

	return 0;
}
#endif
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
/*	HTML BANNER						*/
/****************************************************************/
#ifdef	CM_BBSPINK
void html_banner(void)
{
	pPrintf(pStdout, R2CH_HTML_BBSPINK_BANNER);
}

#else
void html_banner(void)
{
	pPrintf(pStdout, R2CH_HTML_BANNER);
}

#endif

/****************************************************************/
/*	Get file size(out_html1)				*/
/****************************************************************/
static int out_html1(int level)
{
	char *s[20];
#ifndef TYPE_TERI
	char r4[SIZE_BUF];
#endif
	char p[SIZE_BUF];

	if (out_resN)
		return 0;
#ifndef CUTRESLINK
	strncpy(p, BigLine[0], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
	if (!*p)
		return 1; 
#endif
#ifdef	TYPE_TERI
	/*someReplace(s[4],r4,"���M",",")       ; */
	html_head(level, s[4], lineMax);
#else
	someReplace(s[4], r4, "���M", ",");
	html_head(level, r4, lineMax);
#endif
#if 0
	if (!is_imode()) {	/* no imode       */
		pPrintf(pStdout, "<DL>");
	}
#endif
	out_resN++;
	return 0;
}
/****************************************************************/
/*	Get file size(out_html)					*/
/****************************************************************/
static int out_html(int level, int line, int lineNo)
{
	char *s[20];
#ifdef	TYPE_TERI
	char *r0, *r1, *r3, *r4;
#else
	char r0[SIZE_BUF], r1[SIZE_BUF], r3[SIZE_BUF],
	    r4[SIZE_BUF];
#endif
	char p[SIZE_BUF];

/*printf("line=%d[%s]<P>\n",line,BigLine[line]);return 0;*/

	if (!out_resN) {	/* Can I write header ?   */
#ifndef CUTRESLINK
		strncpy(p, BigLine[0], 1024);
		p[1024 - 1] = '\0';
		if (!*p)
			return 1;
		res_split(s, p);
#else
		splitting_copy(s, p, BigLine[0], sizeof(p) - 20, 0);
		if (!*p)
			return 1; 
#endif
#ifdef	TYPE_TERI
		r4 = s[4];
#else
		someReplace(s[4], r4, "���M", ",");
#endif
		html_head(level, r4, lineMax);
#if 0
		if (!is_imode()) {	/* no imode       */
			pPrintf(pStdout, "<DL>");
		}
#endif
	}
	out_resN++;

#ifndef CUTRESLINK
	strncpy(p, BigLine[line], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[line], sizeof(p) - 20, line);
	if (!*p)
		return 1; 
#endif
	
#ifdef	TYPE_TERI
	r0 = s[0];
	r1 = s[1];
	r3 = s[3];
#else
	someReplace(s[0], r0, "���M", ",");
	someReplace(s[1], r1, "���M", ",");
	someReplace(s[3], r3, "���M", ",");
	someReplace(r3, r3, "&amp;", "&");
#endif
#ifndef	CUTRESLINK
	hlinkReplace(r3);
#endif

	if (!is_imode()) {	/* no imode */
#ifndef	CUTRESLINK
		if (*r3 && strlen(r3) < 8192) {
#else
		if (*r3 && s[4]-r3 < 8192) {
#endif
			if (r1 && strcmp(r1, "sage") == 0) {
#ifdef SAGE_IS_PLAIN
				pPrintf(pStdout,
					R2CH_HTML_RES_SAGE("%d", "%d", "%s", "%s", "%s"),
					lineNo, lineNo, r0, s[2], r3);
#else
				pPrintf(pStdout,
					R2CH_HTML_RES_MAIL("%d", "%d", "%s", "%s", "%s", "%s"),
					lineNo, lineNo, r1, r0, s[2], r3);
#endif
			} else if (*r1) {
				pPrintf(pStdout,
					R2CH_HTML_RES_MAIL("%d", "%d", "%s", "%s", "%s", "%s"),
					lineNo, lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_RES_NOMAIL("%d", "%d", "%s", "%s", "%s"),
					lineNo, lineNo, r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_RES_BROKEN_HERE,
				lineNo);
		}
		if (isbusytime && out_resN > RES_NORMAL) {
			if (path_depth)
				pPrintf(pStdout,
					R2CH_HTML_PATH_TAIL,
					lineNo,
					lineNo + RES_NORMAL,
					RES_NORMAL,
					RES_NORMAL,
					RES_NORMAL,
					LIMIT_PM - 12, LIMIT_AM);
			else
				pPrintf(pStdout,
					R2CH_HTML_TAIL,
					CGINAME, zz_bs, zz_ky,
					lineNo,
					lineNo + RES_NORMAL,
					RES_NORMAL,
					CGINAME, zz_bs, zz_ky,
					RES_NORMAL,
					RES_NORMAL,
					LIMIT_PM - 12, LIMIT_AM);
			return 1;
		}
	} else {		/* imode  */

		if (*r3) {
			if (*s[1]) {
				pPrintf(pStdout, R2CH_HTML_IMODE_RES_MAIL,
					lineNo, r1, r0, s[2], r3);
			} else {
				pPrintf(pStdout,
					R2CH_HTML_IMODE_RES_NOMAIL, lineNo,
					r0, s[2], r3);
			}
		} else {
			pPrintf(pStdout, R2CH_HTML_IMODE_RES_BROKEN_HERE,
				lineNo);
		}
		if (out_resN > RES_IMODE) {
			pPrintf(pStdout, R2CH_HTML_IMODE_TAIL,
				CGINAME, zz_bs, zz_ky, lineNo,
				lineNo + RES_IMODE, RES_IMODE, CGINAME,
				zz_bs, zz_ky, RES_IMODE, RES_IMODE);
			return 1;
		}
	}

	return 0;
}
/****************************************************************/
/*	Output raw data file					*/
/****************************************************************/
#ifdef RAWOUT
#ifdef	CUTRESLINK
#if	0
/* BigLine��nul-terminated�ł͂Ȃ�'\n'-terminated�ɂ���ꍇ */
int getlinelen(const char *line)
{
	const char *last = BigBuffer + zz_fileSize;
	const char *end = memchr(line, '\n', last - line);
	if (end)
		return end + 1 - line;
	return last - line;
}
#endif
#endif

int dat_out_raw(void)
{
	int i;

	/* �����񍐂��ꂽ�ŏI���X�ԍ�����уT�C�Y����v���Ă��Ȃ�
	   ��΁A�ŏ��̍s�ɂ��̎|������ */
	if(raw_lastnum > 0
	   && !(raw_lastnum <= lineMax
		&& (BigLine[raw_lastnum - 1]
#ifndef	CUTRESLINK
		    + strlen(BigLine[raw_lastnum - 1]) + 1
#else
			+ (BigLine[raw_lastnum] - BigLine[raw_lastnum - 1])
			/* + getlinelen(BigLine[raw_lastnum - 1]) */
#endif
		    - BigBuffer) == raw_lastsize)) {
		pPrintf(pStdout, "-INCR\n");
		/* �S���𑗐M����悤�ɕύX */
		raw_lastnum = 0;
	} else {
		pPrintf(pStdout, "+OK\n");
	}
	/* raw_lastnum ����S���𑗐M���� */
	for(i = raw_lastnum; i < lineMax; i++) {
		pPrintf(pStdout, "%s\n", BigLine[i]);
	}
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
	int line, lineNo;
#ifdef RELOADLINK
	int lineLast = lineMax;
#endif
	int threadStopped=0;
	char *s[20];
	char p[SIZE_BUF];
	
	for (line = 0; line < lineMax; line++) {
		lineNo = line + 1;

		if (lineNo == 1) {
			if (!strncmp(zz_nf, "t", 1))
				continue;
		} else {
			if (nn_st && lineNo < nn_st)
				continue;
			if (nn_to && lineNo > nn_to)
				continue;
			if (nn_ls && line < lineMax - nn_ls)
				continue;
		}

		if (out_html(level, line, lineNo))
			break;
#ifdef RELOADLINK
		lineLast = lineNo;
#endif
	}
	out_html1(level);
#ifdef RELOADLINK
	if (!level && lineMax == lineLast) {
		html_reload(lineLast);	/*  Button: Reload */
	}
#endif
#ifndef CUTRESLINK
	strncpy(p, BigLine[lineMax-1], 1024);
	p[1024 - 1] = '\0';
	if (!*p)
		return 1;
	res_split(s, p);
#else
	splitting_copy(s, p, BigLine[lineMax-1], sizeof(p) - 20, lineMax-1);
	if (!*p)
		return 1; 
#endif
	if( s[2]!=0 && strstr( s[2], "�X�g�b�p�[" )) threadStopped=1;
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

	zz_fileSize = getFileSize(fname);

	if (zz_fileSize > MAX_FILESIZE)
		html_error(ERROR_TOO_HUGE);
	if (zz_fileSize < 10)
		html_error(ERROR_NOT_FOUND); /* �G���[��ʂ͕ʂɂ��������������� */
	if (*zz_ky == '.')
		html_error(ERROR_NOT_FOUND);

#if 1
	nn_st = st;
	nn_to = to;
	nn_ls = ls;
#else
	nn_st = atoi(zz_st);
	nn_to = atoi(zz_to);
	nn_ls = atoi(zz_ls);
#endif
	if (nn_st < 0)
		nn_st = 0;
	if (nn_to < 0)
		nn_to = 0;
	if (nn_st == 1 && nn_to == 1)
		strcpy(zz_nf, KARA);
	if (is_imode()) {	/* imode */
		if (!nn_st && !nn_to && !nn_ls)
			nn_ls = RES_IMODE;
	}
	if (zz_nf[0] != 't' && nn_ls > 0) {
		nn_ls--;
		if(nn_ls == 0) {
			nn_ls = 1;
			strcpy(zz_nf, "true");
		}
	} else if (nn_ls < 0)
		nn_ls = 0;

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
#ifndef	CUTRESLINK
	/* XXX �Ƃ���ǂ���� 0 ��������? */
	{
		char *end = BigBuffer + zz_fileSize;
		char *p = BigBuffer;
		while ((p = memchr(p, '\0', end - p)) != NULL) {
			*p = '*';
		}
	}
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

#ifndef	CUTRESLINK
	while ((p1 = strchr(p, '\n')) != NULL) {
		BigLine[line] = p;
		*p1 = '\0';
		p = p1;
		if (line > RES_RED)
			break;
		line++;
		p++;
	}
#else
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
		p = memchr(p, '\n', p1-p) + 1;
	} while(p != p1);
	
	/*
		�Ō�̃��X�̎��ɁA�t�@�C�����ւ̃|�C���^�����Ă����B
		����ɂ��A���X�̒����̓|�C���^�̍��ł��ށB
		(dat_out_raw��strlen���Ă��镔���ւ̑Ή�)
	*/
	BigLine[line+1] = BigBuffer + zz_fileSize;
#endif
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
#ifdef LASTMOD
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
#endif
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

	/* strtok()�Ő؂�o�����o�b�t�@�͑���������
	   �������Ă���̂ŁAbuffer overrun�͂Ȃ��͂� */
	if (s[0]) {
		char *p;
		/* �͈͎w��̃t�H�[�}�b�g�͈ȉ��̂��̂�����

		   /4	(st=4&to=4)
		   /4-	(st=4)
		   /-6	(to=6)
		   /4-6	(st=4to=4)

		   �J�L�R1�͓��ʈ����ŁAnofirst�ɂ��
		   ���삪���E�������ۂ��̂ŁA�ǂ����悤 */

		/* ���Ɏw�肪�Ȃ�������Ast=1�ł���Ƃ݂Ȃ� */
		strcpy(zz_st, "1");

		/* st �����o�� */
		if (isdigit(*s)) {
			for (p = zz_st; isdigit(*s); p++, s++)
				*p = *s;
			*p = 0;
		}

		if (*s == '-') {
			s++;
			/* to�����o�� */
			if (isdigit(*s)) {
				for (p = zz_to; isdigit(*s); p++, s++)
					*p = *s;
				*p = 0;
			}
		} else {
			/* �͈͎w��͂Ȃ��̂ŁA
			   �P�_�|�C���g�ƌ��Ȃ� */
			strcpy(zz_to, zz_st);
		}

		/* nofirst�̎d�l�����܂������߂�kludge XXX */
		if (!zz_nf[0])
			strcpy(zz_nf,
			       (atoi(zz_st) == 1
				? "false"
				: "true"));
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
		char *cptr;
		char *endp;
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
					if (cptr[len] == '='
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
	currentTime = (long) time(&t_now);
	putenv("TZ=JST-9");
	tzset();
	tm_now = *localtime(&t_now);

	zz_remote_addr = getenv("REMOTE_ADDR");
	zz_remote_host = getenv("REMOTE_HOST");
	zz_http_referer = getenv("HTTP_REFERER");
	zz_http_cookie = getenv("HTTP_COOKIE");
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
#ifdef LASTMOD
	zz_http_if_modified_since = getenv("HTTP_IF_MODIFIED_SINCE");
#endif

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

#ifdef GSTR2
	zz_GetString(zz_bs, "b");
	zz_GetString(zz_ky, "k");
	zz_GetString(zz_ls, "l");
	zz_GetString(zz_st, "s");
	zz_GetString(zz_to, "t");
	zz_GetString(zz_nf, "n");
	zz_GetString(zz_im, "i");
#ifdef RAWOUT
	zz_GetString(zz_rw, "r");
#endif
#else
	zz_GetString(zz_bs, "bbs");
	zz_GetString(zz_ky, "key");
	zz_GetString(zz_ls, "ls");
	zz_GetString(zz_st, "st");
	zz_GetString(zz_to, "to");
	zz_GetString(zz_nf, "nofirst");
	zz_GetString(zz_im, "imode");
#ifdef RAWOUT
	zz_GetString(zz_rw, "raw");
#endif
#endif
#ifdef USE_PATH
	if (!get_path_info(zz_path_info)) {
		/* ����ȍ~�Apath ���t�^����Ă��邩�ǂ�����
		   ����� zz_path_info �̃e�X�g�ōs���Ă��� */
		zz_path_info = NULL;
	}
#endif
#ifdef COOKIE
	SetFormName();
#endif
#ifdef RAWOUT
	rawmode = (*zz_rw != '\0');
	if(rawmode) {
		char *p = strchr(zz_rw, '.');
		if(p) {
			/* raw=(last_article_no).(local_dat_size) */
			raw_lastnum = atoi(zz_rw);
			raw_lastsize = atoi(p + 1);
		}
		if(!p || raw_lastnum < 1 || raw_lastsize < 1) {
			raw_lastnum = raw_lastsize = 0;
		}
	}
#endif
#ifdef	USE_SETTING_FILE
	readSettingFile(zz_bs);
#endif
	isbusytime = IsBusy2ch();
}

#ifdef ZLIB
/****************************************************************/
/* receive gzipped data from zlib/gzio.c                        */
/****************************************************************/
int gzipped_fwrite(char *buf, int n, int m, FILE *fp)
{
	int l = n*m;

	if ( fp != stdout ) return fwrite(buf,n,m,fp);
	if ( outlen+l > outalloc ) {
		outalloc = outlen + l + 40960;  /* ���߂� */
		if ( outlen == 0 ) {
			outbuf = malloc(outalloc);
		} else {
			if ( outbuf == NULL ) return m;  /* already error */
			outbuf = realloc(outbuf, outalloc);
		}
	}
	if ( outbuf != NULL ) {
		memcpy(outbuf+outlen, buf, l);
	}
	outlen += l;
	return m;
}
#endif

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
		/* gzclose()�����ŏ\���Ǝv��... */
		/* gzflush(pStdout, Z_FINISH);
		fflush(stdout); */
		gzclose(pStdout);

		if ( outlen != 0 && outbuf == NULL ) {
			pPrintf = (zz_printf_t) fprintf;
			pStdout = (gzFile) stdout;
			pPrintf(pStdout,"\n");
			html_error(ERROR_NO_MEMORY);
		}
		if ( gzip_flag == 2 ) {
			fprintf(stdout,"Content-Encoding: x-gzip\n");
		} else {
			fprintf(stdout,"Content-Encoding: gzip\n");
		}
		fprintf(stdout,"Content-Length: %d\n\n",outlen);

		fwrite(outbuf,1,outlen,stdout);
		fflush(stdout);
	}
#elif defined(GZIP)
	if(gzip_flag) {
		fflush(stdout);
		close(1);
		waitpid(pid, NULL, 0);
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
	char fname[1024];

#if	('\xFF' != 0xFF)
#error	-funsigned-char required.
	/* ���̃V�X�e���ł́A-funsigned-char��v������ */
	if ((char)0xFF != (unsigned char)0xFF) {
		puts("Content-Type: text/html\n"
		     "\n"
		     "-funsigned-char required.");
		return 0;
	}
#endif

#ifdef ZLIB
	pStdout = (gzFile) stdout;
#endif
	zz_GetEnv();

#ifdef RAWOUT
	if(!rawmode)
#endif
		pPrintf(pStdout, "Content-Type: text/html\n");
#ifdef RAWOUT
	else
		/* pPrintf(pStdout, "Content-Type: application/octet-stream\n"); */
		/* ���݂�.dat�� MIME type �ɍ��킹��D�e�L�X�g�f�[�^����... */
		pPrintf(pStdout, "Content-Type: text/plain\n");
#endif
#ifdef LASTMOD
	sprintf(fname, "../%.256s/dat/%.256s.dat", zz_bs, zz_ky);
#ifdef DEBUG
	sprintf(fname, "998695422.dat");
#endif
	/* �X���ꗗ�����ɐ������[�h */
	if (1 <= path_depth && path_depth < 3) {
		sprintf(fname, "../%.256s/subject.txt", zz_bs);
	}

	zz_fileLastmod = getFileLastmod(fname);
	get_lastmod_str(lastmod_str, zz_fileLastmod);
#if 1
	get_lastmod_str(expires_str, zz_fileLastmod + 5);
#else
	{
	  /* ���߂��ɔp���������������30�b��ɐݒ肵�Ă݂� */
	  time_t nw;
	  time(&nw);
	  get_lastmod_str(expires_str, nw + 30);
	}
#endif
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
		pPrintf(pStdout, "Status: 304\n");
		pPrintf(pStdout, "\n");
		return (0);
	}
#ifdef PREVENTRELOAD
	}
#endif
#endif

#ifdef GZIP
	if (zz_http_encoding && strstr(zz_http_encoding, "x-gzip")) {
		gzip_flag = 2;
	} else if (zz_http_encoding && strstr(zz_http_encoding, "gzip")) {
		gzip_flag = 1;
	} else {
		gzip_flag = 0;
	}
#ifndef ZLIB
	if ( gzip_flag == 2 ) {
		pPrintf(pStdout, "Content-Encoding: x-gzip\n");
	} else if ( gzip_flag == 1 ) {
		pPrintf(pStdout, "Content-Encoding: gzip\n");
	}
#endif
#endif

/*  Get Last-Modified Date */
#ifdef LASTMOD
	pPrintf(pStdout, "Last-Modified: %.256s\n", lastmod_str);
#ifdef EXPIRES
	/* Expires��f���Ă݂� */
	pPrintf(pStdout, "Expires: %.256s\n", expires_str);
#endif
#endif

#ifdef ZLIB
	if ( gzip_flag == 0 ) pPrintf(pStdout, "\n");
#else
	pPrintf(pStdout, "\n");
#endif
#ifdef DEBUG
	sleep(1);
#endif
	fflush(stdout);

	/*  �I�������o�^ */
	atexit(atexitfunc);
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
		/* ����1��zlib/gzio.c�œ��ʈ����i2ch�o�[�W�����̓Ǝ��d�l�j
		   ����stdout��ݒ肵�Aclose���Ȃ� */
		pStdout = gzdopen(1, "wb9");

		pPrintf = gzprintf;
		/* gzdopen()��"wb9"���w�肵���̂ŕs�v */
		/* gzsetparams(pStdout, Z_BEST_COMPRESSION,
			Z_DEFAULT_STRATEGY); */
#if 0
		/* put 2048byte */
		/* �����v��Ȃ��񂶂�Ȃ����ȁA
		   XXX �ǎ����� */
		while (whitespace--)
			gzputc(pStdout, ' ');
#endif
#endif /* ZLIB */
	}
#endif /* GZIP */

	logOut("");

	dat_read(fname,
		 atoi(zz_st),
		 atoi(zz_to),
		 atoi(zz_ls));

#ifdef RAWOUT
	if (rawmode)
		dat_out_raw();
#ifdef USE_PATH
	else if (path_depth == 2) {
		if (zz_ky[0] == '-')
			dat_out_subback();	/* �X���ꗗ */
		else
			dat_out_index();	/* �_�C�W�F�X�g */
	}
#endif
	else
		dat_out(0);
#else
	dat_out(0);
#endif
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
#ifdef RAWOUT
	if(rawmode) {
		/* ?....�̓G���[�B */
		pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
	}
#endif
	
	*tmp = '\0';
	strcpy(tmp, LastChar(zz_ky, '/'));
	strncpy(zz_soko, tmp, 3);

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
		sprintf(doko, "../%.50s/kako/%.50s/%.50s.html", zz_bs,
			zz_soko, tmp);
		if (!stat(doko, &CountStat)) {
			pPrintf(pStdout, R2CH_HTML_ERROR_5_HTML, doko + 2,
				tmp);
		} else {
			sprintf(doko, "../%.50s/kako/%.50s/%.50s.dat",
				zz_bs, zz_soko, tmp);
			if (!stat(doko, &CountStat)) {
				pPrintf(pStdout, R2CH_HTML_ERROR_5_DAT,
					doko + 2, tmp);
			} else {
				pPrintf(pStdout, R2CH_HTML_ERROR_5_NONE,
					zz_bs);
			}
		}
	}

	pPrintf(pStdout, R2CH_HTML_ERROR_6);

	exit(0);
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
		/* ?....�̓G���[�B */
		pPrintf(pStdout, "-ERR %s\n", mes);
		exit(0);
	}
#endif
	strcpy(tmp, LastChar(zz_ky, '/'));
	strncpy(zz_soko, tmp, 3);
	sprintf(tmp_time, "%02d:%02d:%02d", tm_now.tm_hour, tm_now.tm_min,
		tm_now.tm_sec);

	pPrintf(pStdout, R2CH_HTML_ERROR_999_1,
		mes, zz_bs, zz_ky, zz_ls, zz_st, zz_to, zz_nf, zz_fileSize,
		lineMax, tmp_time, zz_bs, zz_soko, tmp, tmp);
	html_banner();
	pPrintf(pStdout, R2CH_HTML_ERROR_999_2);

	exit(0);
}
/****************************************************************/
/*								*/
/****************************************************************/
#ifdef GSTR2
char *zz_GetString(char *dst, char const *tgt)
{
	int i;
	int len;
	int ch = (int) '&';
	int ch2 = (int) '=';
	char const *kk = zz_query_string;
	char *kk0;
	char *kk1;
	for (i = 0; i < 200; i++) {
		if (kk[0] == tgt[0]) {
			kk0 = strchr(kk, ch);	/* & */
			if (!kk0)
				kk0 = strchr(kk, (int) '\0');
			kk1 = strchr(kk, ch2);	/* = */
			len = kk0 - kk1 - 1;
			if (len > 0) {
				if (kk0)
					*kk0 = '\0';
				strncpy(dst, kk1 + 1, 20);
				dst[20 - 1] = '\0';
				if (kk0)
					*kk0 = '&';
				return dst;
			}
		}
		kk = strchr(kk, ch);
		if (!kk)
			break;
		kk++;
	}
	*dst = '\0';
	return dst;
}
#else
char *zz_GetString(char *dst, char *tgt)
{
	int i;
	int ch = (int) '&';
	char *kk0;
	char *kk1;
	char *kk = zz_query_string;
	for (i = 0; i < 200; i++) {
		if (!strncmp(kk, tgt, strlen(tgt))) {
			kk0 = kk;
			kk1 = strchr(kk, ch);
			if (kk1)
				*kk1 = '\0';
			strncpy(dst, kk0 + strlen(tgt) + 1, 1024);
			dst[1024 - 1] = '\0';
			if (kk1)
				*kk1 = ch;
			return dst;
		}
		kk = strchr(kk, ch);
		if (!kk)
			break;
		kk++;
	}
	*dst = '\0';
	return dst;
}
#endif
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
#ifdef	TYPE_TERI
char *findSplitter(char *stt, int sp)
{
	char *p = stt;
	char ss = (unsigned char) (sp & 0x00ff);

	while (*p) {
/*
		if( *p >= 0xa0 && *p <= 0xdf)	{p++	;continue;}
		if((*p >= 0x81 && *p <= 0x9f || *p >= 0xe0 && *p <= 0xfc))	{p += 2	;continue;}
*/
		if (*p == ss && *(p + 1) == '>')
			return p;
		p++;
	}
	return NULL;
}
#else
char *findSplitter(char *stt, int sp)
{
	char *p = stt;
	char ss = (unsigned char) (sp & 0x00ff);

	while (*p) {
		if (*p >= 0xa0 && *p <= 0xdf) {
			p++;
			continue;
		}
		if (((*p >= 0x81 && *p <= 0x9f)
		     || (*p >= 0xe0 && *p <= 0xfc))) {
			p += 2;
			continue;
		}
/*		if((*p >= 0x40 && *p <= 0xfc && *p != 0x7f))	p += 2	;*/
		if (*p == ss)
			return p;
		p++;
	}
	return NULL;
}
#endif
/****************************************************************/
/*								*/
/****************************************************************/
int res_split(char **s, char *p)
{
	int i;
	char *p0;

	for (i = 0; i < 5; i++)
		s[i] = KARA;

	s[0] = p;
	for (i = 1; i < 5; i++) {
#ifdef	TYPE_TERI
		p0 = findSplitter(s[i - 1], '<');
		if (!p0) {
			return 0;
		}
		*p0 = '\0';
		p0++;
		*p0 = '\0';
		p0++;
		s[i] = p0;
#else
		p0 = findSplitter(s[i - 1], ',');
		if (!p0) {
			return 0;
		}
		*p0 = '\0';
		p0++;
		s[i] = p0;
#endif
	}
	return 1;
}
/****************************************************************/
/*								*/
/****************************************************************/
void dump_out16(char const *d)
{
	char const *p = d;
	pPrintf(pStdout, "\n\n<!-- ");
	while (*p) {
		pPrintf(pStdout, ",%02x", *p);
		p++;
	}
	pPrintf(pStdout, " -->\n\n");
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
/*	����[							*/
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
/****************************************************************/
/*	HTML HEADER						*/
/****************************************************************/
void html_head(int level, char const *title, int line)
{
#ifdef CHUNK_ANCHOR
	int i;
#endif
	char fname[1024];

	if (level) {
		pPrintf(pStdout,
			R2CH_HTML_DIGEST_HEADER_2("%s"),
			title);
		/* ���ꂾ���o�͂��Ă��ǂ� */
		return;
	}

	if (!is_imode()) {	/* no imode       */
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_HEADER_1("%s", "../"),
				title);
		else {
			sprintf(fname, "../%.256s/index2.cgi", zz_bs);
			if (access(fname, S_IXUSR) == -1)
				pPrintf(pStdout,
					R2CH_HTML_HEADER_1("%s", "/%s/index2.htm%s"),
					title, zz_bs, gzip_flag ? "" : "l");
			else
				pPrintf(pStdout,
					R2CH_HTML_HEADER_1("%s", "/%s/index2.cgi"),
					title, zz_bs);
		}
#ifdef ALL_ANCHOR
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_ALL_ANCHOR); 
		else
			pPrintf(pStdout,
				R2CH_HTML_ALL_ANCHOR,
				zz_bs, zz_ky); 
#endif
#ifdef CHUNK_ANCHOR
		for (i = 1; i <= line; i += CHUNK_NUM) {
			if (path_depth)
				pPrintf(pStdout,
					R2CH_HTML_PATH_CHUNK_ANCHOR,
					i,
					i + CHUNK_NUM - 1, 
					i);
			else
				pPrintf(pStdout, R2CH_HTML_CHUNK_ANCHOR,
					zz_bs, zz_ky,
					i,
					i + CHUNK_NUM - 1, 
					(i == 1 ? "" : "&n=t"),
					i);
		}
#endif /* CHUNK_ANCHOR */
#ifdef LATEST_ANCHOR
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_LATEST_ANCHOR,
				LATEST_NUM, LATEST_NUM);
		else
			pPrintf(pStdout,
				R2CH_HTML_LATEST_ANCHOR,
				zz_bs, zz_ky,
				LATEST_NUM, LATEST_NUM);
#endif
	} else {
		pPrintf(pStdout, R2CH_HTML_IMODE_HEADER_1,
			title, zz_bs, zz_bs, zz_ky, RES_IMODE, zz_bs,
			zz_ky, RES_IMODE, RES_IMODE);
	}

	if (line > RES_RED) {
		pPrintf(pStdout, R2CH_HTML_HEADER_RED, RES_RED);
	} else if (line > RES_REDZONE) {
		pPrintf(pStdout, R2CH_HTML_HEADER_REDZONE, RES_REDZONE,
			RES_RED);
	} else if (line > RES_YELLOW) {
		pPrintf(pStdout, R2CH_HTML_HEADER_YELLOW, RES_YELLOW,
			RES_RED);
	}

	if (is_imode())
		pPrintf(pStdout, R2CH_HTML_HEADER_2_I, title);
	else
		pPrintf(pStdout, R2CH_HTML_HEADER_2, title);
}

/****************************************************************/
/*	RELOAD						        */
/****************************************************************/
#ifdef RELOADLINK
void html_reload(int startline)
{
	if (is_imode())	/*  imode */
		pPrintf(pStdout, R2CH_HTML_RELOAD_I, zz_bs, zz_ky,
			startline);
	else {
		if (path_depth)
			pPrintf(pStdout,
				R2CH_HTML_PATH_RELOAD,
				startline);
		else
			pPrintf(pStdout,
				R2CH_HTML_RELOAD,
				zz_bs, zz_ky,
				startline);
	}
}
#endif
/****************************************************************/
/*	HTML FOOTER						*/
/****************************************************************/
static void html_foot(int level, int line, int stopped)
{
	out_resN = 0;	/* �����ŏ���������Ƃ����炵��? */

	if (is_imode())
		return html_foot_im(line,stopped);
	if (line <= RES_RED && !stopped) {
#ifndef COOKIE
		if (path_depth == 3)
			pPrintf(pStdout,
				R2CH_HTML_FORM("../../../", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
		else if (path_depth == 2)
			pPrintf(pStdout,
				R2CH_HTML_FORM("../../", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
		else
			pPrintf(pStdout,
				R2CH_HTML_FORM("", "%s", "%s", "%ld"),
				zz_bs, zz_ky, currentTime);
#else
		pPrintf(pStdout, R2CH_HTML_FORM, form_name,
			form_mail, zz_bs, zz_ky,
			currentTime);
#endif
		}

	if (level)
		pPrintf(pStdout, R2CH_HTML_DIGEST_FOOTER);
	else
		pPrintf(pStdout, R2CH_HTML_FOOTER);
}
/****************************************************************/
/*	HTML FOOTER(i-MODE)					*/
/****************************************************************/
void html_foot_im(int line, int stopped)
{
	if (line <= RES_RED && !stopped ) {
		pPrintf(pStdout, R2CH_HTML_FORM_IMODE, zz_bs, zz_ky, currentTime);
	}
	pPrintf(pStdout, R2CH_HTML_FOOTER_IMODE);
}

/****************************************************************/
/*	Replace(do)						*/
/****************************************************************/
char *doReplace(char *des, char const *str0, char const *str1)
{
	char *p; 

	int str0_length; 
	int str1_length; 

	/* �u��������ׂ�������̈ʒu���擾 */ 
	p = strstr(des, str0); 
	if (p == NULL) { 
		return NULL; 
	} 

	str0_length = strlen(str0); 
	str1_length = strlen(str1); 

	/* ���̕�����ړI�̈ʒu�܂ňړ� */
	memmove( p + str1_length, p + str0_length, strlen(p + str0_length)+1 );

	/* str1���͂ߍ��� */
	memcpy( p, str1, str1_length );

	/* ����������Ȍ�̕�����̈ʒu��Ԃ� */ 
	return p + str1_length; 
}
/****************************************************************/
/*	Replace(some)						*/
/****************************************************************/
void someReplace(char const *src,
		 char *des,
		 char const *str0, char const *str1)
{
	char *last;
	strcpy(des, src);

	last = des;
	while (last) {
		last = doReplace(last, str0, str1);
	}
}
/****************************************************************/
/*	Replace(hrefStop)					*/
/****************************************************************/
#ifndef CUTRESLINK
int hrefStop(char x)
{
	if ('A' <= x && x <= 'Z')
		return 0;
	if ('a' <= x && x <= 'z')
		return 0;
	if ('0' <= x && x <= '9')
		return 0;
	if (x == '#')
		return 0;
	if (x == '/')
		return 0;
	if (x == '~')
		return 0;
	if (x == '_')
		return 0;
	if (x == '.')
		return 0;
	if (x == ',')
		return 0;
	if (x == '$')
		return 0;
	if (x == '%')
		return 0;
	if (x == '&')
		return 0;
	if (x == '@')
		return 0;
	if (x == '?')
		return 0;
	if (x == '=')
		return 0;
	if (x == '-')
		return 0;
	if (x == '+')
		return 0;
	if (x == '*')
		return 0;
	if (x == ';')
		return 0;
	if (x == ':')
		return 0;
	if (x == '!')
		return 0;
	if (x == '^')
		return 0;
	if (x == '`')
		return 0;
	if (x == '|')
		return 0;
	if (x == '[')
		return 0;
	if (x == ']')
		return 0;
	if (x == '{')
		return 0;
	if (x == '}')
		return 0;
	if (x == '\\')
		return 0;
	return 1;
}
#endif
/****************************************************************/
/*	Replace(cutWordOff)					*/
/****************************************************************/
int cutWordOff(char *src, char *word)
{
	char *p = strstr(src, word);
	if (!p)
		return 0;
	*p = '\0';
	return 1;
}
/****************************************************************/
/*	Replace(ExistHlinkX)					*/
/****************************************************************/
int ExistHlinkX(char *tgt, char *src, char *url, char *hrefStr)
{
	char *p;
	char *u;
	p = strstr(src, tgt);
	if (!p)
		return 0;
	/*NASHI*/ for (u = url; *p; p++) {
		if (hrefStop(*p))
			break;
		*u = *p;
		u++;
	}
	*u = '\0';
	/*-- &quot; ���̍폜 --*/
	cutWordOff(url, "&quot;");
/*	cutWordOff(url,";")		;*/
	/*---------------------*/
	sprintf(hrefStr,
		"<A HREF=\"%.1024s\" TARGET=\"_blank\">%.1024s</A>", url,
		url);
	return 1;
/*ARI*/}
/****************************************************************/
/*	Replace(ExistHlink)					*/
/****************************************************************/
int ExistHlink(char *src, char *h0, char *h1)
{
	if (ExistHlinkX("http://", src, h0, h1))
		return 1;
	if (ExistHlinkX("ftp://", src, h0, h1))
		return 1;
	return 0;
}
/****************************************************************/
/*	Replace(hlinkReplace)					*/
/****************************************************************/
void hlinkReplace(char *src)
{
	char *last = src;
	char hlink0[SIZE_BUF];
	char hlink1[SIZE_BUF];

	while (last) {
		if (!ExistHlink(last, hlink0, hlink1))
			break;
		last = doReplace(last, hlink0, hlink1);
	}
}


/****************************************************************/
/*	END OF THIS FILE					*/
/****************************************************************/
