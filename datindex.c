/*  file: datindex.c
 *
 *  �C���f�N�X�^�p
 *
 *  $Id: datindex.c,v 1.5 2001/09/03 10:35:23 2ch Exp $ */

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "datindex.h"
#include "read.h"

/* /board/dat/idx/XXXXXXXXXX.idx */
#define DATINDEX_PATH "dat/idx"

/* version �� 0 �̂Ƃ��́A��蒼������(sec) */
#define DATINDEX_EXPIRATION 60

/****************************************************************
 *	���t����͂���
 *	��͂ɐ���������A�X�L����������������Ԃ�
 *	���s������ 0 ��Ԃ�
 *	�c�Ȃ����b�̎擾�ɂ��Ή����Ă��邼(��
 ****************************************************************/
static int get_date(struct tm *datetime,
		    char const *p)
{
	char buf[24];
	char const *dp;
	int n, ofs, yy;

	memset(datetime, 0, sizeof(struct tm));

	ofs = strcspn(p, "<>\n0123456789");
	if (!isdigit(p[ofs]))
	return 0;

	/* yymmdd�𓾂� */
	n = strspn(&p[ofs], "/0123456789");
	if (n >= 20)
		return 0;
	memcpy(buf, &p[ofs], n);
	buf[n] = 0;
	ofs += n;
	if (!(dp = strtok(buf, "/")))
		return 0;
	yy = atoi(dp);
	if (yy >= 1900)
		yy -= 1900;
	else if (yy < 70)
		yy += 100;
	datetime->tm_year = yy;
	if (!(dp = strtok(NULL, "/")))
		return 0;
	datetime->tm_mon = atoi(dp) - 1;
	if (!(dp = strtok(NULL, "/")))
		return 0;
	datetime->tm_mday = atoi(dp);

	/* hhmm[ss]�𓾂� */
	ofs += strcspn(&p[ofs], "<>\n0123456789");
	if (!isdigit(p[ofs]))
		return 0;
	n = strspn(&p[ofs], ":0123456789");
	if (n >= 20)
		return 0;
	memcpy(buf, &p[ofs], n);
	buf[n] = 0;
	ofs += n;
	if (!(dp = strtok(buf, ":")))
		return 0;
	datetime->tm_hour = atoi(dp);
	if (!(dp = strtok(NULL, ":")))
		return 0;
	datetime->tm_min = atoi(dp);
	if ((dp = strtok(NULL, ":")) != NULL)
		datetime->tm_sec = atoi(dp);

	return ofs;
}
/****************************************************************
 *	�p�ӂ��ꂽ�̈��
 *	�V���ɃC���f�N�X�����肠����
 *	private_dat�Ƀt�@�C�����ǂ܂�Ă���̂��O��
 *	�����͏������s��Ȃ�
 ****************************************************************/
static int buildup_index(DATINDEX_OBJ *dat,
			 int linenum,	/* 0 origin */
			 DATINDEX volatile *idx,
			 long ofs,
			 long datlen)
{
	int chunk;
	char const *const p = dat->private_dat;

	/* .dat �� mtime �͋L�^���Ă��� */
	idx->lastmod = dat->dat_stat.st_mtime;

	chunk = linenum / DATINDEX_CHUNK_SIZE;
	linenum %= DATINDEX_CHUNK_SIZE;
	for (/* nil */;
		      (ofs < datlen
		       && (DATINDEX_CHUNK_SIZE * chunk + linenum
			   < DATINDEX_MAX_ARTICLES));
		      linenum++, (linenum + 1 >= DATINDEX_CHUNK_SIZE
				  ? idx->idx[chunk].nextofs = ofs
				  : 0)) {
		struct DATINDEX_LINE *line;
		int i, n;
		int orig_ofs;
		struct tm datetime;

		/* �킩��Ȃ��t���O���������Ă��߂�� */
		int aborted = 0;	/* ���ځ[��? */
		int completed = 0;	/* �p�[�X���ʂ�����Z�b�g���ׂ� */
		time_t lastmod = idx->lastmod;

		if (linenum >= DATINDEX_CHUNK_SIZE) {
			/* �J��オ�� */
			idx->idx[++chunk].lastmod = 0; /* epoch(�m */
			linenum = 0;
			/* XXX nextofs �́A
			   ���[�v���E�ōX�V���Ă��� */
		}

		line = &dat->line[DATINDEX_CHUNK_SIZE * chunk + linenum];
		orig_ofs = ofs;

		/* name<>mailto<>date id<>text<>[subject]\n */
		/* name, mailto���X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->name.p = &p[ofs];
			else if (i == 1)
				line->mailto.p = &p[ofs];

			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}

			if (i == 0)
				line->name.len = &p[ofs] - line->name.p;
			else if (i == 1)
				line->mailto.len = &p[ofs] - line->mailto.p;

			ofs += 2;
		}

		/* ���t����͂���
		   ���t�������܂����������Ȃ�������A
		   ���ځ[��ƌ��Ȃ� */
		n = get_date(&datetime, &p[ofs]);
		if (n == 0) {
			/* ���ځ[��ƌ��Ȃ�
			   ���̂܂܍s�̏�����ł��؂��Ă������̂���
			   �X���^�C�g�����擾�������ꍇ������̂ŁA
			   ���̏ꍇ�̓t���O�𗧂Ăđ��s(�T */
			if (chunk == 0 && linenum == 0)
				aborted = 1;
		} else {
			/* �擾���������� L-M �Ƃ��ăZ�b�g���Ă��� */
			lastmod = mktime(&datetime);
		}
		/* date�̎c��A�{�����X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->date.p = &p[ofs];
			else if (i == 1)
				line->text.p = &p[ofs];

			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}

			if (i == 0)
				line->date.len = &p[ofs] - line->mailto.p;
			else if (i == 1)
				line->text.len = &p[ofs] - line->date.p;

			ofs += 2;
		}
		/* �����܂ŗ��ꂽ�炠�ځ[��ł͂Ȃ��ƌ��Ȃ�
		   (�� aborted ���Z�b�g����Ă�ꍇ )*/
		if (!aborted) {
			n = DATINDEX_CHUNK_SIZE * chunk + linenum;
			idx->valid_bitmap[n >> 5] |= 1 << (n & 31);
			completed = 1;
		}
		/* �X���^�C�g���擾�����݂� */
		if (chunk == 0 && linenum == 0) {
			idx->title_ofs = ofs;
		}
		ofs += 1 + strcspn(&p[ofs], "\n");

	can_not_parse:
		if (!completed) {
			/* �s�̉�͂��������Ă��Ȃ��ꍇ */

			static struct DATINDEX_LINE ab =
			{
				{"���ځ[��", 8},
				{"���ځ[��", 8},
				{"���ځ[��", 8},
				{"���ځ[��", 8},
				0, 0
			};

			*line = ab;
		}

		line->lastmod = lastmod;
		line->len = ofs - orig_ofs;	/* \n �܂ł̒��� */

		/* ���ځ[��s�ł���Ƃ��́A
		   lastmod ���ŐV�����̂܂܂ł���͂� */
		if (idx->idx[chunk].lastmod < lastmod)
			idx->idx[chunk].lastmod = lastmod;
	}

	/* linenum �� CHUNK_SIZE �𒴂��Ă�ꍇ�����邪�A
	   ���L�̎��͓K�؂ɓ����B */
	idx->linenum = DATINDEX_CHUNK_SIZE * chunk + linenum;

	return 1;
}
/****************************************************************
 *	�q�[�v�ɗ̈���m�ۂ��A�C���f�N�X�����ݏグ��
 *	�������������s��Ȃ�
 *	�����͍쐬����INDEX��Ԃ����Ƃɒ���
 ****************************************************************/
static DATINDEX *create_local_index(DATINDEX_OBJ *dat)
{
	DATINDEX *idx = calloc(1, sizeof(DATINDEX));
	if (!idx)
		return 0;
	dat->shared_idx = idx;
	return (buildup_index(dat,
			      0,	/* �J�n�s */
			      idx,
			      0,	/* �t�@�C���I�t�Z�b�g */
			      dat->dat_stat.st_size)
		? idx
		: NULL);
}
/****************************************************************
 *	�C���f�N�X�t�@�C�����쐬���Ă݂�
 *	�쐬�ł��Ȃ��Ƃ��̓��[�J���ɍ��
 ****************************************************************/
static int create_index(DATINDEX_OBJ *dat,
			char const *fn)
{
	int fd;
	DATINDEX *tidx;

	/* �܂��A���[�J���ɃC���f�N�X������Ă݂�
	   dat���e�t�B�[���h�́A���[�J�����e��
	   ���������̂ŁA���̂܂܋A���Ă����ȂȂ��B */
	tidx = create_local_index(dat);
	if (!tidx) {
		/* .dat�t�@�C�����s���������Ƃ݂Ȃ� */
		return 0;
	}

	fd = open(fn, O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0) {
		/* ���Ƃ��΁Aidx�f�B���N�g����
		   ���݂��Ȃ������ꍇ�ł��Aopen(2)��
		   ���s����B�C���f�N�X�̓��[�J���ɍ쐬����� */
		return 0;
	}

	/* �t�@�C���̈�����o��
	   �쐬���Ă������ private idx �������o��*/
	if (write(fd, tidx, sizeof(DATINDEX)) < 0) {
		/* XXX �{���Ȃ�� unlink ������������ */
		return 0;
	}
	/* �������������̂��Amap���� */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		return 0;
	}

	/* �Ō�ɁA�����Ȃǂ���������� */
	dat->shared_idx->signature = DATINDEX_VERSION;
	dat->shared_idx->version   = DATINDEX_VERSION;

	return 1;
}
/****************************************************************
 *	�{��.dat����������
 *	�Œ�ł��邩�ǂ������킵�Ă݂�
 *	���܂�BigBuffer�̂��Ƃ͍l�����ĂȂ�
 *	.dat���J�����W���[���́A���ꂾ���ł���B
 ****************************************************************/
static int open_dat(DATINDEX_OBJ *dat,
		    char const *bs, long ky)
{
	int fd;
	char fn[64];

	/* �s�C���f�N�X���A��̏�Ԃō쐬 */
	dat->line = calloc(sizeof(dat->line[0]),
			   DATINDEX_MAX_ARTICLES);
	if (!dat->line)
		return 0;

	sprintf(fn, "../%.64s/dat/%ld.dat", bs, ky);
	fd = open(fn, O_RDONLY);
	if (fd < 0)
		return 0;
	if (fstat(fd, &dat->dat_stat) < 0)
		return 0;
	dat->private_dat = mmap(NULL,
				dat->dat_stat.st_size + 1,
				PROT_READ,
				MAP_PRIVATE,
				fd,
				0);
	if (dat->private_dat == MAP_FAILED)
		return 0;

	/* ���������Ƃ������Ƃɂ��Ă�����
	   ������� fd �̌�n���̓T�{�� XXX */

	return 1;
}
/****************************************************************
 *	�C���f�N�X�t�@�C�����������ɌŒ肷��
 *	�Œ�ł��Ȃ��Ƃ��́A�G���[��Ԃ�
 ****************************************************************/
int datindex_open(DATINDEX_OBJ *dat,
		  char const *bs, long ky)
{
	int fd;
	int ver;

	char fn[64];

	/* �܂��͖{��.dat����������
	   �Œ�ł��邩�ǂ������킵�Ă݂�
	   ���܂�BigBuffer�̂��Ƃ͍l�����ĂȂ� */
	if (!open_dat(dat, bs, ky))
		return 0;

	/* idx �̃t�@�C���n���h�����I�[�v�� */
	sprintf(fn, "../%.64s/" DATINDEX_PATH "/%ld.idx", bs, ky);
	fd = open(fn, O_RDWR);
	if (fd < 0) {
		/* �C���f�N�X���쐬
		   (����������ƃ��[�J����
		   ����邩������Ȃ�) */
		return create_index(dat, fn);
	}

	/* �܂��� map ������󂯂� */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		/* ���͂ŃC���f�N�X��� */
		return !!create_local_index(dat);
	}

	/* �C���f�N�X�̃o�[�W���������� */
	ver = dat->shared_idx->version;
	if (ver == 0) {
		/* �v�������莞�Ԃ��o���ĂȂ�����? */
		struct stat idx_stat;
		if (fstat(fd, &idx_stat) >= 0
		    && time(NULL) >= idx_stat.st_mtime + DATINDEX_EXPIRATION
		    && unlink(fn) >= 0) {
			/* ���܂莞�Ԃ��o�������ȃ��c�́A
			   ���葹�˂ƍl���A
			   �������Ǎ쐬�����݂� */
			return create_index(dat, fn);
		}

		/* ���͂ŃC���f�N�X���� */
		return !!create_local_index(dat);
	} else if (ver > DATINDEX_VERSION) {
		/* ���m�̃o�[�W�����Ȃ̂ŋ����Q���� */
		return !!create_local_index(dat);
	} else if (ver < DATINDEX_VERSION
		   || dat->shared_idx->signature != DATINDEX_VERSION) {
		/* �ׂ��č�蒼���� */
		if (unlink(fn) >= 0)
			return create_index(dat, fn);
	}

	/* ���ׂĂ�������
	   �������炪�z���g�̓���
	   �܂��́A����index�������Ă���s�����
	   �i��ł��邩�ǂ����`�F�b�N */

	/* �i��ł��镪�̍s���A�s�C���f�N�X(���[�J����)
	   �����Ƃ� */
	
	/* �i�񂾕���\�� */

	/* �\�����ʂ��āA���Achunk boundary��
	   �܂�������Achunk index���X�V����`���𕉂� */

	return 1;
}

/*EOF*/
