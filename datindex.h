/*  file: datindex.h
 *
 *  ���E���ʃC���f�N�X�̒�`
 *
 *  $Id: datindex.h,v 1.6 2001/09/03 14:53:57 2ch Exp $ */

#ifndef DATINDEX_H__
#define DATINDEX_H__

#include <time.h>
#include <sys/stat.h>

#define DATINDEX_VERSION 1

/*
 *  �������l���āA�C���f�N�X�͂Ȃ�ׂ�4096�o�C�g(page size)�ȓ���
 *  ���܂�悤�ɂ���B�t�Ɍ����ƁA4096�o�C�g��菬�������Ă��A
 *  �����������E�f�B�X�N�e�ʂ̓_�Ń����b�g�͂Ȃ��B
 *  (Linux ext2�́Afragment�����Ȃ��͂��Ȃ̂ŁA
 *   minimum allocation unit��4096�o�C�g�̃n�Y)
 *
 *  �y�^�p�̍Œ�񑩁z
 *
 *    * version ��0�̃C���f�N�X�ɑ����A
 *	���邢�̓C���f�N�X�̃T�C�Y��0��������A
 *	��{�I�ɉ��������A�C���f�N�X�ɗ��炸���͂ŏ�������B
 *	�������Aversion��0�̂܂܈�莞�Ԍo�߂������̂́A
 *	�p��-�č쐬���s���ׂ��B(�쐬�r���Ŏ��񂾂ƍl����)
 *    * version ���m���Ă���̂��傫���Ƃ��́A
 *	���������A���͂ŏ�������B
 *    * version ���m���Ă���̂�菬�����ꍇ�́A
 *	�p�����ĐV�K�쐬�B
 *
 *  �y�V�K�쐬�̂����z
 *    * creat(2)�ł���B
 *	creat()�ɏ������v���Z�X���ӔC��������
 *	�C���f�N�X���\�z����!
 *	creat()�ŕ������v���Z�X�́A���͂ŏ�������!
 *	�C���f�N�X�\�z���I�������Aversion���Z�b�g����!
 *
 *  �y�I�[�v���̂����z
 *    * MAP_SHARED�ŊJ���ׂ��B
 *
 *  �y�Â��o�[�W�����̔p���̂����z
 *    * �t�@�C�����f�B���N�g������unlink
 *    * ���Ƃ́y�V�K�쐬�z�ցB
 *
 *  �y�j��X�V�̂����z
 *    * version��0�ɂ���B
 *	version0�ɐ��������v���Z�X���A
 *	�ӔC�������Č�̍�Ƃ�����s���B
 *	version0�����Ȃ������v���Z�X�́A���͂ŏ�������!
 *    * ���낢��č\�z�I�������Aversion���Z�b�g����!
 *
 */

/* �z��L������
   �L���̂��������ł���ׂ��� */
#define DATINDEX_MAX_ARTICLES 1024

/* �C���f�N�X�P��
   CHUNK_NUM�̖񐔂ł���ׂ��� */
#define DATINDEX_CHUNK_SIZE 10

#define DATINDEX_IDX_SIZE ((DATINDEX_MAX_ARTICLES + DATINDEX_CHUNK_SIZE - 1) \
			   / DATINDEX_CHUNK_SIZE)

/* �C���f�N�X
   �T�C�Y��4096�𒴂���̂͊��}�ł��Ȃ� */
typedef struct DATINDEX
{
	/* �o�[�W�������B�j�󑀍�̃Z�}�t�H�ɂ��Ȃ��Ă��� */
	unsigned long volatile version;

	/* ������
	   chunk idx�X�V�̃Z�}�t�H�ɂ��Ȃ��Ă��� */
	unsigned long linenum;

	/* .dat �̍ŏI�X�V���� */
	time_t lastmod;

	/* �X���^�C�g���ւ̃I�t�Z�b�g */
	unsigned title_ofs;

	/* �C���f�N�X
	   �{���Ȃ�΁A�S�L���ɑ΂���
	   L-M�����������Ƃ���Ȃ̂����A
	   ��������ƃT�C�Y���傫���Ȃ肷���ğT���c

	   �e�L����L-M�́Achunk L-M����̍���(16bit)��
	   ���悤�ɂ��A����18���Ԉȏ゠����̂�
	   ���ʈ�������A�Ƃ���kludge���l�����邪�c */
	struct
	{
		unsigned nextofs;
		time_t lastmod;
		unsigned valid_bitmap;
		long pad;
	} idx[DATINDEX_IDX_SIZE];

	/* XXX ���߂���1 */
	unsigned pad1[607];
#if 0
	/* �L������ bitmap
	   little endian �Ȃ̂ŁALSB���琔���邱�ƁB
	   ���ځ[�񂳂�Ă��锭���� 0 �ɂȂ�B*/
	unsigned long valid_bitmap[DATINDEX_MAX_ARTICLES / 32];

	/* XXX ���߂���2 */
	unsigned pad2[223];
#endif

	/* �V�O�l�`�� */
	unsigned long signature;
} DATINDEX;

struct DATINDEX_STRING
{
	char const *p;
	int len;
};

struct DATINDEX_LINE
{
	/* �����̃|�C���^���m���Čv�Z���ׂ��ł͂Ȃ� XXX */
	struct DATINDEX_STRING name;
	struct DATINDEX_STRING mailto;
	struct DATINDEX_STRING date;
	struct DATINDEX_STRING text;
	int len;	/* ���v�Z�̎��� 0 ������ */
	time_t lastmod;	/* ���ځ[�񎞂ɂ� 0 ������ */
};

/* index�Ǘ�
   �ׂɃO���[�o���ϐ��ł��������ǂ� */
typedef struct DATINDEX_OBJ
{
	/* .dat�{�� */
	struct stat dat_stat;
	char *private_dat;

	/* .dat��ǂ�Ő��������e�[�u�� */
	struct DATINDEX_LINE *line;
	/* line�̌� */
	int linenum;

	/* �C���f�N�X */
	DATINDEX volatile *shared_idx;
} DATINDEX_OBJ;

/* ����: i386 �ł͓����܂��[��(�m */
#define DATINDEX_CMPXCHG(sem, cur, new) \
	({ \
		long eax; \
		__asm__ ("cmpxchg %2,%3": \
			 "=a"(eax) : \
			 "0"(cur), \
			 "r"(new), \
			 "g"(sem)); \
		eax; \
	 })

/* �j��X�V�̂��߂ɁAidx��r���擾����
   �������� ��0 (�Ƃ������DATINDEX_VERSION)��߂�
   �������� 0 ��߂�
   ����: i386 �ł͓����܂��[��(�m */
#if 0
#define DATINDEX_GET_DESTRUCTIVE_UPDATE(idx) \
	((idx)->version == DATINDEX_VERSION \
	 ? ({ \
		long eax; \
		__asm__ ("cmpxchg %2,%3": \
			 "=a"(eax) : \
			 "0"(DATINDEX_VERSION), \
			 "r"(0), \
			 "g"((idx)->version)); \
		eax; \
	 }) : 0)
#endif

/* �����K�����ԈႦ���c�T
   ���J�C���^�t�F�C�X�́A
   <module>_<method>()
   ���Ė����ɂ���悤�ɂ��悤�A����́B */

/* �C���f�N�X�� dat �ɓǂݍ���
   dat �͊��S�ɏ��������s���� */
extern int datindex_open(DATINDEX_OBJ *dat,
			 char const *bs, long ky);

/* lastmod���E���グ��
   first �́A!is_nofirst() �ł��邱�Ƃɒ��� */
extern time_t datindex_lastmod(DATINDEX_OBJ const *dat,
			       int first,	/* 1�Ԗڂ��܂߂� */
			       int st,
			       int to,
			       int ls);

/* ���͘R��AC++��D���Ȃ񂾁c
   �Ȃ�ƂȂ��������킹�鏑�����ɂȂ��Ă�ł���?(�T
   (6411) */

#endif /* DATINDEX_H__ */
