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
#define R2CH_HTML_RES_MAIL(n, l, m, nm, d, t) \
	"<dt>" n " ���O�F<a name=" l " href=\"mailto:" m " \"><b>" nm " </b></a> ���e���F" d "<dd>" t "<br><br>"

/* ���X(mailto����): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_NOMAIL(n, l, nm, d, t) \
	"<dt>" n " <a name=" l ">���O�F<font color=green><b>" nm " </b></font></a> ���e���F" d "<dd>" t "<br><br>"

/* ���X(sage): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_SAGE(n, l, nm, d, t) \
	"<dt>" n " <a name=" l ">���O�F<font color=#0000c0><b>" nm " </b></font></a> ���e���F" d "<dd>" t "<br><br>"

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
	"<style type=\"text/css\"><!--\n" \
	"td.Type1{" \
	"color:#fff; " \
	"text-align:left;" \
	"} " \
	"a.BigLine{" \
	"color:#000; " \
	"text-decoration:none;" \
	"}\n" \
	"--></style>" \
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
	"<br></center><hr><font size=-2>" CGIVER "</font><hr><p>"

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
	"<script type=\"text/javascript\" defer><!--\n" \
	"function l(e){" \
	"var N=g(\"NAME\"),M=g(\"MAIL\"),i;" \
	"with(document) for(i=0;i<forms.length;i++)" \
	"if(forms[i].FROM&&forms[i].mail)" \
	"with(forms[i]){FROM.value=N;mail.value=M;}" \
	"}" \
	"onload=l;" \
	"function g(key,tmp1,tmp2,xx1,xx2,xx3,len){" \
	"tmp1=\" \"+document.cookie+\";\";" \
	"xx1=xx2=0;" \
	"len=tmp1.length;" \
	"while(xx1<len){" \
	"xx2=tmp1.indexOf(\";\",xx1);" \
	"tmp2=tmp1.substring(xx1+1,xx2);" \
	"xx3=tmp2.indexOf(\"=\");" \
	"if(tmp2.substring(0,xx3)==key)" \
	"return unescape(tmp2.substring(xx3+1,xx2-xx1-1));" \
	"xx1=xx2+1;" \
	"}" \
	"return \"\";" \
	"}\n" \
	"//--></script>" 
#else
#define R2CH_HTML_COOKIE_SCRIPT ""
#endif

/* �u���E�U�Ō����Ƃ�: %s=�X���� %s=�� */
#define R2CH_HTML_HEADER_1(title, board) \
	"<html>" \
	"<head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<title>" title "</title>" \
	R2CH_HTML_COOKIE_SCRIPT \
	"</head>" \
	"<body bgcolor=#efefef text=black link=blue alink=red vlink=#660099>" \
	"<a href=\"" board "\">���f���ɖ߂遡</a>"

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
	"<p><font size=+1 color=red>%s</font>" \
	"<dl>"

#define R2CH_HTML_HEADER_2_I \
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
#define R2CH_HTML_FORM(depth, bs, ky, tm) \
	"<form method=POST action=\"" depth "bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<input name=FROM size=19>" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<input name=mail size=19>" \
	"<br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=" bs ">" \
	"<input type=hidden name=key value=" ky ">" \
	"<input type=hidden name=time value=" tm ">" \
	"</form>"
#else
#define R2CH_HTML_FORM \
	"<form method=POST action=\"bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<input name=FROM size=19 value=\"%s\">" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<input name=mail size=19 value=\"%s\">" \
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
	"<form method=POST action=\"./bbs.cgi\">\n" \
	"<input type=submit value=\"��������\" name=submit> " \
	"NAME�F<input name=FROM size=20 value=\"\">" \
	"MAIL�F<input name=mail size=20 value=\"\">" \
	"<br>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"<textarea rows=5 cols=60 wrap=off name=MESSAGE></textarea>" \
	"</form><br>"

/* i-MODE�̂Ƃ��̃t�b�^: %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FOOTER_IMODE \
	"<p>" CGIVER "</body></html>"

/* html_error()�p�G���[���b�Z�[�W */
#define ERRORMES_TOO_HUGE "���̃X���b�h�傫�����܂��B"
#define ERRORMES_NOT_FOUND "����Ȕ�or�X���b�h�Ȃ��ł��B"
#define ERRORMES_NO_MEMORY "�������̊m�ۂɎ��s���܂����B"
#define ERRORMES_MAINTENANCE "�������B�B�B"
#define ERRORMES_LOGOUT "�Ȃ񂩕s���ł��B"


/* �X���_�C�W�F�X�g�pHTML */

#define R2CH_HTML_INDEX_HEADER(title1, title2) \
"<head><meta http-equiv=Content-Type content=\"text/html; charset=Shift_JIS\">

<style type=\"text/css\"><!--
body{margin: 2%%;}
h3{color:red}
img{border:solid 0}
div{border:.5em ridge;margin:1em;padding:1em}
.m{border-color:#cfc;background:#cfc}
.r{border-color:#eee;background:#eee}
dt{margin:1em 0 0}
b{color:#080}
h1,.c{text-align:center}
--></style>
<title>" title1 "</title></head><body bgcolor=white>

<h1><a href=\"http://www.2ch.net/info.html\"><img src=\"http://www.geocities.co.jp/SiliconValley-Cupertino/5657/img20010514210907.gif\" border=0></a></h1>
<div class=m>
<form method=POST action=\"../test/bbs.cgi\">
<h2>" title2 "</h2>
<dl>
<dt>�������b�聡<dd><ul>
<li>HTML�ACSS�AFLASH�Ȃǂ�<em>�T�C�g����̋Z�p</em><li>JavaScript�AVBScript�Ȃǂ�<em>�N���C�A���g�T�C�h�v���O����</em><li>Web�T�C�g�̉^�c����ъǗ��ɂ��Ă̏������E�G�k<li>2ch�ł͍r�炵�˗��͂ł��܂���</ul>
<dt>���Ⴂ�Șb�聡<dd><ul>
<li>HP�ώ@�A�l�b�g�A�C�h���A�l�b�g��̎������w<a href=\"http://kaba.2ch.net/net/index2.html\">�l�b�g������</a>�x<li>�����^���T�[�o�A�h���C�����w<a href=\"/perl/index2.html\">�����^���I��</a>�x<li>HP�̐�`�A�l�b�g���邾���Ŗׂ���b�������w<a href=\"http://cocoa.2ch.net/ad/index2.html\">��`�f����</a>�x<li>CGI�APerl�APHP�Ȃǂ̃T�[�o�T�C�h�v���O���~���O���w<a href=\"http://corn.2ch.net/php/index2.html\">WebProg</a>�x</ul>
<dt>���e�푍���X����<dd><ul><li>������HP�̕]�������߂�ꍇ���w<a href=\"/test/read.cgi?bbs=hp&key=993635902\">����HP�]���X���b�h����</a>�x<li>�X�^�C���V�[�g�ɂ��Ă̎��⁨�w<a href=\"/test/read.cgi?bbs=hp&key=996828258\">CSS�A�X�^�C���V�[�g����p�X���b�h</a>�x<li>FLASH�ɂ��Ă̎��⁨�w<a href=\"/test/read.cgi?bbs=hp&key=994898823\">FLASH�̐���Ɋւ��鎿��͂����Ł�</a>�x<li>JavaScript�ɂ��Ă̎��⁨�w<a href=\"/test/read.cgi?bbs=hp&key=997445548\">JavaScript�̎���p�X���b�h</a>�x<li>����ȊO�̎���E�ē����w<a href=\"/test/read.cgi?bbs=hp&key=995867650\">Web�T�C�g���쏉�S�җp�X���b�h</a>�x<li>�w<a href=\"/test/read.cgi?bbs=hp&key=991370397\">Dreamweaver</a>�x�A�w<a href=\"/test/read.cgi?bbs=hp&key=997848771\">�z�[���y�[�W�r���_�[</a>�x�A�w<a href=\"/test/read.cgi?bbs=hp&key=987875928\">Golive</a>�x�A�w<a href=\"/test/read.cgi?bbs=hp&key=991367944\">Photoshop�Eillustrater</a>�x���͐�p�X��������܂�</ul>
<dt>���X���b�h�����Ă�O�Ɂ�<dd><ul>
<li>�Ⴂ�ł͂Ȃ����A���o�ł͂Ȃ����A������x�m�F���܂��傤�B<li>�P������͋֎~�ł��B�����X���ɏ�������ł��������B</ul>
<dt>���Z�l�̐S����<dd><ul>
<li>�Ⴂ�E�X���Ⴂ�𔭌�������D�����U�����Ă����܂��傤�B<li>����ɃX���b�h���I��������悤�ȍs�ׂ͐T�݂܂��傤�B<li>�ʃX�����X���ɂ̓��X�������S���u�B�u���u�̔��w�v�ł��B<li>�u��C��ǂށv�̂��Y�ꂸ�ɁB<li>���؂ƂˁB</ul>
</dl>

<input type=submit value=\"�X���b�h�쐬\" name=submit><input type=hidden name=bbs value=hp><input type=hidden name=time value=998848311>
<a href=http://www.2ch.net/before.html>�������ޑO�ɓǂ�ł�</a> �b <a href=http://www.2ch.net/guide/>2ch�K�C�h</a> �b <a href=\"http://www.2ch.net/guide/faq.html\">FAQ</a></form>
<p><a href=http://210.150.210.150/sage/bbstable.html target=_blank>���f���ꗗ��</a></p>
</div>

<div class=m>
<small>"

#define R2CH_HTML_INDEX_LABEL(anchor, num, label, title) \
	"<a href=\"" anchor "/?ls=50\">" num ":</a> " \
	"<a href=\"" label "\">" title "</a>\n"

#define R2CH_HTML_INDEX_ANCHOR(anchor, num, title) \
	"<a href=\"" anchor "/?ls=50\" target=\"body\">" num ": " title "</a>\n"

#define R2CH_HTML_INDEX_AD \
"<p><a href=\"-\">�X���b�h�ꗗ</a></p>
</small>
</div>

<div class=m>
<a href=\"http://www.gekiura.com/index.shtml\"><img src=\"http://www.ceres.dti.ne.jp/~gekiura/banner/bana_m3.jpg\"></a>
<a href=\"http://software.wakusei.ne.jp/free/\"><img src=\"http://www2.kitanet.ne.jp/~mail/2ch/fplnklgo.gif\"></a>
<a href=\"http://www.uranews.com/articles/tools/\"><img src=\"http://www.uranews.com/articles/tools/utoolb.gif\"></a>
<a href=\"http://www.jyukujyo.com/\"><img src=\"http://www.jyukujyo.com/ad/jyukujyo.gif\"></a>
<a href=\"http://www.tanteifile.com\"><img src=\"http://homepage2.nifty.com/kirik/2ch/game.gif\"></a>
<a href=\"http://homepage2.nifty.com/tbks/\"><img src=\"http://homepage2.nifty.com/tbks/chiee0ffff.jpg\"></a>
<p><a href=\"http://www.2ch.net/ad.html\">�L���o�e�̂��₢���킹</a></p>
</div>


<table border=1 cellspacing=0 cellpadding=0 width=95%%><tr bgcolor=black><td colspan=2>
<table width=100%%><tr bgcolor=#FFCC44><td>
�� 2ch�́A<a href=\"http://www.maido3.com/server/usagi/news.html\" target=\"_blank\"><font color=navy size=+1><b>�����̃T�[�o</b></font></a>���g���Ă�ł��B�B�B
</td></tr></table>
</td></tr><tr><td bgcolor=black>
<font color=#CCFFCC>
�@<font size=+1 face=\"Arial\"><b>.jp</b></font> �h���C���������̂��q�l�劽�}�B<a href=\"http://welcome.maido3.jp/\" target=\"_blank\"><font size=+1 face=\"Arial\" color=red><b>maido3.jp</b></font></a><br>
�@<font size=+1 face=\"Arial\"><b>.com .net .org</b></font> �擾��s���܂��B(US$10) <a href=\"http://www.maido3.com/server/\" target=\"_blank\"><font size=+1 face=\"Arial\" color=cyan><b>maido3.com</b></font></a><br>
�@<font size=+1 face=\"Arial\" color=white><b>.cc .to .tv .fm .ca</b></font> �擾��s�͂��߂܂����B<font color=white face=\"Arial\" size=-1><b>NEW</b></font><br>
</td><td align=center>
�����z�[���y�[�W�X�y�[�X<P>
<a href=\"http://space.kakiko.com/\" target=\"_blank\"><font size=+1 face=\"Arial\" color=blue><b>Space.Kakiko.com</b></font></a><br>
</font>
</td></tr><tr bgcolor=black><td colspan=2>
<table width=100%%><tr bgcolor=black><td>
<font color=#ccffcc>���X1,000�~�����</font> <a href=\"http://www.binboserver.com/\" target=\"_blank\"><font color=cyan face=\"Arial\"><b>BinboServer.com</b></font></a>
</td><td>
<font color=#ccffcc>�Q�����˂���g���Ă���</font> <a href=\"http://www.maido3.com/server/\" target=\"_blank\"><font color=cyan face=\"Arial\"><b>Big-Server.com</b></font></a>
</td></tr></table>
</td></tr><tr bgcolor=black><td colspan=2>
<table width=100%%><tr bgcolor=#FFCC44><td align=right>
&gt;&gt; �Q�����˂�A<a href=\"http://news.kakiko.com/mentai/\"><font color=NAVY>�T�[�o�Ď���</font></a>
</td></tr></table>
</td></tr></table>"

#define R2CH_HTML_DIGEST_HEADER_1(label) \
"<div class=r><a name=\"#" label "\">"

#define R2CH_HTML_DIGEST_HEADER_2(title) \
"<h3>" title "</h3><dl>"

#define R2CH_HTML_DIGEST_FOOTER \
"<form method=POST action=\"../test/bbs.cgi\">
<input type=hidden name=bbs value=hp><input type=hidden name=key value=998774537><input type=hidden name=time value=998848311>
<input type=submit value=\"���e\" name=submit> ���O�F
<input name=FROM size=19> E-mail�F<input name=mail size=19>
<ul>
<textarea rows=5 cols=60 wrap=off name=MESSAGE></textarea>
</ul>
<a href=\"../test/read.cgi?bbs=hp&key=998774537\">�S���X</a> <a href=\"../test/read.cgi?bbs=hp&key=998774537&ls=50\">�ŐV50</a>
</form></dl>
</div>"

#define R2CH_HTML_INDEX_FOOTER \
"<p class=c>
<a href=\"http://www.honeyline.co.jp/\"><img src=\"http://honeyline.co.jp/link/img/honey8.gif\"></a> 
<a href=http://FRIEND.P-CLUB.COM/WEB/link.cgi/DDN2785/HREF/hp><img src=http://FRIEND.P-CLUB.COM/WEB/link.cgi/DDN2785/IMG/hp></a><br>
�ǂ̂悤�Ȍ`�̍폜�˗��ł�����J�����Ă��������܂�
</p>

</body>"


/* �X���C���f�N�X�pHTML */

#define R2CH_HTML_SUBBACK_HEADER \
	"<html><head>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"</head>" \
	"<body target=\"body\">" \
	"<font size=-1>\n"

#define R2CH_HTML_SUBBACK_ITEM(anchor, num, title) \
	"<a href=\"" anchor "/?ls=50\">" num ": " title "</a>\n"

#define R2CH_HTML_SUBBACK_FOOTER \
	"<div align=right><a href=\"/tech/kako/\"><b>�ߋ����O�q�ɂ͂�����</b></a></div></body></html>"
