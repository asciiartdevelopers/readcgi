/*
 * read.cgi����HTML�����𔲂��o�������́B
 *
 * �ҏW��̒��ӁB
 * �E%s�Ȃǂ̐��Ə��Ԃ�ς��Ă͂����܂���(%s,%d,%ld)
 * �E���̍s�֑�����Ƃ��� \<���s> �ł��B
 * �E" �� \"�A���s��\n�A%��%%�ł��B
 * �E���X��������\���L��܂�������͂��܂��Ȃ��ł��B�����Ȃ��łˁB
 * �ECGIVER�� ""�ł�����Ȃ����ƁB
 * �E�R�����g�� �^���`���^ (���p��)�B
 * �E�����ł��A�Ȃ��Ȃ��B
 */

/*
 * BANNERS: �X�����������̃G���[���ɏo��L���B
 */
#define	R2CH_HTML_NEW_BANNER \
	"<table cellspacing=0 cellpadding=0 border=1 width=100%%><tr><td>" \
	"<table cellspacing=0 cellpadding=0 border=0 width=100%%>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr bgcolor=#cccccc><td height=20 nowrap><font size=2>&nbsp;" \
	"| <a href=\"http://www.maido3.com/\" class=BigLine>��������</a>" \
	"| <a href=\"http://news.kakiko.com/2chwatch/\" class=BigLine>2ch�T�[�o�Ď���</a>" \
	"| <a href=\"http://www.binboserver.com/\" class=BigLine>BinboServer</a>" \
	"| <a href=\"http://www.maido3.com/server/subdomain.html\" class=BigLine>Hikky.Zansu.com</a>" \
	"| <a href=\"http://news.kakiko.com/\" class=BigLine>�����T�[�r�X</a>" \
	"|</font></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"</table>" \
	"<br>" \
	"<center>" \
	"���ԃh���C���V���[�Y�o��<br>" \
	"�@�@�@*****.<b><font size=+1 face=Arial color=#ff1493>syo-ten</font></b>.com<br>" \
	"�@�@�@*****.<b><font size=+1 face=Arial color=#ff1493>gasuki</font></b>.com<br>" \
	"�@�@�@*****.<b><font size=+1 face=Arial color=#ff1493>zansu</font></b>.com<br>" \
	"�������Ȗ��O�𖳗��Ŏg���܂��B�����ҏ����B<br>" \
	"<b><font color=#ff1493>�H�v�������ŁA�y����100�{�B</font></b>" \
	"<p><a href=\"http://www.maido3.com/server/\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X(n)" \
	"</a>" \
	"<br>���̋���ȃT�C�g���g���Ă��܂��B" \
	"</center><p>" \
	"</td></tr></table><p>"

/* �g���ĂȂ�...�͂��B */
#define	R2CH_HTML_BBSPINK_BANNER \
	"<center>" \
	"<a href=\"http://www.pinknavi.com/?pinkid=PB0001&prg=3&act=0&txt=�����ł܂�&url=http://www.seiai.com/\" target=\"_blank\">�����ł܂��A�_���g�T�C�g</a><br>" \
	"<br><a href=\"http://www.maido3.com/mentai/\"><b><font size=4>�[��ł����K�ȃ����^���T�[�o�T�[�r�X</font></b></a><br>" \
	"</center>" \
	"<p>"

/* �g���ĂȂ�...�͂��B */
#define	R2CH_HTML_BANNER \
	"<center>" \
	"<a href=\"http://www.maido3.com/server/\" target=\"_blank\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X" \
	"</a>" \
	"<br>���̋���ȃT�C�g���g���Ă��܂��B" \
	"</center><p>"

/*
 * BROWSER�Ń��X��\���B
 */
/* ���X(mailto����): %d=���X�ԍ�  %s=mailto %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_MAIL \
	"<dt>%d ���O�F<a href=\"mailto:%s \"><b>%s </b></a> ���e���F%s<dd>%s<br><br>"

/* ���X(mailto����): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_NOMAIL \
	"<dt>%d ���O�F<font color=green><b>%s </b></font> ���e���F%s<dd>%s<br><br>"

/* ���X(sage): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_SAGE \
	"<dt>%d ���O�F<font color=#0000c0><b>%s </b></font> ���e���F%s<dd>%s<br><br>"

/* ���X(�������Ă��܂�): %d=���X�ԍ� */
#define R2CH_HTML_RES_BROKEN_HERE \
	"<dt>%d ���O�F ���e���F[�������Ă��܂�]<dd>[�������Ă��܂�]<br><br>"

/* tail: npath=��������URL����1 nst=����xxx���X
	 lpath=��������URL����2 ls=�ŐV���Xxxx
         %02d=�����J�n���� %02d=�����I������ */
#define R2CH_HTML_T_TAIL(npath, nst, lpath, ls) \
	" <a href=\"" npath "\">����" nst "���X</a>" \
	" <a href=\"" lpath "\">�ŐV���X" ls "</a>\n" \
	" (%02d:00PM - %02d:00AM �̊Ԉ�C�ɑS���͓ǂ߂܂���)<br>\n"

/* tail: PATH�i�V
	%s=cgi %s=board %s=key %d=�J�n %d=�I�� %d=���X��
	%s=cgi %s=board %s=key %d=���X�� %d=���X��
	%02d=�����J�n���� %02d=�����I������ */
#define R2CH_HTML_TAIL \
	R2CH_HTML_T_TAIL("./%s?bbs=%s&key=%s&st=%d&to=%d&n=t", "%d", \
			 "./%s?bbs=%s&key=%s&ls=%d&n=t", "%d")
/* tail: PATH�d�l
	%d=�J�n %d=�I�� %d=���X��
	%d=���X�� %d=���X��
	%02d=�����J�n���� %02d=�����I������ */
#define R2CH_HTML_PATH_TAIL \
	R2CH_HTML_T_TAIL("%d-%d", "%d", \
			 "./?ls=%d", "%d")

/*
 * i-MODE�Ń��X��\���B
 */
/* ���X(mailto����): %d=���X�ԍ�  %s=mailto %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_IMODE_RES_MAIL \
	"<p>[%d:<a href=\"mailto:%s \"><b>%s </b></a>(%s)]<br>%s</p><hr>"

/* ���X(mailto����): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_IMODE_RES_NOMAIL \
	"<p>[%d:<b>%s </b>(%s)]<br>%s</p><hr>"

/* ���X(�������Ă��܂�): %d=���X�ԍ� */
#define R2CH_HTML_IMODE_RES_BROKEN_HERE "<p>[%d:�������Ă��܂�]</p><hr>"

/* tail: %s=cgi %s=board %s=key %d=�J�n %d=�I�� %d=���X�� %s=cgi %s=board %s=key %d=���X�� %d=���X�� */
#define R2CH_HTML_IMODE_TAIL \
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d&imode=true\">����%d���X</a>\n" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d&imode=true&n=t\">�ŐV���X%d</a><br>\n"

/*
 * ERROR
 * ERROR1, ERROR2?, ERROR3, BANNER, ERROR4, (ERROR5_HTML|ERROR5_DAT|ERROR5_NONE)?, ERROR_6
 */

/* �G���[(�₢���킹��): */
#define R2CH_HTML_ERROR_ADMIN \
	"<a href=\"http://green.jbbs.net/computer/20/bigserver.html\">���₢���킹��</a>"

/* �G���[(1/6): %s=�G���[���b�Z�[�W %s=�G���[���b�Z�[�W %s=�G���[���b�Z�[�W %s=�R�����g */
#define R2CH_HTML_ERROR_1 \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>%s</title>" \
	"<style type=\"text/css\">" \
	"<!--\n" \
	"td.Type1 {" \
	"color: #fff;" \
	"text-align: left;" \
	"}" \
	"a.BigLine {" \
	"color: #000;" \
	"text-decoration: none;" \
	"}\n" \
	"-->" \
	"</style>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<p><font size=+1 color=red>%s</font></b><br>" \
	"<dl>" \
	"<dt>1 ���O�F<font color=green><b>" \
	CGIVER \
	"</b></font>���e���F2001/04/12(��) 15:11<dd>%s<br><br>%s<br>\n"

/* �G���[(2/6): ������[����̂݁B */
#define R2CH_HTML_ERROR_2 \
	"<dt>2 ���O�F<font color=green><b>" CGIVER "</b></font>���e���F2001/04/12(��) 15:11<dd>"\
	"<br>" \
	"<p><a href=\"http://www.maido3.com/server/\" target=\"_blank\">" \
	"http://www.maido3.com/server/" \
	"</a><br>" \
	"���ԃh���C���V���[�Y�o��<br>" \
	"�@�@�@*****.<b><font size=+1 face=\"Arial\" color=deeppink>syo-ten</font></b>.com<br>" \
	"�@�@�@*****.<b><font size=+1 face=\"Arial\" color=deeppink>gasuki</font></b>.com<br>" \
	"�@�@�@*****.<b><font size=+1 face=\"Arial\" color=deeppink>zansu</font></b>.com<br>" \
	"�������Ȗ��O�𖳗��Ŏg���܂��B�����ҏ����B<br>" \
	"<font color=deeppink><b>�H�v�������ŁA�y����100�{�B</b></font>" \
	"<p><a target=\"_blank\" href=\"http://www.maido3.com/server/\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X" \
	"</a>" \
	"<br>���̋���ȃT�C�g���g���Ă��܂��B<br><br>" \
	"<table cellspacing=0 cellpadding=0 border=0 width=100%%>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr bgcolor=#cccccc><td height=20 nowrap><font size=2>&nbsp;" \
	"| <a href=\"http://www.maido3.com/\" class=\"BigLine\" target=\"_blank\">��������</a>" \
	"| <a href=\"http://news.kakiko.com/2chwatch/\" class=\"BigLine\" target=\"_blank\">2ch�T�[�o�Ď���</a>" \
	"| <a href=\"http://www.binboserver.com/\" class=\"BigLine\" target=\"_blank\">BinboServer</a>" \
	"| <a href=\"http://www.maido3.com/server/subdomain.html\" class=\"BigLine\" target=\"_blank\">Hikky.Zansu.com</a>" \
	"| <a href=\"http://news.kakiko.com/\" class=\"BigLine\" target=\"_blank\">�����T�[�r�X</a>" \
	"|</font></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#efefef><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"<tr><td height=1 bgcolor=#333333><img height=1 width=1 src=\"/space1.gif\"></td></tr>" \
	"</table>" \
	"<br><br>"

/* �G���[(3/6): */
#define R2CH_HTML_ERROR_3 \
	"</dl><center>\n<br>"

/* �G���[(4/6): */
#define R2CH_HTML_ERROR_4 \
	"<br></center><HR><font size=-2>" CGIVER "</font><HR><p>"

/* �G���[(5a/6,HTML): %s=�ꏊ %s=�X���ԍ� */
#define R2CH_HTML_ERROR_5_HTML \
	"����! �ߋ����O�q�ɂŁA<a href=\"%s\">�X���b�h %s.html</a> �𔭌����܂����B\n"

/* �G���[(5b/6,DAT): %s=�ꏊ %s=�X���ԍ� */
#define R2CH_HTML_ERROR_5_DAT \
	"����! �ߋ����O�q�ɂŁA<a href=\"%s\">�X���b�h %s.dat</a> �𔭌����܂����B<br><b><font size=+1 color=red>��</font></b>�A�������܂� html������Ă��܂���B�c�O�A�҂����Ȃ��B\n"

/* �G���[(5c/6,NONE): %s=�� */
#define R2CH_HTML_ERROR_5_NONE \
	"<a href=\"/%s/kako/\">�ߋ����O�q��</a>�ɂ�����܂���ł����B<br>�₢���킹�Ă�������\\���͂قƂ�ǂ���܂���B\n"

/* �G���[(6/6): */
#define R2CH_HTML_ERROR_6 \
	"</body></html>"

/* �G���[999: %s=�G���[���b�Z�[�W %s=board %s=key %s=ls %s=st %s=to %s=nofirst %d=file-size %d=max-line
              %s=time(hh:mm:ss) %s=board %s=soko %s=key %s=key */
/* �g���ĂȂ�...�͂��B */
#define R2CH_HTML_ERROR_999_1 \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>ERROR</title>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<table border=1 cellspacing=11 cellpadding=2 width=95%% bgcolor=#efefef align=center><tr><td>" \
	"����[�A%s<p>" \
	"bbs = [%s]�@" \
	"key = [%s]�@" \
	"ls = [%s]�@" \
	"st = [%s]�@" \
	"to = [%s]�@" \
	"nofirst = [%s]<br>" \
	"fileSize = [%d bytes] %dlines<br>" \
	"time=[%s]<br>" \
	"<br>�u����[�A����Ȕ�or�X���b�h�Ȃ��ł��v�̏ꍇ�͂����q�ɂ� Go <br>" \
	"<a href=\"/%s/kako/%s/%s.html\">�ߋ����O�q�� %s.html</a>\n" \
	"<br>�܂��Adat -> html �ϊ����s���Ă��Ȃ��ꍇ���ǂ�����܂��B" \
	"<br><a href=\"http://teri.2ch.net/test/read.cgi?bbs=accuse&key=980507715&ls=30\">����</a>�ői����ƁA�ߋ����O���������Ă����ꍇ�����邩���ł��B" \
	"</td></tr></table>" \
	"<br><br><br>"

/* �g���ĂȂ�...�͂��B */
#define R2CH_HTML_ERROR_999_2 \
	"<br><br><br>" \
	"<font size=-2>" CGIVER "</font>" \
	"</body>" \
	"</html>"

/*
 * HEADER:
 * ((HEADER ALL_ANCHOR CHUNK_ANCHOR LATEST_ANCHOR)|IMODE_HEADER)
 * (HEADER_REDZONE|HEADER_REDZONE|HEADER_REDZONE)
 * HEADER_2
 */
#ifndef COOKIE
#define R2CH_HTML_COOKIE_SCRIPT \
	"<script language=JavaScript>" \
	"<!--\n" \
	"N=g(\"NAME\");" \
	"M=g(\"MAIL\");" \
	"function g(key,tmp1,tmp2,xx1,xx2,xx3){" \
	"tmp1=\" \"+document.cookie+\";\";" \
	"xx1=xx2=0;" \
	"len=tmp1.length;" \
	"while(xx1<len){" \
	"xx2=tmp1.indexOf(\";\",xx1);" \
	"tmp2=tmp1.substring(xx1+1,xx2);" \
	"xx3=tmp2.indexOf(\"=\");" \
	"if(tmp2.substring(0,xx3)==key){" \
	"return(unescape(tmp2.substring(xx3+1,xx2-xx1-1)));" \
	"}" \
	"xx1=xx2+1;" \
	"}return(\"\");" \
	"}\n" \
	"// -->" \
	"</script>" 
#else
#define R2CH_HTML_COOKIE_SCRIPT ""
#endif

/* �u���E�U�Ō����Ƃ�: %s=�X���� %s=�� */
#define R2CH_HTML_HEADER_1 \
	"<html>" \
	"<head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>%s</title>" \
	R2CH_HTML_COOKIE_SCRIPT \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<a href=\"/%s/index2.html\">���f���ɖ߂遡</a>"

/* path=��������URL */
#define R2CH_HTML_T_ALL_ANCHOR(path) \
	" <a href=\"" path "\">���X��S���ǂ�</a>"
/* path=��������URL st=�J�n�ʒu */
#define R2CH_HTML_T_CHUNK_ANCHOR(path, st) \
	" <a href=\"" path "\">" st "-</a>"
/* path=��������URL ls=�L���� */
#define R2CH_HTML_T_LATEST_ANCHOR(path, ls) \
	" <a href=\"" path "\">�ŐV���X"ls"</a>"

/* �ȉ��̂��̂́APATH�i�V�d�l�ŗp������ */

/* %s=�� %s=�X���ԍ� */
#define R2CH_HTML_ALL_ANCHOR \
	R2CH_HTML_T_ALL_ANCHOR(CGINAME "?bbs=%s&key=%s")
/* %s=�� %s=�X���ԍ� %d=�J�n���X %d=�I�����X %s="&n=f" %d=�J�n���X */
#define R2CH_HTML_CHUNK_ANCHOR \
	R2CH_HTML_T_CHUNK_ANCHOR(CGINAME "?bbs=%s&key=%s&st=%d&to=%d%s", "%d")
/* %s=�� %s=�X���ԍ� %d=���X�� %d=���X�� */
#define R2CH_HTML_LATEST_ANCHOR \
	R2CH_HTML_T_LATEST_ANCHOR(CGINAME "?bbs=%s&key=%s&ls=%d&n=t", "%d")

/* �ȉ��̂��̂́APATH�d�l�ŗp������
   �E�X���Ԃ͂��łɌ��肵�Ă���̂ŁA���ߍ��܂�Ȃ� */

/* �O���p�����[�^�i�V */
#define R2CH_HTML_PATH_ALL_ANCHOR \
	R2CH_HTML_T_ALL_ANCHOR("1-")
/* %d=�J�n���X %d=�I�����X %d=�J�n���X */
#define R2CH_HTML_PATH_CHUNK_ANCHOR \
	R2CH_HTML_T_CHUNK_ANCHOR("%d-%d", "%d")
/* %d=���X�� %d=���X�� */
#define R2CH_HTML_PATH_LATEST_ANCHOR \
	R2CH_HTML_T_LATEST_ANCHOR("./?ls=%d", "%d")


/* i-Mode�Ō����Ƃ�: %s=�X���� %s=�� %s=�� %s=�X���ԍ� %d=��x�ɕ\�����郌�X��
                     %s=�� %s=�X���ԍ� %d=��x�ɕ\�����郌�X�� %d=��x�ɕ\�����郌�X�� */
#define R2CH_HTML_IMODE_HEADER_1 \
	"<html>" \
	"<head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>%s</title>" \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<a href=\"/%s/i/\">���f���ɖ߂遡</a>" \
	" <a href=\"" CGINAME "?bbs=%s&key=%s&st=1&to=%d&imode=true\">���X���ŏ�����ǂ�</a>" \
	" <a href=\"" CGINAME "?bbs=%s&key=%s&ls=%d&imode=true&n=t\">�ŐV���X%d</a>"

/* ���X���I�[�o�[: %d=�ő僌�X�� */
#define R2CH_HTML_HEADER_RED \
	"<p><table><tr><td bgcolor=red>" \
	"<br><br><font color=white>���X����%d�𒴂��Ă��܂��B�c�O�Ȃ���S���͕\\�����܂���B</font>" \
	"</td></tr></table>"

/* ���X����΂�: %d=��΂��Ȃ郌�X�� */
#define R2CH_HTML_HEADER_REDZONE \
	"<p><table><tr><td bgcolor=red>\n" \
	"<font color=white>���X����%d�𒴂��Ă��܂��B%d�𒴂���ƕ\\���ł��Ȃ��Ȃ��B</font>" \
	"</td></tr></table>"

/* ���X�����낻��?: %d=�x���̏o�郌�X�� */
#define R2CH_HTML_HEADER_YELLOW \
	"<p><table><tr><td bgcolor=yellow>" \
	"���X����%d�𒴂��Ă��܂��B%d�𒴂���ƕ\\���ł��Ȃ��Ȃ��B" \
	"</td></tr></table>"

/* �X����: %s=�X���� */
#define R2CH_HTML_HEADER_2 \
	"<p><font size=+1 color=red>%s</font>"

/*
 * RELOAD
 */
#define R2CH_HTML_T_RELOAD(path) \
	"<hr><center><a href=\"" path "\">�V���X�̕\\��</a></center><hr>"

#define R2CH_HTML_RELOAD \
	R2CH_HTML_T_RELOAD("read.cgi?bbs=%s&key=%s&st=%d&n=t")

#define R2CH_HTML_PATH_RELOAD \
	R2CH_HTML_T_RELOAD("%d-")

#define R2CH_HTML_RELOAD_I \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d&i=t&n=t\">�V���X�̕\\��</a></center><hr>"

/*
 * FOOTER
 * (FORM? , FOOTER) | FORM_IMODE
 */
#ifndef COOKIE
/* %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FORM \
	"<form method=post action=\"bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<script language=JavaScript>" \
	"<!--\n" \
	"document.write(\"<input type=text name=FROM size=19 value=\",N,\"> E-mail<font size=1> (�ȗ���) </font>: <input type=text name=mail size=19 value=\",M,\">\");\n" \
	"// -->" \
	"</script>" \
	"<noscript>" \
	"<input type=text name=FROM size=19>" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<input type=text name=mail size=19>" \
	"</noscript>" \
	"<br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#else
#define R2CH_HTML_FORM \
	"<form method=post action=\"bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<input type=text name=FROM size=19 value=\"%s\">" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<input type=text name=mail size=19 value=\"%s\">" \
	"<br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#endif
	
/* �u���E�U�̂Ƃ��̃t�b�^: %s=���e�Җ� %s=mail %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FOOTER \
	"</dl><p>" CGIVER "</body></html>"

/* i-MODE�̂Ƃ���FORM: %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FORM_IMODE \
	"<form method=post action=\"./bbs.cgi\">\n" \
	"<input type=submit balue=\"��������\" name=submit> " \
	"NAME�F<input type=text name=FROM size=20 value=\"\">" \
	"MAIL�F<input type=text name=mail size=20 value=\"\">" \
	"<br>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"<textarea rows=5 cols=60 wrap=off name=MESSAGE></textarea>" \
	"</form><br>"

/* i-MODE�̂Ƃ��̃t�b�^: %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FOOTER_IMODE \
	"<p>" CGIVER "</body></html>"

