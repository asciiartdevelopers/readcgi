/* read2ch.h */

/* ���������Ӥˡ�>>000 �����Υ쥹�ؤΥ�󥯤������� */
#define CUTRESLINK

/* ���ֺǸ�ˡ��ֹ��������쥹��ɽ���פΥ�󥯤�ɽ������ */
#define RELOADLINK

/* Last-Modified ���ɲä��� */
#define LASTMOD

/* ��Ŭ�����줿 BadAccess ����Ѥ��� */
#define NEWBA

/* nofirst �� n �ʤɡ�û�̷��ǻؼ��Ǥ���褦�ˤ��� */
#define GSTR2

/* teri �����פΥ����ɤ߹��� */
#define TYPE_TERI

/* PATH_INFO��������뤳�Ȥˤ�ꡢ
   read.cgi/board/nnnnnn/?query
   �Υꥯ�����Ȥ�����Ǥ���褦�ˤʤ� */
#define USE_PATH

/* Cookie �ˤ��̾����E-mail �ե�����ɤν���ͤ������ߤ� CGI ¦�ǹԤ�
 * Last-Modified�ղäˤ�ꡢproxy�ǥ���å��夵�줿���˳Ƽ�����Թ�
 * (�ǰ��ξ�硢����åס��ȥ�åפΥѥ�ϳ��)��ȯ�����뤿������ԲĤ�
 */
/* #define COOKIE */

/* �񤭹���ľ��Υ���ɤ��ɻߤ��� */
#define PREVENTRELOAD

/* �嵭�� PREVENTRELOAD ����ꤷ���Ȥ��ˡ���������Ƥ��Ƥ� 304 ���֤��ÿ� */
#define FORCE_304_TIME 60

/* gzip ����Ѥ��ư��̤��� */
#define GZIP

/* zlib ����Ѥ��ư��̤��� */
#define ZLIB

/* raw=xxx.yyy��ͭ���ˤ��롣 xxx=�ǽ��쥹�ֹ�, yyy=���ΤȤ��Υ�������
 * ����ܤϥ��ơ�����:
 *     [+OK] �ξ��Ϻ�ʬ�Τߤ��������롣
 *     [-INCR] (Incorrect)�ξ��Ϥ��٤ƤΥǡ������������롣
 *     [-ERR (�ƥ�����)]�ξ��Ϥʤ󤫥��顼����������
 */
#define RAWOUT

/** Header custumization **/

/* �������ɤ�פ�Ĥ��� */
#undef ALL_ANCHOR 

/* page�� top�ˡ�CHUNK_NUM�֤��Ȥ˶��ڤä��쥹�ؤ� anchor��Ĥ���
 */
#define CHUNK_ANCHOR
#define CHUNK_NUM 50

/* �ֺǿ��쥹 LATEST_NUM�פ�Ĥ��� */
#define LATEST_ANCHOR
#define LATEST_NUM 50


/* sage�쥹�ΤȤ���̾���������ˤ��ʤ� */
/* #define SAGE_IS_PLAIN */

/* mmap(2) ����ѡ�
   �񸻤γ������Ѷ�Ū�˥��ܤꤿ���� */
#define USE_MMAP

/*
 *  �ɡ��Ǥ⤤����Ǥ�����C�������桢
 *  ���Υե��������EUC�Ǥ���(��
 *  �ɤ����衣�Ŀ�Ū�ˤ�����EUC�ˤǤ���ʤ�
 *  �������Ȥ���ǤϤ��롣(6411)
 */
