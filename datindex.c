/*  file: datindex.c
 *
 *  �C���f�N�X�^�p
 *
 *  $Id: datindex.c,v 1.1 2001/08/31 18:36:11 2ch Exp $ */

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "datindex.h"
#include "read.h"

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
	strncpy(buf, &p[ofs], n);
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

	/* hhmmss�𓾂� */
	ofs += strcspn(&p[ofs], "<>\n0123456789");
	if (!isdigit(p[ofs]))
		return 0;
	n = strspn(&p[ofs], "/0123456789");
	if (n >= 20)
		return 0;
	strncpy(buf, &p[ofs], n);
	ofs += n;
	if (!(dp = strtok(buf, "/")))
		return 0;
	datetime->tm_hour = atoi(dp);
	if (!(dp = strtok(NULL, "/")))
		return 0;
	datetime->tm_min = atoi(dp);
	if ((dp = strtok(NULL, "/")) != NULL)
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
		       DATINDEX *idx)
{
	long ofs;
	int chunk, linenum;
	long datlen = dat->dat_stat.st_size;
	char const *const p = dat->private_dat;

	for (ofs = 0, chunk = 0, linenum = 0;
	     (ofs < datlen
	      && (DATINDEX_CHUNK_SIZE * chunk + linenum
		  < DATINDEX_MAX_ARTICLES));
	     linenum++) {
		int i, n;
		long nt;
		struct tm datetime;

		if (linenum >= DATINDEX_CHUNK_SIZE) {
			/* �J��オ�� */
			chunk++;
			linenum = 0;
			idx->idx[chunk].ofs = ofs;
			idx->idx[chunk].lastmod = 0; /* epoch(�m */
		}

		/* name<>mailto<>date id<>text<>[subject]\n */
		/* name, mailto���X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}
			ofs += 2;
		}

		/* ���t����͂���
		   ���t�������܂����������Ȃ�������A
		   ���ځ[��ƌ��Ȃ� */
		n = get_date(&datetime, &p[ofs]);
		if (n == 0) {
			/* ���ځ[��ƌ��Ȃ� */
			if (idx->idx[chunk].lastmod < dat->dat_stat.st_mtime)
				idx->idx[chunk].lastmod = dat->dat_stat.st_mtime;
			/* ���̂܂܍s�̏�����ł��؂��Ă������̂���
			   �X���^�C�g�����擾�������ꍇ������̂ŁA
			   �����Ă��� */
		} else {
			/* �擾���������� L-M �Ƃ��ăZ�b�g���Ă��� */
			nt = mktime(&datetime);
			if (idx->idx[chunk].lastmod < nt)
				idx->idx[chunk].lastmod = nt;
		}
		/* date�̎c��A�{�����X�L�b�v���� */
		for (i = 0; i < 2; i++) {
			while (!(p[ofs] == '<' && p[ofs + 1] == '>')) {
				if (p[ofs] == '\n') {
					ofs++;
					goto can_not_parse;
				}
				ofs++;
				ofs += strcspn(&p[ofs], "<>\n");
			}
			ofs += 2;
		}
		/* �X���^�C�g���擾�����݂� */
		if (chunk == 0 && linenum == 0) {
			idx->title_ofs = ofs;
		}
		ofs += strcspn(&p[ofs], "\n");
	can_not_parse:
		;
	}

	return 1;
}
/****************************************************************
 *	�q�[�v�ɗ̈���m�ۂ��A�C���f�N�X�����ݏグ��
 *	�������������s��Ȃ�
 ****************************************************************/
static int create_local_index(DATINDEX_OBJ *dat)
{
	DATINDEX *idx = malloc(sizeof(DATINDEX));
	if (!idx)
		return 0;
	/* private idx��shared idx�́A
	   �������̂�������悤�ɂ��Ă��� */
	dat->private_idx = idx;
	dat->shared_idx = idx;
	return buildup_index(dat, idx);
}
/****************************************************************
 *	�C���f�N�X�t�@�C�����쐬���Ă݂�
 *	�쐬�ł��Ȃ��Ƃ��̓��[�J���ɍ��
 ****************************************************************/
static int create_index(DATINDEX_OBJ *dat,
			char const *fn)
{
	int fd;

	/* �܂��A���[�J���ɃC���f�N�X������Ă݂� */
	if (!create_local_index(dat)) {
		/* .dat�t�@�C�����s���������Ƃ݂Ȃ� */
		return 0;
	}

	fd = open(fn, O_CREAT | O_EXCL | O_RDWR, 0666);
	if (fd < 0) {
		return create_local_index(dat);
	}

	/* �t�@�C���̈�����o�� */
	if (lseek(fd, sizeof(DATINDEX), SEEK_SET) < 0) {
		return create_local_index(dat);
	}
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		return create_local_index(dat);
	}

	/* ���̎��_�� private_idx �́A
	   �q�[�v��ɍ���Ă���͂� */
	memcpy((DATINDEX *)dat->shared_idx,
	       dat->private_idx,
	       sizeof(DATINDEX));

	/* �Ō�ɁA�����Ȃǂ���������� */
	dat->shared_idx->signature = DATINDEX_VERSION;
	dat->shared_idx->version   = DATINDEX_VERSION;

	return 1;
}
/****************************************************************
 *	�C���f�N�X�t�@�C�����������ɌŒ肷��
 *	�Œ�ł��Ȃ��Ƃ��́A�G���[��Ԃ�
 ****************************************************************/
int read_index(DATINDEX_OBJ *dat,
	       char const *bs, long ky)
{
	char fn[64];
	int fd;
	int ver;

	/* �܂��͖{��.dat����������
	   �Œ�ł��邩�ǂ������킵�Ă݂�
	   ���܂�BigBuffer�̂��Ƃ͍l�����ĂȂ� */
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

	/* �������fd�̌�n���̓T�{�� XXX */

	/* idx�̃t�@�C���n���h�����I�[�v�� */
	sprintf(fn, "../%.64s/dat/%ld.idx", bs, ky);
	fd = open(fn, O_RDWR);
	if (fd < 0) {
		/* �C���f�N�X���쐬 */
		return create_index(dat, fn);
	}

	/* �܂��� private map ������󂯂� */
	dat->private_idx = mmap(NULL,
				sizeof(DATINDEX),
				PROT_READ,
				MAP_PRIVATE,
				fd,
				0);
	if (dat->private_idx == MAP_FAILED) {
		/* ���͂ŃC���f�N�X��� */
		return create_local_index(dat);
	}

	/* �C���f�N�X�̃o�[�W���������� */
	ver = dat->private_idx->version;
	if (ver == 0) {
		/* �v�������莞�Ԃ��o���ĂȂ�����? */

		/* ���͂ŃC���f�N�X���� */
		return create_local_index(dat);
	} else if (ver > DATINDEX_VERSION) {
		/* ���m�̃o�[�W�����Ȃ̂ŋ����Q���� */
		return create_local_index(dat);
	} else if (ver < DATINDEX_VERSION
		   || dat->private_idx->signature != DATINDEX_VERSION) {
		/* �ׂ��č�蒼���� */

	}

	/* ����ł���ƁAshared map���^������ */
	dat->shared_idx = mmap(NULL,
			       sizeof(DATINDEX),
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       fd,
			       0);
	if (dat->shared_idx == MAP_FAILED) {
		/* ���͂ŃC���f�N�X���Ă� */
		return create_local_index(dat);
	}
	return 1;
}

/*EOF*/
