/* file: digest.c
   �ꗗ�_�C�W�F�X�g�f���o������ */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "digest.h"
#include "read.h"
#include "read2ch.h"
#include "r2chhtml.h"

/****************************************************************/
/*	�_�C�W�F�X�g(index2.html)���o�͂��Ă݂�B		*/
/*	path_depth == 1 �̂͂�					*/
/****************************************************************/
void dat_out_index(void)
{
	char *dat_name_4digest[N_INDEX_DIGESTS];
	char fname[1024];
	char *html;
	int title_s = 0, title_e = 0;

	int i;
	int fd;
	int max = 0;

	/* XXX �ނ���^�C�g�����擾���Ă݂�
	   �\�͓I�ȕ��@���Ȃ� */
	sprintf(fname, "../%.64s/index2.html", zz_bs);
	fd = open(fname, O_RDONLY);
	if (fd < 0)
		html_error(ERROR_NOT_FOUND);
	html = mmap(NULL, 65536, PROT_READ, MAP_SHARED, fd, 0);
	if (html == MAP_FAILED)
		html_error(ERROR_NO_MEMORY);
	/* <TITLE> (7 chars) ��T��
	   strstr() ���g�������Ƃ��낾��
	   �Ԑl�����Ȃ��̂ŁA��ŒT�� */
	i = 0;
	while (i < 8192 - 7)
		switch (html[i + 6]) {
		default:	i += 7; continue;
		case '<':	i += 6; continue;
		case 'I':
		case 'i':	i += 4; continue;
		case 'T':
		case 't':	i += 3; continue;
		case 'L':
		case 'l':	i += 2; continue;
		case 'E':
		case 'e':	i += 1; continue;
		case '>':
			if (memcmp(&html[i], "<TITLE>", 7) != 0
			    && memcmp(&html[i], "<title>", 7) != 0) {
				/* ������Ǝ蔲�� */
				i++;
				continue;
			}
			/* found! */	
			i += 7;
			title_s = i;
			/* ����ǂ� </TITLE> (8 chars) ��T�� */
			while (i < 8192 - 8)
				switch (html[i + 7]) {
				default:	i += 8; continue;
				case '<':	i += 7; continue;
				case '/':	i += 6; continue;
				case 'I':
				case 'i':	i += 4; continue;
				case 'T':
				case 't':	i += 3; continue;
				case 'L':
				case 'l':	i += 2; continue;
				case 'E':
				case 'e':	i += 1; continue;
				case '>':
					if (memcmp(&html[i],
						   "</TITLE>",
						   8) != 0
					    && memcmp(&html[i],
						      "</title>",
						      8) != 0) {
						/* ������Ǝ蔲�� */
						i++;
						continue;
					}
					/* found! */
					title_e = i;
					goto found;
				}

			/* i �͐i��ł���͂� */
			continue;
		}

 found:
	if (title_s > 0 && title_e - title_s > 0)
		pPrintf(pStdout,
			R2CH_HTML_INDEX_HEADER("%.*s", "%.*s"),
			title_e - title_s, &html[title_s],
			title_e - title_s, &html[title_s]);
	else
		pPrintf(pStdout,
			R2CH_HTML_INDEX_HEADER("%s", "%s,%d,%d,%d"),
			"�ȂȂ���",
			"�ȂȂ���", title_s, title_e, i);

	/* fd, html, �����̕����͍s���ĂȂ��c�T�� */

	/* �X���ꗗ�̏o��
	   ���̎��_�ł͂܂���������� subject.txt �����݁B*/
	for (i = 0; i < N_INDEX_THREADS && i < lineMax; i++) {
		char const *p = BigLine[i];
		char const *subj;
		int datn, subjn;
		datn = strspn(p, "0123456789");
		if (datn == 0)
			continue;
		if (memcmp(&p[datn], ".dat<>", 6) != 0)
			continue;
		subj = p + datn + 6;
		subjn = strcspn(subj, "\r\n");
		if (subjn == 0)
			continue;
		if (i < N_INDEX_DIGESTS) {
			char *q;
			pPrintf(pStdout,
				R2CH_HTML_INDEX_LABEL("%.*s",
						      "%d",
						      "#%d",
						      "%.*s"),
				datn, p,
				1 + i,
				1 + i,
				subjn, subj);
			q = malloc(datn + 1);
			memcpy(q, p, datn);
			q[datn] = 0;
			max = i;
			dat_name_4digest[max++] = q;
		} else {
			pPrintf(pStdout,
				R2CH_HTML_INDEX_ANCHOR("%.*s",
						       "%d",
						       "%.*s"),
				datn, p,
				1 + i,
				subjn, subj);
		}
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_AD);

	/* ���ȁ[��C�x�� */
#ifndef USE_MMAP
	if (BigBuffer)
		free(BigBuffer);
#endif
	BigBuffer = NULL;
	/* ����ȍ~�A�O���[�o���ϐ���
	   �M�p�����Ȃ��Ȃ�\������
	   �Ȃ񂾂��Ђ�����Ԃ��ď�����������
	   �c�T�����̂� */

	/* �X���_�C�W�F�X�g�̏o�� */
	zz_nf[0] = 0;
	for (i = 0; i < N_INDEX_DIGESTS && i < max; i++) {
		/* ������Z�b�g���Ă����Ȃ���
		   �S�ʓI�ɒ��q���� */
		strcpy(zz_ky, dat_name_4digest[i]);

		/* �t�@�C����ǂݍ���ł��� */
		sprintf(fname,
			"../%.64s/dat/%.64s.dat",
			zz_bs,
			dat_name_4digest[i]);
		dat_read(fname, 0, 0, 10);

		/* ��������������\�����Ă����� */
		pPrintf(pStdout,
			R2CH_HTML_DIGEST_HEADER_1("%d"),
			1 + i);
		dat_out(1);
#ifndef USE_MMAP
		if (BigBuffer)
			free(BigBuffer);
#endif
		BigBuffer = NULL;
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_FOOTER);
}
/****************************************************************/
/*	�X���ꗗ(subback.html����)���o�͂��Ă݂�B		*/
/*	path_depth == 2 �̂͂�					*/
/****************************************************************/
void dat_out_subback(void)
{
	int i;

	pPrintf(pStdout, R2CH_HTML_SUBBACK_HEADER);

	/* �s��ǂݍ���ŉ�͂��Ă���
	   BigLine[]�̏����Ƃ肠�����M�p���Ă������Ƃɂ��� */
	for (i = 0; i < lineMax; i++) {
		char const *p = BigLine[i];
		char const *subj;
		int datn, subjn;
		datn = strspn(p, "0123456789");
		if (datn == 0)
			continue;
		if (memcmp(&p[datn], ".dat<>", 6) != 0)
			continue;
		subj = p + datn + 6;
		subjn = strcspn(subj, "\r\n");
		if (subjn == 0)
			continue;
		pPrintf(pStdout,
			R2CH_HTML_SUBBACK_ITEM("%.*s", "%d", "%.*s"),
			datn, p,
			1 + i,
			subjn, subj);
	}

	pPrintf(pStdout, R2CH_HTML_SUBBACK_FOOTER);
}

/*EOF*/
