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
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=1 WIDTH=100%%><TR><TD>" \
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 WIDTH=100%%>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR BGCOLOR=#ccc><TD HEIGHT=20 NOWRAP><FONT SIZE=2>&nbsp;" \
	"| <A HREF=\"http://www.maido3.com/\" CLASS=BigLine> �������� </A>" \
	"| <A HREF=\"http://news.kakiko.com/2chwatch/\" CLASS=BigLine> 2ch�T�[�o�Ď��� </A>" \
	"| <A HREF=\"http://www.binboserver.com/\" CLASS=BigLine> BinboServer </A>" \
	"| <A HREF=\"http://www.maido3.com/server/subdomain.html\" CLASS=BigLine> Hikky.Zansu.com </A>" \
	"| <A HREF=\"http://news.kakiko.com/\" CLASS=BigLine> �����T�[�r�X </A>" \
	"|</FONT></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"</TABLE>" \
	"<CENTER>" \
	"<BR>" \
	"���ԃh���C���V���[�Y�o��<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>syo-ten</FONT></B>.com<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>gasuki</FONT></B>.com<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=Arial COLOR=#FF1493>zansu</FONT></B>.com<BR>" \
	"�������Ȗ��O�𖳗��Ŏg���܂��B�����ҏ����B<BR>" \
	"<FONT COLOR=#FF1493><B>�H�v�������ŁA�y����100�{�B</B></FONT>" \
	"<P><A HREF=\"http://www.maido3.com/server/\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X(n)" \
	"</A>" \
	"<BR>���̋���ȃT�C�g���g���Ă��܂��B" \
	"</CENTER><P>" \
	"</TD></TR></TABLE><P>"

/* �g���ĂȂ�...�͂��B */
#define	R2CH_HTML_BBSPINK_BANNER \
	"<CENTER>" \
	"<A TARGET=\"_blank\" HREF=\"http://www.pinknavi.com/?pinkid=PB0001&prg=3&act=0&txt=�����ł܂�&url=http://www.seiai.com/\">�����ł܂��A�_���g�T�C�g</A><br>" \
	"<br><a href=\"http://www.maido3.com/mentai/\"><b><font size=4>�[��ł����K�ȃ����^���T�[�o�T�[�r�X</font></b></a><br>" \
	"</CENTER>" \
	"<P>"

/* �g���ĂȂ�...�͂��B */
#define	R2CH_HTML_BANNER \
	"<CENTER>" \
	"<A HREF=\"http://www.maido3.com/server/\" TARGET=\"_blank\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X" \
	"</A>" \
	"<BR>���̋���ȃT�C�g���g���Ă��܂��B" \
	"</CENTER><P>"

/*
 * BROWSER�Ń��X��\���B
 */
/* ���X(mailto����): %d=���X�ԍ�  %s=mailto %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_MAIL \
	"<dt>%d ���O�F<a href=\"mailto:%s \"><b>%s </b></a> ���e���F%s<dd>%s<br><br>"

/* ���X(mailto����): %d=���X�ԍ� %s=���O %s=���e�� %s=�{��*/
#define R2CH_HTML_RES_NOMAIL \
	"<dt>%d ���O�F<font color=green><b>%s </b></font> ���e���F%s<dd>%s<br><br>"

/* ���X(�������Ă��܂�): %d=���X�ԍ� */
#define R2CH_HTML_RES_BROKEN_HERE \
	"<dt>%d ���O�F ���e���F[�������Ă��܂�]<DD>[�������Ă��܂�]<br><br>"

/* tail: %s=cgi %s=board %s=key %d=�J�n %d=�I�� %d=���X��
         %s=cgi %s=board %s=key %d=���X�� %d=���X��
         %02d=�����J�n���� %02d=�����I������ */
#define R2CH_HTML_TAIL \
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d\">���� %d ���X������</a>" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d\">�ŐV���X %d</a>\n" \
	" (%02d:00PM - %02d:00AM �̊Ԉ�C�ɑS���͓ǂ߂܂���)<BR>\n"


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
	" <a href=\"./%s?bbs=%s&key=%s&st=%d&to=%d&imode=true\">���� %d ���X������</a>\n" \
	" <a href=\"./%s?bbs=%s&key=%s&ls=%d&imode=true\">�ŐV���X %d</a><BR>\n"

/*
 * ERROR
 * ERROR1, ERROR2?, ERROR3, BANNER, ERROR4, (ERROR5_HTML|ERROR5_DAT|ERROR5_NONE)?, ERROR_6
 */

/* �G���[(�₢���킹��): */
#define R2CH_HTML_ERROR_ADMIN \
	"<A HREF=\"http://green.jbbs.net/computer/20/bigserver.html\">���₢���킹��</A>"

/* �G���[(1/6): %s=�G���[���b�Z�[�W %s=�G���[���b�Z�[�W %s=�G���[���b�Z�[�W %s=�R�����g */
#define R2CH_HTML_ERROR_1 \
	"<HTML><HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s</TITLE>" \
	"<style type=\"text/css\">" \
	"<!--\n" \
	"TD.Type1 {" \
	"color: #fff;" \
	"text-align: left;" \
	"}" \
	"A.BigLine {" \
	"color: #000;" \
	"text-decoration: none;" \
	"}\n" \
	"-->" \
	"</style>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=\"#00F\" alink=\"#F00\" vlink=\"#609\">" \
	"<P><FONT SIZE=+1 COLOR=\"#F00\">%s</FONT></B><BR>" \
	"<DL>" \
	"<dt>1 ���O�F<font color=green><b>" \
	CGIVER \
	"</b></font>���e���F2001/04/12(��) 15:11<dd>%s<br><br>%s<br>\n"

/* �G���[(2/6): ������[����̂݁B */
#define R2CH_HTML_ERROR_2 \
	"<dt>2 ���O�F<font color=green><b>" CGIVER "</b></font>���e���F2001/04/12(��) 15:11<dd>"\
	"<BR>" \
	"<P><A target=\"_blank\" HREF=\"http://www.maido3.com/server/\">" \
	"http://www.maido3.com/server/" \
	"</A><BR>" \
	"���ԃh���C���V���[�Y�o��<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>syo-ten</FONT></B>.com<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>gasuki</FONT></B>.com<BR>" \
	"�@�@�@*****.<B><FONT SIZE=+1 FACE=\"Arial\" COLOR=DEEPPINK>zansu</FONT></B>.com<BR>" \
	"�������Ȗ��O�𖳗��Ŏg���܂��B�����ҏ����B<BR>" \
	"<FONT COLOR=DEEPPINK><B>�H�v�������ŁA�y����100�{�B</B></FONT>" \
	"<P><A target=\"_blank\" HREF=\"http://www.maido3.com/server/\">" \
	"�[��ł����K�ȃ����^���T�[�o�T�[�r�X" \
	"</A>" \
	"<BR>���̋���ȃT�C�g���g���Ă��܂��B<BR><BR>" \
	"<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 WIDTH=100%%>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR BGCOLOR=#ccc><TD HEIGHT=20 NOWRAP><FONT SIZE=2>&nbsp;" \
	"| <A target=\"_blank\" HREF=\"http://www.maido3.com/\" CLASS=\"BigLine\"> �������� </A>" \
	"| <A target=\"_blank\" HREF=\"http://news.kakiko.com/2chwatch/\" CLASS=\"BigLine\"> 2ch�T�[�o�Ď��� </A>" \
	"| <A target=\"_blank\" HREF=\"http://www.binboserver.com/\" CLASS=\"BigLine\"> BinboServer </A>" \
	"| <A target=\"_blank\" HREF=\"http://www.maido3.com/server/subdomain.html\" CLASS=\"BigLine\"> Hikky.Zansu.com </A>" \
	"| <A target=\"_blank\" HREF=\"http://news.kakiko.com/\" CLASS=\"BigLine\"> �����T�[�r�X </A>" \
	"|</FONT></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#efefef><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"<TR><TD HEIGHT=1 BGCOLOR=#333><IMG HEIGHT=1 WIDTH=1 SRC=\"/space1.gif\"></TD></TR>" \
	"</TABLE>" \
	"<br><br>"

/* �G���[(3/6): */
#define R2CH_HTML_ERROR_3 \
	"</DL><CENTER>\n<BR>"

/* �G���[(4/6): */
#define R2CH_HTML_ERROR_4 \
	"<BR></CENTER><HR><FONT SIZE=-2>" CGIVER "</FONT><HR><P>"

/* �G���[(5a/6,HTML): %s=�ꏊ %s=�X���ԍ� */
#define R2CH_HTML_ERROR_5_HTML \
	"����! �ߋ����O�q�ɂŁA<A HREF=\"%s\">�X���b�h %s.html</A>�@�𔭌����܂����B\n"

/* �G���[(5b/6,DAT): %s=�ꏊ %s=�X���ԍ� */
#define R2CH_HTML_ERROR_5_DAT \
	"����! �ߋ����O�q�ɂŁA<A HREF=\"%s\">�X���b�h %s.dat</A>�@�𔭌����܂����B<BR><B><FONT SIZE=+1 COLOR=RED>��</FONT></B>�A�������܂� html������Ă��܂���B�c�O�A�҂����Ȃ��B\n"

/* �G���[(5c/6,NONE): %s=�� */
#define R2CH_HTML_ERROR_5_NONE \
	"<A HREF=\"/%s/kako/\">�ߋ����O�q��</A>�ɂ�����܂���ł����B<BR>�₢���킹�Ă�������\\���͂قƂ�ǂ���܂���B\n"

/* �G���[(6/6): */
#define R2CH_HTML_ERROR_6 \
	"</BODY></HTML>"

/* �G���[999: %s=�G���[���b�Z�[�W %s=board %s=key %s=ls %s=st %s=to %s=nofirst %d=file-size %d=max-line
              %s=time(hh:mm:ss) %s=board %s=soko %s=key %s=key */
/* �g���ĂȂ�...�͂��B */
#define R2CH_HTML_ERROR_999_1 \
	"<HTML><HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>ERROR</TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=#00F alink=#F00 vlink=#609>" \
	"<TABLE border=1 cellspacing=11 cellpadding=2 width=95%% bgcolor=#efefef align=center><TR><TD>" \
	"����[�A%s<P>" \
	"bbs = [%s]�@" \
	"key = [%s]�@" \
	"ls = [%s]�@" \
	"st = [%s]�@" \
	"to = [%s]�@" \
	"nofirst = [%s]<BR>" \
	"fileSize = [%d bytes] %dlines<BR>" \
	"time=[%s]<BR>" \
	"<BR>�u����[�A����Ȕ�or�X���b�h�Ȃ��ł��v�̏ꍇ�͂����q�ɂ� Go <BR>" \
	"<A HREF=\"/%s/kako/%s/%s.html\">�ߋ����O�q�� %s.html</A>\n" \
	"<BR>�܂��Adat -> html �ϊ����s���Ă��Ȃ��ꍇ���ǂ�����܂��B" \
	"<BR><A HREF=\"http://teri.2ch.net/test/read.cgi?bbs=accuse&key=980507715&ls=30\">����</A>�ői����ƁA�ߋ����O���������Ă����ꍇ�����邩���ł��B" \
	"</TD></TR></TABLE>" \
	"<BR><BR><BR>"

/* �g���ĂȂ�...�͂��B */
#define R2CH_HTML_ERROR_999_2 \
	"<BR><BR><BR>" \
	"<FONT SIZE=-2>" CGIVER "</FONT>" \
	"</BODY>" \
	"</HTML>"

/*
 * HEADER
 * (HEADER|IMODE_HEADER), (HEADER_REDZONE|HEADER_REDZONE|HEADER_REDZONE)? , HEADER_2
 */
/* �u���E�U�Ō����Ƃ�: %s=�X���� %s=�� %s=�� %s=�X���ԍ� %s=�� %s=�X���ԍ� */
#ifndef COOKIE
#define R2CH_HTML_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"<SCRIPT LANGUAGE=JavaScript>" \
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
	"</SCRIPT>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/index2.html\">���f���ɖ߂遡</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s\">���X��S���ǂ�</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=100\">�ŐV���X100</A>"
#else
#define R2CH_HTML_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/index2.html\">���f���ɖ߂遡</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s\">���X��S���ǂ�</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=100\">�ŐV���X100</A>"
#endif
	
/* i-Mode�Ō����Ƃ�: %s=�X���� %s=�� %s=�� %s=�X���ԍ� %d=��x�ɕ\�����郌�X��
                     %s=�� %s=�X���ԍ� %d=��x�ɕ\�����郌�X�� %d=��x�ɕ\�����郌�X�� */
#define R2CH_HTML_IMODE_HEADER_1 \
	"<HTML>" \
	"<HEAD>" \
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">" \
	"<TITLE>%s </TITLE>" \
	"</HEAD>" \
	"<BODY TEXT=black BGCOLOR=#efefef link=blue alink=red vlink=#609>" \
	"<A HREF=\"/%s/i/\">���f���ɖ߂遡</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&st=1&to=%d&imode=true\">���X���ŏ�����ǂ�</A>" \
	" <A HREF=\"" CGINAME "?bbs=%s&key=%s&ls=%d&imode=true\">�ŐV���X%d</A>"

/* ���X���I�[�o�[: %d=�ő僌�X�� */
#define R2CH_HTML_HEADER_RED \
	"<P><TABLE><TR><TD BGCOLOR=RED>" \
	"<FONT COLOR=#fff><BR><BR>���X���� %d �𒴂��Ă��܂��B�c�O�Ȃ���S���͕\\�����܂���B<BR><BR></FONT>" \
	"</TD></TR></TABLE>"

/* ���X����΂�: %d=��΂��Ȃ郌�X�� */
#define R2CH_HTML_HEADER_REDZONE \
	"<P><TABLE><TR><TD BGCOLOR=RED>\n" \
	"<FONT COLOR=#fff>���X���� %d �𒴂��Ă��܂��B%d�𒴂���ƕ\\���ł��Ȃ��Ȃ��B</FONT>" \
	"</TD></TR></TABLE>"

/* ���X�����낻��?: %d=�x���̏o�郌�X�� */
#define R2CH_HTML_HEADER_YELLOW \
	"<P><TABLE><TR><TD BGCOLOR=#ff0>" \
	"���X���� %d �𒴂��Ă��܂��B%d�𒴂���ƕ\\���ł��Ȃ��Ȃ��B" \
	"</TD></TR></TABLE>"

/* �X����: %s=�X���� */
#define R2CH_HTML_HEADER_2 \
	"<P><FONT SIZE=+1 COLOR=red>%s </FONT>"

/*
 * RELOAD
 */
#define R2CH_RELOAD \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d\">�V���X�̕\\��</a></center><hr>"

#define R2CH_RELOAD_I \
	"<hr><center><a href=\"read.cgi?bbs=%s&key=%s&st=%d&imode=true\">�V���X�̕\\��</a></center><hr>"

/*
 * FOOTER
 * (FORM? , FOOTER) | FORM_IMODE
 */
#ifndef COOKIE
/* %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FORM \
	"<form method=POST action=\"bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<SCRIPT Language=JavaScript>" \
	"<!--\n" \
	"document.write(\"<INPUT TYPE=text NAME=FROM SIZE=19 VALUE=\",N,\"> E-mail<font size=1> (�ȗ���) </font>: <INPUT TYPE=text NAME=mail SIZE=19 VALUE=\",M,\">\");\n" \
	"// -->" \
	"</SCRIPT>" \
	"<NOSCRIPT>" \
	"<INPUT TYPE=text NAME=FROM SIZE=19>" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<INPUT TYPE=text NAME=mail SIZE=19>" \
	"</NOSCRIPT>" \
	"<BR><textarea rows=5 cols=70 wrap=OFF name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#else
/* �u���E�U�̂Ƃ��̃t�b�^: %s=���e�Җ� %s=mail %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FORM \
	"<form method=POST action=\"bbs.cgi\">" \
	"<input type=submit value=\"��������\" name=submit> ���O�F " \
	"<INPUT TYPE=text NAME=FROM SIZE=19 VALUE=\"%s\">" \
	" E-mail<font size=1> (�ȗ���) </font>: " \
	"<INPUT TYPE=text NAME=mail SIZE=19 value=\"%s\">" \
	"<BR><textarea rows=5 cols=70 wrap=OFF name=MESSAGE></textarea>" \
	"<input type=hidden name=bbs value=%s>" \
	"<input type=hidden name=key value=%s>" \
	"<input type=hidden name=time value=%ld>" \
	"</form>"
#endif
	
#define R2CH_HTML_FOOTER \
	"</DL><P>" CGIVER "</BODY></HTML>"

/* i-MODE�̂Ƃ��̃t�b�^: %s=�� %s=�X�� %ld=���ݎ��� */
#define R2CH_HTML_FORM_IMODE \
	"<FORM METHOD=POST ACTION=\"./bbs.cgi\">\n" \
	"<INPUT TYPE=submit VALUE=\"��������\" name=submit> " \
	"NAME�F<INPUT TYPE=text NAME=FROM SIZE=20 VALUE=\"\">" \
	"MAIL�F<INPUT TYPE=text NAME=mail SIZE=20 VALUE=\"\">" \
	"<br>" \
	"<INPUT TYPE=hidden name=bbs VALUE=%s>" \
	"<INPUT TYPE=hidden name=key VALUE=%s>" \
	"<INPUT TYPE=hidden name=time VALUE=%ld>" \
	"<TEXTAREA ROWS=5 COLS=60 wrap=off NAME=MESSAGE></TEXTAREA>" \
	"</FORM><br>" \
	"<P>" CGIVER "</BODY></HTML>"
