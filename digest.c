/* file: digest.c
   �ꗗ�_�C�W�F�X�g�f���o������ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	int i;
	int max = 0;

	pPrintf(pStdout,
		R2CH_HTML_INDEX_HEADER("%s", "%s"),
		"�v���O�����Z�p��2ch�f����",
		"�v���O�����Z�p��2ch�f����");

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
				R2CH_HTML_INDEX_LABEL("%.64s/%.*s",
						      "%d",
						      "#%d",
						      "%.*s"),
				zz_bs,
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
				R2CH_HTML_INDEX_ANCHOR("%.64s/%.*s",
						       "%d",
						       "%.*s"),
				zz_bs,
				datn, p,
				1 + i,
				subjn, subj);
		}
	}

	pPrintf(pStdout,
		R2CH_HTML_INDEX_AD("%.64s/"),
		zz_bs);

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
		char fname[1024];

		/* �t�@�C����ǂݍ���ł��� */
		sprintf(fname,
			"../%.64s/dat/%.64s.dat",
			zz_bs,
			dat_name_4digest[i]);
		dat_read(fname, 0, 0, 10);

		/* ��������������\�����Ă����� */
		pPrintf(pStdout,
			"<HR><P><A name=\"#%d\">%s\n",
			1 + i, fname);
		dat_out();
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
