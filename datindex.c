/*  file: datindex.c
 *
 *  �C���f�N�X�^�p
 *
 *  $Id: datindex.c,v 1.9 2001/09/04 07:26:48 2ch Exp $ */

#include <assert.h>
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
 *	�w�肳�ꂽ�s(������chunk�ȓ�)��
 *	�s�C���f�N�X�����肠����
 *	������shared_idx��������Ȃ�
 *	�i�񂾃I�t�Z�b�g��Ԃ�
 ****************************************************************/
static int buildup_line(struct DATINDEX_LINE *line,
			int line_n,
			char const *dat,
			int datlen,
			time_t dat_lastmod,
			int *valid_lines,
			time_t *newest_lastmod)
{
	int ofs;
	int linenum;

	/* �܂��� epoch ����n�߂� */
	*newest_lastmod = 0;

	for (ofs = 0, linenum = 0;
	     ofs < datlen && linenum < line_n;
	     line++, linenum++) {
		int i, n;
		int orig_ofs;
		struct tm datetime;

		/* �킩��Ȃ��t���O���������Ă��߂�� */
		int aborted = 0;	/* ���ځ[��? */
		int completed = 0;	/* �p�[�X���ʂ�����Z�b�g���ׂ� */
		time_t lastmod = dat_lastmod;

		orig_ofs = ofs;

		/* name<>mailto<>date id<>text<>[subject]\n */
		/* name, mailto���X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->name.p = &dat[ofs];
			else if (i == 1)
				line->mailto.p = &dat[ofs];

			while (!(dat[ofs] == '<' && dat[ofs + 1] == '>')) {
				if (dat[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&dat[ofs], "<>\n");
			}

			if (i == 0)
				line->name.len = &dat[ofs] - line->name.p;
			else if (i == 1)
				line->mailto.len = &dat[ofs] - line->mailto.p;

			ofs += 2;
		}

		/* ���t����͂���
		   ���t�������܂����������Ȃ�������A
		   ���ځ[��ƌ��Ȃ� */
		n = get_date(&datetime, &dat[ofs]);
		if (n == 0) {
			/* ���ځ[��ƌ��Ȃ�
			   ���̂܂܍s�̏�����ł��؂��Ă������̂���
			   �X���^�C�g�����擾�������ꍇ������̂ŁA
			   ���̏ꍇ�̓t���O�𗧂Ăđ��s(�T */
			aborted = 1;
		} else {
			/* �擾���������� L-M �Ƃ��ăZ�b�g���Ă��� */
			lastmod = mktime(&datetime);
		}
		/* date�̎c��A�{�����X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			if (i == 0)
				line->date.p = &dat[ofs];
			else if (i == 1)
				line->text.p = &dat[ofs];

			while (!(dat[ofs] == '<' && dat[ofs + 1] == '>')) {
				if (dat[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&dat[ofs], "<>\n");
			}

			if (i == 0)
				line->date.len = &dat[ofs] - line->mailto.p;
			else if (i == 1)
				line->text.len = &dat[ofs] - line->date.p;

			ofs += 2;
		}
		/* �����܂ŗ��ꂽ�炠�ځ[��ł͂Ȃ��ƌ��Ȃ�
		   (�� aborted ���Z�b�g����Ă�ꍇ )*/
		if (!aborted) {
			completed = 1;
		}
		/* �X���^�C�g�������邩������Ȃ����A�ǂݔ�΂� */

	can_not_parse:
		/* XXX �t�@�C���I�[�̃P�c��j��\������
		   �C������!! */
		ofs += 1 + strcspn(&dat[ofs], "\n");

		if (completed) {
			line->lastmod = lastmod;
		} else {
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

		line->len = ofs - orig_ofs;	/* \n �܂ł̒��� */

		/* ���ځ[��s�ł���Ƃ��́A
		   lastmod ���ŐV�����̂܂܂ł���͂� */
		if (*newest_lastmod < lastmod)
			*newest_lastmod = lastmod;
	}

	*valid_lines = linenum;
	return ofs;
}
/****************************************************************
 *	�p�ӂ��ꂽ�̈��
 *	�V���ɃC���f�N�X�����肠����
 *	private_dat�Ƀt�@�C�����ǂ܂�Ă���̂��O��
 *	�����͏������s��Ȃ�
 *	idx �� NULL ��^���邱�ƂŁA
 *	�s�C���f�N�X�̂ݍX�V�ł���B
 *	���������ŏI�s����Ԃ�
 ****************************************************************/
static int buildup_index(DATINDEX_OBJ *dat,
			 int linenum,	/* 0 origin */
			 DATINDEX volatile *idx,
			 long ofs,
			 long datlen)
{
	int chunk;
	char const *const p = dat->private_dat;
	int n_line_processed;

	/* .dat �� mtime �͋L�^���Ă��� */
	if (idx)
		idx->lastmod = dat->dat_stat.st_mtime;

	chunk = linenum / DATINDEX_CHUNK_SIZE;
	linenum %= DATINDEX_CHUNK_SIZE;

	for (;
	     (ofs < datlen
	      && (DATINDEX_CHUNK_SIZE * chunk + linenum
		  < DATINDEX_MAX_ARTICLES));
	     linenum += n_line_processed,
		     (linenum >= DATINDEX_CHUNK_SIZE
		      ? chunk++, linenum = 0
		      : 0)) {
		struct DATINDEX_LINE *line;
		int i, n;

		time_t chunk_lastmod;

		n = DATINDEX_CHUNK_SIZE * chunk + linenum;
		line = &dat->line[n];

		/* �s�C���f�N�X�𐶐������� */
		ofs += buildup_line(line,
				    DATINDEX_CHUNK_SIZE - linenum,
				    &p[ofs],
				    datlen - ofs,
				    dat->dat_stat.st_mtime,
				    &n_line_processed,
				    &chunk_lastmod);
		if (!idx)
			continue;

		/* �ȍ~�Aidx�̍X�V�ɓ��� */

		if (linenum == 0
		    && n_line_processed == DATINDEX_CHUNK_SIZE) {
			/* chunk�����S�ɑ��������Ă��Ȃ���
			   �C���f�N�X�͍X�V���Ȃ� */
			idx->idx[chunk].lastmod = chunk_lastmod;
			idx->idx[chunk].nextofs = ofs;

			/* �L���ȍs�ɑ΂��ăt���O���ĂĂ� */
			for (i = 0; i < n_line_processed; i++) {
				if (line[i].lastmod)
					idx->idx[chunk].valid_bitmap |= 1 << i;
			}
		}

		/* �T�u�W�F�N�g���̂�Ă�������������A�̂� */
		if (n == 0
		    && line->len
		    && memcmp(&line->text.p[line->text.len],
			      "<>",
			      2) == 0)
			idx->title_ofs = (line->text.p
					  - line->name.p
					  + line->text.len + 2);
	}

	/* linenum �� CHUNK_SIZE �𒴂��Ă�ꍇ�����邪�A
	   ���L�̎��͓K�؂ɓ����B */
	linenum = DATINDEX_CHUNK_SIZE * chunk + linenum;
	
	if (idx)
		idx->linenum = linenum;

	return linenum;
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
	if (buildup_index(dat,
			  0,	/* �J�n�s */
			  idx,
			  0,	/* �t�@�C���I�t�Z�b�g */
			  dat->dat_stat.st_size)) {
		dat->linenum = idx->linenum;
		return idx;
	} else
		return NULL;
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
	int local_ofs, local_st, current_n, old_n;
	int i;

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
	   �܂��́A�C���f�N�X�ɑ���Ȃ�
	   �s�C���f�N�X(���[�J����)�����Ƃ� */

	/* ���łɗp�ӂ���Ă���C���f�N�X���v�� */
	local_ofs = 0;
	for (i = 0; 
	     i < DATINDEX_IDX_SIZE
		     && dat->shared_idx->idx[i].nextofs != 0;
	     i++)
		local_ofs = dat->shared_idx->idx[i].nextofs;

	local_st = DATINDEX_CHUNK_SIZE * i;

	/* ���ɁA���[�J���s�C���f�N�X(�s����)���\�z */
	dat->linenum = buildup_index(dat,
				     local_st,
				     NULL,
				     local_ofs,
				     dat->dat_stat.st_size);
	/* �X�V�s���Ȃ������ꍇ�͉������邱�Ƃ��Ȃ��A�͂� */
	if (!dat->linenum)
		return 1;

	/* index���X�V���ׂ����ǂ����̔��f */
	current_n = dat->shared_idx->linenum;
	if (current_n >= dat->linenum)
		return 1;

	/* �i�񂾕���\�� */
	old_n = DATINDEX_CMPXCHG(dat->shared_idx->linenum,
				 current_n,
				 dat->linenum);

	/* �\�����ʂ�Ȃ�������A������߂ċA�� */
	if (old_n == dat->linenum)
		return 1;

	/* chunk boundary���܂����ł��Ȃ�������A��������K�v�i�V */
	if (old_n / DATINDEX_CHUNK_SIZE
	    == dat->linenum / DATINDEX_CHUNK_SIZE)
		return 1;

	/* �\�����ʂ��āA���Achunk boundary��
	   �܂�������Achunk index���X�V����`���𕉂� */
	for (i = old_n / DATINDEX_CHUNK_SIZE;
	     i < dat->linenum / DATINDEX_CHUNK_SIZE;
	     i++) {
		int j;
		dat->shared_idx->idx[i].lastmod = 0;
		for (j = 0; j < DATINDEX_CHUNK_SIZE; j++)
			if (dat->line[DATINDEX_CHUNK_SIZE * i + j].lastmod)
				dat->shared_idx->idx[i].valid_bitmap |= 1 << j;
	}

	return 1;
}

/****************************************************************
 *	lastmod���E���グ��
 *	first �́A!is_nofirst() �ł��邱�Ƃɒ���
 *	�^������p�����[�^�́A1 origin��z��B
 ****************************************************************/
time_t datindex_lastmod(DATINDEX_OBJ const *dat,
			int first,	/* 1�Ԗڂ��܂߂� */
			int st,
			int to)
{
	int i;
	int st_chunk = (st - 1) / DATINDEX_CHUNK_SIZE;
	time_t lastmod = 0;

	for (i = (first ? 0 : st_chunk);
	     i <= (to - 1) / DATINDEX_CHUNK_SIZE;
	     (i == 0 && st_chunk > 0
	      ? i = st_chunk
	      : i++)) {
		if (dat->shared_idx->idx[i].nextofs == 0) {
			if (lastmod < dat->dat_stat.st_mtime)
				lastmod = dat->dat_stat.st_mtime;
		} else if (lastmod < dat->shared_idx->idx[i].lastmod)
			lastmod = dat->shared_idx->idx[i].lastmod;
	}
	return lastmod;
}
/*EOF*/
