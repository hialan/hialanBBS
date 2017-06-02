/*

             bbs2html  v1.0        by CCCA.NCTU.edu.tw
		       v2.0        by PTT.m8.ntu.edu.tw	 
    §Ú¼g¤F¤@­Ó¥i¥H§â BBS ºëµØ°Ïªº¤º®e (~bbs/0Announce) Âà¦¨ html ªºµ{¦¡,
  ·|§â BBS ºëµØ°Ïªº¤å³¹§@¥H¤Uªº±Æª©:

    1. ¥[ background, title, icons
    2. §â ANSI color code Âà¦¨ html <font color=???>
    3. §â ANSI °{Ã{±±¨î½XÂà¦¨ <blink>..
    4. §â¤å¤¤©Ò¦³§äªº¨ìªº URL Âà¦¨ hyper link..


    SOURCE code ¸Ì¦³¤@¨ÇÅÜ¼Æ¥i¥H¤è«K¤j®a­×§ï:

	WWW_TITLE      title «e­±ªº mark
	WWW_BACKGROUND ­I´º
	WWW_HEAD_ICON  head ¤Wªº icon
	WWW_COLOR      page ªºÃC¦â
	WWW_END        WWW ©³³¡¥[ªº link..

--
Webadmin of Ptt Server
Consultant of Campus Computer Communication Association
Rm 101, Computer Center, National Chiao Tung University, Hsinchu, Taiwan
TEL : (035)712121 Ext 24388

*/
#include "bbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAX_WORDS 1024

int ansi_on, ansi_blink;

char WWW_TITLE[]="[¤p¯¸BBS]";
char WWW_BACKGROUND[]="/GIF/pttpad.jpg";
char WWW_HEAD_ICON[]="/GIF/green.gif";
char WWW_COLOR[]="bgcolor=#000000 text=#ddffdd vlink=#ffff00 link=#ffff11";
char WWW_END[]=
"<hr><CENTER><TABLE BORDER=5 cellspacing=10><TR>"
"<TD><a href=\"/ind.html\">¤p¯¸¥Dµe­±</a></TD>"
"<TD><a href=\"/~bbs\">bbs¥Dµe­±</a></TD>"
"<TD><a href=\"/message/message.html\">¯d¤U±zªº·N¨£</a></TD>"
"</TR></TABLE></CENTER></BODY></HTML>";
void cut_cr( char *l )
{
   char *i;

   for( i=l; *i!=0; i++ );
   if( *(i-1) != 0 ) *(i-1)=0;
}

void add_href( char *l )
{
   char tag[MAX_WORDS], url[MAX_WORDS];
   char *ap, *tp, *u;
   int found;

   ap=l; tp=tag;

   while( *ap!=0 )
   {
      found=0;

      strncpy( url, ap, 7 ); url[7]=0;
      if( !strcmp(url,"http://") || !strcmp(url,"HTTP://") ) found=1;

      strncpy( url, ap, 6 ); url[6]=0;
      if( !strcmp(url,"ftp://") || !strcmp(url,"FTP://") ) found=1;

      strncpy( url, ap, 7 ); url[7]=0;
      if( !strcmp(url,"file://") || !strcmp(url,"FILE://") ) found=1;

      strncpy( url, ap, 9 ); url[9]=0;
      if( !strcmp(url,"gopher://") || !strcmp(url,"GOPHER://") ) found=1;

      strncpy( url, ap, 9 ); url[9]=0;
      if( !strcmp(url,"telnet://") || !strcmp(url,"TELNET://") ) found=1;

      strncpy( url, ap, 7 ); url[7]=0;
      if( !strcmp(url,"mailto:") || !strcmp(url,"MAILTO:") ) found=1;

      if( found )
      {
	 for( u=url; *ap!=0; *u++=*ap++ )
	 {
	    if( *ap==' ' ) break;
	    if( *ap=='\"' ) break;
            if( *ap=='&' ) break;
	    if( (unsigned char)*ap>=127 ) break;
	 }
	 *u=0;

	 sprintf( tp, "<A HREF=\"%s\">%s</A>", url, url );
	 tp+=(strlen(tp));
      }
      else /* Not URL */
      {
	 *tp++=*ap++;
      }
   }

   *tp = 0;
   strcpy( l, tag );
}


void tag8859_1( char **tag, char c )
{
   switch( c )
   {
   case '\"': strcpy( *tag, "&quot" );   *tag+=5;break;
   case '<':  strcpy( *tag, "&lt" );     *tag+=3;break;
   case '>':  strcpy( *tag, "&gt" );     *tag+=3;break;
   case '&':  strcpy( *tag, "&amp" );    *tag+=4;break;

   default:
      **tag=c; (*tag)++;
   }
}


void tagcolor( char **tag, int attr )
{
   switch( attr ) /* === filter no-used ansi control === */
   {
   case 0: case 5:
   case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
      break;

   default:
      return;
   }


   if( attr==5 )
   {
      if( ansi_blink==0 )
      { ansi_blink=1; strcpy( *tag, "<blink>" ); *tag+=7; }
      return;
   }

   if( ansi_blink ) { strcpy( *tag, "</blink>" ); *tag+=8; }

   if( ansi_on )
   {
      strcpy( *tag, "</FONT>" ); *tag+=7;
      ansi_on=0;
   }


   switch( attr )
   {
   case 0:  ansi_blink=0; return;

   case 30: strcpy( *tag, "<FONT color=gray>" );   *tag+=17;break;
   case 31: strcpy( *tag, "<FONT color=red>" );    *tag+=16;break;
   case 32: strcpy( *tag, "<FONT color=green>" );  *tag+=18;break;
   case 33: strcpy( *tag, "<FONT color=yellow>" ); *tag+=19;break;
   case 34: strcpy( *tag, "<FONT color=blue>" );   *tag+=17;break;
   case 35: strcpy( *tag, "<FONT color=fuchsia>" );*tag+=20;break;
   case 36: strcpy( *tag, "<FONT color=aqua>" );   *tag+=17;break;
   case 37: strcpy( *tag, "<FONT color=white>" );  *tag+=18;break;

   default:
      if( ansi_blink ) { strcpy( *tag, "<blink>" ); *tag+=7; }
      return;
   }

   if( ansi_blink ) { strcpy( *tag, "<blink>" ); *tag+=7; }
   ansi_on=1;
}

int getansi( char **ap, int *attr, char *cmd )
{
   char cattr[100], *cap;

   cap = cattr; *cap=0;
   if( **ap == 0 ) return EOF;

   while( **ap>='0' && **ap<='9' )
   {
      *cap=**ap; cap++; (*ap)++;
   }
   if( cap==cattr ) return 0;

   *cap=0;
   sscanf( cattr, "%d", attr );
   if( **ap == 0 ) return 1;

   *cmd=**ap; (*ap)++;
   if( **ap == 0 ) return 2;
   return 3;
}

void ansi2tag( char *l )
{
   char tag[MAX_WORDS], esc[3];
   char *ap, *tp, cmd;
   int  attr, num, nextansi;

   esc[2]=0; nextansi=0;
   ap=l; tp=tag;
   while( *ap!=0 )
   {
      esc[0]=ap[0];esc[1]=ap[1];
      if( !strcmp(esc,"[") || nextansi )
      {
	 if( nextansi ) nextansi=0; else ap+=2;
	 num=getansi( &ap, &attr, &cmd );
	 if( num==EOF )
	 {
	    /* end-of-string */
	    break;
	 }
	 else if( num==0 )
	 {
	    /* ANSI+noint? eat the word */
	    ap++;
	 }
	 else if( num==1 )
	 {
	    /* ANSI+int+EOL go next line */
	    break;
	 }
	 else if( num==2 )
	 {
	    /* ANSI+attr+cmd+EOL, set color and go next line */
	    if( cmd=='m') tagcolor( &tp, attr );
	    break;
	 }
	 else if( num==3 )
	 {
	    /* ANSI+attr+cmd+str, set color.. */
	    tagcolor( &tp, attr );
	    if( cmd==';') nextansi=1;
	 }
      }
      else /* Not ANSI cntrol */
      {
	 tag8859_1( &tp, *ap ); ap++;
      }
   }
   *tp = 0;
   strcpy( l, tag );
}

int ann2html( char *bbsfile, char *htmlfile, char *tit )
{
   char l1[MAX_WORDS], title[MAX_WORDS];
   FILE *fi, *fo;
   struct stat st;
   time_t htmltime;

   if( (fi=fopen( bbsfile,"rt")) == NULL )
   {
      printf( "ann2html: No input file: %s\n", bbsfile );
      return 1;
   }
   lstat( bbsfile, &st );

   if( fo=fopen( htmlfile,"rt") )
   {
      /* === check for update === */
      if( fgets( l1, MAX_WORDS, fo) != NULL )
      {
	 if( sscanf(l1,"<!-- BBS2HTML[%lu]",&htmltime)==1 )
	    if( htmltime == st.st_mtime )
	    {
               printf( "ann2html: no need to update: %s\n");
	       fclose( fi ); fclose( fo );
	       return 2;
	    }
      }
      fclose(fo);
   }

   printf("file %s -> %s\n", bbsfile, htmlfile);

   if( (fo=fopen( htmlfile,"wt")) == NULL )
   {
      printf( "ann2html: Can't write html file: %s\n", htmlfile );
      fclose( fi );
      return 1;
   }

   ansi_on = 0;
   ansi_blink = 0;
   strcpy( title, tit ); ansi2tag( title );

   /* ========== html headers ============= */
   fprintf( fo,
"<!-- BBS2HTML[%lu] by ptt@ptt.m8.ntu.edu.tw  -->\n"
"<HTML>"
"<HEAD>"
"<TITLE>%s %s</TITLE>"
"</HEAD>"
"\n"
"<BODY background=\"%s\" %s"
">\n"
"<center>"
"<H2><IMG ALT=\"\" SRC=\"%s\">"
"%s</H2>\n</center>\n<pre><hr>"
	   , st.st_mtime, WWW_TITLE, title, WWW_BACKGROUND, 
             WWW_COLOR, WWW_HEAD_ICON, title );

   fprintf(fo,"<H4>\n");
   /* ========== text body ============= */
   while( fgets( l1, MAX_WORDS, fi) != NULL )
   {
      cut_cr( l1 );
      ansi2tag( l1 );
      add_href( l1 );
      if( !strcmp( l1, "--" ) )
	 fprintf( fo, "<hr>" );
      else
	 fprintf( fo, "%s\n", l1 );
   }
   fprintf(fo,"</H4>");
   /* ========== end html ============= */
   if( ansi_blink ) fprintf( fo, "</blink>" );
   if( ansi_on ) fprintf( fo, "</FONT>" );
   fprintf( fo, "</pre>%s", WWW_END );

   fclose( fi ); fclose( fo );
   return 0;
}

int isdir( char *fname )
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

void idxconvert( FILE *fi, FILE *fo )
{
   char l1[MAX_WORDS], l2[MAX_WORDS], oldname[MAX_WORDS], *p;
   fileheader item;
   /* ========== text body ============= */
/* Ptt */
   fprintf(fo,"<H4><ol>\n");

   while((fread( &item, sizeof(item), 1, fi))>0)
   {
         strcpy(l1,item.title);
/*	 cut_cr( l1 );*/
	 ansi2tag( l1 );
/*	 l1[4]=0;*/

         strcpy(l2,item.filename);
/*	 cut_cr( l2 );*/
/*	 l2[4]=0;*/

      strcpy( oldname, l2 );

      /* === Replace . in the Pathname === */
      for( p=l2; *p!=0; p++ ) if( *p=='.' ) *p='_';

      if( isdir(oldname) )
      {
	 fprintf( fo,
"<li>[%-5s] <a href=\"%s\">%s</a> (%s)<br>\n"
		   , item.date,l2,l1,item.owner );
      }
      else
      {
	 fprintf( fo,
"<li>[%-5s] <a href=\"%s.html\">%s</a> (%s)<br>\n"
		    ,item.date,l2,l1 ,item.owner);
      }
   }
 fprintf(fo,"</ol></H4>\n");
}


int idx2html( char *bbsfile, char *htmlfile, char *tit )
{
   char l1[MAX_WORDS], title[MAX_WORDS];
   FILE *fi, *fo;
   struct stat st;
   time_t htmltime;

   if( (fi=fopen( bbsfile,"rt")) == NULL )
   {
      printf( "idx2html: no input file: %s\n", bbsfile );
      return 1;
   }
   lstat( bbsfile, &st );

   if( fo=fopen( htmlfile,"rt") )
   {
      /* === check for update === */
      if( fgets( l1, MAX_WORDS, fo) != NULL )
      {
	 if( sscanf(l1,"<!-- BBS2HTML[%lu]",&htmltime)==1 )
	    if( htmltime == st.st_mtime )
	    {
	       fclose( fi ); fclose( fo );
	       return 2;
	    }
      }
      fclose(fo);
   }

   printf("Index %s -> %s\n", bbsfile, htmlfile);

   if( (fo=fopen( htmlfile,"wt")) == NULL )
   {
      printf( "idx2html: Can't write html file: %s\n", htmlfile );
      fclose( fi );
      return 1;
   }

   ansi_on = 0;
   ansi_blink = 0;
   strcpy( title, tit ); ansi2tag( title );

   /* ========== html headers ============= */
   fprintf( fo,
"<!-- BBS2HTML[%lu] Index by ptt@ptt.m8.ntu.edu.tw  -->\n"
"<HTML>\n"
"<HEAD>\n"
"<TITLE>%s %s</TITLE>\n"
"</HEAD>\n"
"\n"
"<BODY background=\"%s\" %s"
">\n\n"
"<center>\n"
"<H2><IMG ALT=\"\" SRC=\"%s\">\n"
"%s</H2>\n</center>\n<hr>"
	   , st.st_mtime, WWW_TITLE, title, WWW_BACKGROUND, 
             WWW_COLOR, WWW_HEAD_ICON, title );

   /* ========== text body ============= */
   idxconvert( fi, fo );

   /* ========== end html ============= */
   if( ansi_blink ) fprintf( fo, "</blink>" );
   if( ansi_on ) fprintf( fo, "</FONT>" );
   fprintf( fo, "%s", WWW_END );

   fclose( fi ); fclose( fo );
   return 0;
}

int bbs2html( char *sdir, char *hdir, char *title )
{
   char l1[MAX_WORDS], l2[MAX_WORDS], *p;
   char spath[MAX_WORDS], hpath[MAX_WORDS];
   FILE *fi;
   fileheader item;

   strcpy( spath, sdir );
   chdir( spath );
   strcat( spath, "/.DIR" );
   if( (fi=fopen( spath,"rt")) == NULL )
   {
      printf( "bbs2html: No file: %s\n", spath );
      return 1;
   }

   strcpy( hpath, hdir );
   strcat( hpath, "/index.html" );
   idx2html( spath, hpath, title );

   /* ========== text body ============= */

/* Ptt */

   while((fread( &item, sizeof(item), 1, fi))>0)
   {      
      strcpy(l1,item.title);
/*      cut_cr( l1 );*/
/*      l1[4]=0;*/

      strcpy(l2,item.filename);
/*      cut_cr( l2 );*/
/*      l2[4]=0;*/
      strcpy( spath, sdir );
      strcat( spath, "/" );
      strcat( spath, l2 );

      /* === Replace . in the Pathname === */

      for( p=l2; *p!=0; p++ ) if( *p=='.' ) *p='_';

      strcpy( hpath, hdir );
      strcat( hpath, "/" );
      strcat( hpath, l2 );

      if( isdir(spath) )
      {
	 /* === Entry is a directory === */
	 mkdir( hpath, S_IRWXU+S_IRGRP+S_IXGRP+S_IROTH+S_IXOTH );
	 bbs2html( spath, hpath, l1 );
      }
      else
      {
	 /* === Entry is a file === */
	 strcat( hpath, ".html" );
	 ann2html( spath, hpath, l1 );
      }
   }

   return 0;
}


int main( int argc, char *argv[])
{
   if( argc <= 3 )
   {
      printf( "usage: bbs2html [0Announce dir] [output html dir] [title]\n" );
      exit(1);
   }

   bbs2html( argv[1], argv[2], argv[3] );
}

