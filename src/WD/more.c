/*-------------------------------------------------------*/
/* more.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : simple & beautiful ANSI/Chinese browser      */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

int beep = 0;
                                
char *
Ptt_prints(char* str,int mode)
{
  char *po , *px, strbuf[256];

  while (po = strstr(str, "\033[12"))
  {
    po[0] = 0;
  }
  while (po = strstr(str, "\033[10"))
  {
    po[0] = 0;
  }
  while (po = strstr(str, "\033n"))
  {
    po[0] = 0;
  }
  while (po = strstr(str, "\033]"))
  {
    po[0] = 0;
  }
  while (po = strstr(str, "\033[="))
  {
    po[0] = 0;
  }
  while (po = strstr(str, "\033*"))
  {
    switch(*(po+2))
    {
     case 'S' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%s", str, BOARDNAME);
        break;

     case 's' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%s", str, cuser.userid);
        break;

     case 't' :
       {
        time_t now = time(0);
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%s", str, Etime(&now));
        break;
       }

     case 'u' :
       {
        int attempts;
        extern struct UTMPFILE *utmpshm;
        attempts = utmpshm->number;
        *po = 0;
        px = po + 3;
        sprintf(strbuf,"%s%d", str, attempts);
        break;
       }

/*	ychia.0721 */
/* unmark by hialan.020722*/
     case 'z' :
       {
        *po = 0;
        px = po + 3;
//        sprintf(strbuf,"%s%d", str, count_ulist());
        sprintf(strbuf,"%s%d", str, guest_count_ulist()); //·|¦©°£©Ò¦³Áô¨­ªº¤H
        break;
       }
/**/     
     case 'b' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf,"%s%d/%d", str, cuser.month, cuser.day, px);
        break;

     case 'l' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%d", str, cuser.numlogins);
        break;

     case 'p' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%d",  str, cuser.numposts);
        break;

     case 'n' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%s", str, cuser.username);
        break;

     case 'm' :
        *po = 0;
        px = po + 3;
        sprintf(strbuf, "%s%d", str, cuser.silvermoney);
        break;

     default :
        *po = 0;
        px = NULL;
        break;
   }

   if (px)
   {
     strcat(strbuf, px);
     strcpy(str, strbuf);
   }

  }
  return str;
}

static int
readln(fp, buf)
  FILE *fp;
  char *buf;
{
  register int ch, i, len, bytes, in_ansi;

  len = bytes = in_ansi = i = 0;
  while (len < 80 && i < ANSILINELEN && (ch = getc(fp)) != EOF)
  {
    bytes++;
    if (ch == '\n')
    {
      break;
    }
    else if (ch == '\t')
    {
      do
      {
        buf[i++] = ' ';
      } while ((++len & 7) && len < 80);
    }
    else if (ch == '\a')
    {
      beep = 1;
    }
    else if (ch == '\033')
    {
      if (showansi)
        buf[i++] = ch;
      in_ansi = 1;
    }
    else if (in_ansi)
    {
      if (showansi)
        buf[i++] = ch;
      if (!strchr("[0123456789;,", ch))
        in_ansi = 0;
    }
    else if (isprint2(ch))
    {
      len++;
      buf[i++] = ch;
    }
  }
  buf[i] = '\0';
  return bytes;
}


int
more(fpath, promptend)
  char *fpath;
  int promptend;
{
  extern char* strcasestr();
  static char *head[4] = {" §@ªÌ ", " ¼ÐÃD ", " ®É¶¡ ", " Âà«H "};
  char *ptr, *word, buf[1024],*ch1;
  struct stat st;
  FILE *fp;
  usint pagebreak[MAXPAGES], pageno, lino;
  int line, ch, viewed, pos, numbytes;
  int header = 0;
  int local = 0;
  char search_char0=0;
  static char search_str[81]="";
  typedef char* (*FPTR)();
  static FPTR fptr;
  int searching = 0;
  int scrollup = 0;
//  int decode=0;
  char *http[80]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
                        /* Ptt */
  char pagemode = 0;
  char pagecount = 0;

  inmore = 1;
  memset(pagebreak, 0, sizeof(pagebreak));
  if (*search_str)
     search_char0 = *search_str;
  *search_str = 0;
  if (!(fp = fopen(fpath, "r")))
  {
    inmore = 0;
    return -1;
  }
    
  if (fstat(fileno(fp), &st))
  {
    inmore = 0;
    fclose(fp);
    return -1;
  }

  pagebreak[0] = pageno = viewed = line = pos = 0;
  clear();

  while ((numbytes = readln(fp, buf)) || (line == t_lines))
  {
    if (scrollup) {
       rscroll();
       move(0, 0);
    }
    if (numbytes)               /* ¤@¯ë¸ê®Æ³B²z */
    {
      if (!viewed)              /* begin of file */
      {
        if (showansi)           /* header processing */
        {
          if (!strncmp(buf, str_author1, LEN_AUTHOR1))
          {
            line = 3;
            word = buf + LEN_AUTHOR1;
            local = 1;
          }
          else if (!strncmp(buf, str_author2, LEN_AUTHOR2))
          {
            line = 4;
            word = buf + LEN_AUTHOR2;
          }

          while (pos < line)
          {
            if (!pos && ((ptr = strstr(word, str_post1)) ||
                (ptr = strstr(word, str_post2))))
            {
              ptr[-1] = '\0';
              
              /*hialan §ïÃC¦â*/
              prints("\033[m[34;47m%s\033[1m[37;44m%-53.53s\033[m[34;47m %.4s \033[m[37;44m%-13s[0m\n", head[0], word, ptr, ptr + 5);
            }
            else if (pos < (local ? 3 : 4))
            {
              if(!local && pos == 1) str_decode(word);
              prints("\033[m[34;47m%s\033[1m[37;44m%-72.72s[m\n", head[pos], word);
            }

            viewed += numbytes;
            numbytes = readln(fp, buf);

            if(!pos && viewed >= 79)            /* ²Ä¤@¦æ¤Óªø¤F */
            {
              if (memcmp( buf, head[1], 2))     /* ²Ä¤G¦æ¤£¬O [¼Ð....] */
              {
                viewed += numbytes;             /* Åª¤U¤@¦æ¶i¨Ó³B²z */
                numbytes = readln(fp, buf);
              }
            }

            pos++;
          }
          if (pos)
          {
            header = 1;

            prints("[0;36m%s[m\n", msg_seperator);
            line = pos = (local ? 4 : 5);
          }
        }
        lino = pos;
        word = NULL;
      }
//     if(strstr(buf,"This is a multi-part message in MIME format"))
//       decode = 1;
//     if(decode)
       str_decode(buf);

      /* ¡°³B²z¤Þ¥ÎªÌ & ¤Þ¨¥ */

      if ((buf[1] == ' ') && (buf[0] == ':' || buf[0] == '>'))
        word = "[36m";
      if ((buf[1] == ' ') && (buf[0] == ':' || buf[0] == '>') && (buf[2] == ':' || buf[2] == '>'))
        word = "[33m";
      if ((buf[0] == '¡' && buf[1] == '°') || !strncmp(buf, "==>", 3))
        word = "[1;36m";

      ch1 = buf ;
      while(1) /* Ptt */
        {
          int i;
          char e,*ch2;
          if(ch2 = strstr(ch1,"gopher://"));
          else if(ch2 = strstr(ch1,"mailto:"));
// ¤ä´©ª½±µ¶i¤J¬ÝªO
          else if(ch2 = strstr(ch1,"board://"));
          else break;
          for(e=0; ch2[e] != ' ' && ch2[e] != '\n' && ch2[e] != '\0'
           && ch2[e] != '"' && ch2[e] != ';' && ch2[e] != ']'; e++);
          for(i=0;http[i] && i<80;i++)
                if(!strncmp(http[i],ch2,e) && http[e]==0) break;
          if(!http[i])
                {
                 http[i] = (char *) malloc( e+1 );
                 strncpy(http[i],ch2,e);
                 http[i][e]=0;
                 pagecount++;
                }
          ch1 = &ch2[7];
        }

      if (word)
        outs(word);
      {
/*
woju
*/
         char msg[500], *pos;

        if (*search_str && (pos = fptr(buf, search_str))) {
           char SearchStr[41];
           char buf1[100], *pos1;

           strncpy(SearchStr, pos, strlen(search_str));
           SearchStr[strlen(search_str)] = 0;
           searching = 0;
           sprintf(msg, "%.*s[7m%s[0m", pos - buf, buf,
              SearchStr);
           while (pos = fptr(pos1 = pos + strlen(search_str), search_str)) {
              sprintf(buf1, "%.*s[7m%s[0m", pos - pos1, pos1, SearchStr);
              strcat(msg, buf1);
           }
           strcat(msg, pos1);
           outs(Ptt_prints(msg,NO_RELOAD));
        }
        else {
           outs(Ptt_prints(buf,NO_RELOAD));
        }
      }
      if (word) {
        outs("[0m");
        word = NULL;
      }
      outch('\n');

      if (beep)
      {
        bell();
        beep = 0;
      }

      if (line < b_lines)       /* ¤@¯ë¸ê®ÆÅª¨ú */
        line++;

      if (line == b_lines && searching == -1) {
        if (pageno > 0)
           fseek(fp, (off_t)(viewed = pagebreak[--pageno]), SEEK_SET);
        else
           searching = 0;
        lino = pos = line = 0;
        clear();
        continue;
      }

      if (scrollup) {
         move(line = b_lines, 0);
         clrtoeol();
         for (pos = 1; pos < b_lines; pos++)
            viewed += readln(fp, buf);
      }
      else if (pos == b_lines)  /* ±²°Ê¿Ã¹õ */
        scroll();
      else
        pos++;


      if (!scrollup && ++lino >= b_lines && pageno < MAXPAGES - 1)
      {
        pagebreak[++pageno] = viewed;
        lino = 1;
      }

      if (scrollup) {
         lino = scrollup;
         scrollup = 0;
      }
      viewed += numbytes;       /* ²Ö­pÅª¹L¸ê®Æ */
    }
    else
      line = b_lines;           /* end of END */


    if (promptend && (!searching && line == b_lines || viewed == st.st_size))
    {
      /* Kaede ­è¦n 100% ®É¤£°± */
/*
      if (viewed == st.st_size && viewed - numbytes == pagebreak[1])
        continue;
*/
      move(b_lines, 0);
      if (viewed == st.st_size) {
         if (searching == 1)
            searching = 0;
/*
woju
*/
      }
      else if (pageno == 1 && lino == 1) {
         if (searching == -1)
            searching = 0;
      }
      prints(COLOR2"  ÂsÄý P.%d(%d%%)  ", pageno,(viewed * 100) / st.st_size);

      prints(COLOR1" [1m[33m (^Z)[37m¨D§U \
[33m¡÷¡õ [PgUp][PgDn][Home][End][33m ¡ö(q)[37mµ²§ô[m");

      move(b_lines,0);   /*¸Ñ¨Mª½±µ¸õ¨ì¬ÝªOªºBug*/
      while (line == b_lines || (line > 0 && viewed == st.st_size))
      {
        switch (ch = igetkey())
        {
        case ':': {
           char buf[10];
           int i = 0;

           getdata(b_lines - 1, 0, "Goto Page: ", buf, 5, DOECHO,0);
           sscanf(buf, "%d", &i);
           if (0 < i && i <  MAXPAGES && (i == 1 || pagebreak[i - 1]))
              pageno = i - 1;
           else if (pageno)
              pageno--;
           lino = line = 0;
           break;
        }

        case '/': {
           char ans[4] = "n";
           *search_str = search_char0;
           getdata(b_lines - 1, 0,"[·j´M]ÃöÁä¦r:", search_str, 40, DOECHO,0);
           if (*search_str) {
              searching = 1;
              if (getdata(b_lines - 1, 0, "°Ï¤À¤j¤p¼g(Y/N/Q)? [N] ", ans, 4, LCECHO,0) && *ans == 'y')
                 fptr = strstr;
              else
                 fptr = strcasestr;
           }
           if (*ans == 'q')
              searching = 0;
           if (pageno)
              pageno--;
           lino = line = 0;
           break;
        }
        case 'n':
           if (*search_str) {
              searching = 1;
              if (pageno)
                 pageno--;
              lino = line = 0;
           }
           break;
        case 'N':
           if (*search_str) {
              searching = -1;
              if (pageno)
                 pageno--;
              lino = line = 0;
           }
           break;
        case 'r':
        case 'R':
        case 'Y':
           fclose(fp);
           inmore = 0;
           return 7;
        case 'y':
           fclose(fp);
           inmore = 0;
           return 8;
        case 'A':
           fclose(fp);
           inmore = 0;
           return 9;
        case 'a':
           fclose(fp);
           inmore = 0;
           return 10;
        case 'F':
           fclose(fp);
           inmore = 0;
           return 11;
        case 'B':
           fclose(fp);
           inmore = 0;
           return 12;
        case KEY_LEFT:
          fclose(fp);
          inmore = 0;
          return 6;
        case 'q':
          fclose(fp);
          inmore = 0;
          return 0;
        case 'b':
           fclose(fp);
           inmore = 0;
           return 1;
        case 'f':
           fclose(fp);
           inmore = 0;
           return 3;
        case ']':       /* Kaede ¬°¤F¥DÃD¾\Åª¤è«K */
           fclose(fp);
           inmore = 0;
           return 4;
        case '[':       /* Kaede ¬°¤F¥DÃD¾\Åª¤è«K */
           fclose(fp);
           inmore = 0;
           return 2;
        case '=':       /* Kaede ¬°¤F¥DÃD¾\Åª¤è«K */
           fclose(fp);
           inmore = 0;
           return 5;
        case Ctrl('F'):
        case KEY_PGDN:
          line = 1;
          break;
        case 't':
          if (viewed == st.st_size) {
             fclose(fp);
             inmore = 0;
             return 4;
          }
          line = 1;
          break;
        case ' ':
          if (viewed == st.st_size) {
             fclose(fp);
             inmore = 0;
             return 3;
          }
          line = 1;
          break;
        case KEY_RIGHT:
          if (viewed == st.st_size) {
             fclose(fp);
             inmore = 0;
             return 0;
          }
          line = 1;
          pagemode = 0;
          break;
        case '\r':
        case '\n':
          if (pagemode){
                 more_web(http[pagemode-1],YEA);
                 /*pagebreak[0] = pageno = viewed = line = pos = 0;*/
                pagemode = 0;
                *search_str = 0;
                 if (pageno)
                    pageno--;
                 lino = line = 0;
                 break;
                }

        case KEY_DOWN:
          if (viewed == st.st_size ||
              promptend == 2 && (ch == '\r' || ch == '\n')) {
             fclose(fp);
             inmore = 0;
             return 3;
          }
          line = t_lines - 2;
          break;

        case '$':
        case 'G':
        case KEY_END:
          line = t_lines;
          break;

        case '0':
        case 'g':
        case KEY_HOME:
          pageno = line = 0;
          break;

        case 'E':
          if (strcmp(fpath, "etc/ve.hlp")) {
             fclose(fp);
             inmore = 0;
             vedit(fpath, HAS_PERM(PERM_SYSOP) ? 0 : 2);
             return 0;
          }
          break;

        case KEY_ESC:
           if (KEY_ESC_arg == 'n') {
              edit_note();
              if (pageno)
                 pageno--;
              lino = line = 0;
           }
           else if (KEY_ESC_arg == 'c')
              capture_screen();
           break;

        case Ctrl('I'):
           if(!pagecount) break;
           pagemode = (pagemode % pagecount) + 1;
           strcpy(search_str,http[pagemode-1]);
           fptr = strstr;
           if (pageno)
              pageno--;
           lino = line = 0;
           break;

        case KEY_UP:
           line = -1;
           break;

        case Ctrl('B'):
        case KEY_PGUP:
          if (pageno > 1)
          {
            if (lino < 2)
               pageno -= 2;
            else
               pageno--;
            lino = line = 0;
          }
          else if (pageno && lino > 1)
            pageno = line = 0;
          break;
        case Ctrl('H'):
          if (pageno > 1)
          {
            if (lino < 2)
               pageno -= 2;
            else
               pageno--;
            lino = line = 0;
          }
          else if (pageno && lino > 1)
            pageno = line = 0;
/*
woju
*/
          else {
             fclose(fp);
             inmore = 0;
             return 1;
          }
        }
      }

      if (line > 0)
      {
        move(b_lines, 0);
        clrtoeol();
        refresh();
      }
      else if (line < 0) {                      /* Line scroll up */
         if (pageno <= 1) {
            if (lino == 1 || !pageno) {
               fclose(fp);
               inmore = 0;
               return 1;
            }
            if (header && lino <= 5) {
               fseek(fp, (off_t)(viewed = pagebreak[scrollup = lino = pageno = 0] = 0), SEEK_SET);
               clear();
            }
         }
         if (pageno && lino > 1 + local) {
            line =  (lino - 2) - local;
            if (pageno > 1 && viewed == st.st_size)
               line += local;
            scrollup = lino - 1;
            fseek(fp, (off_t)(viewed = pagebreak[pageno - 1]), SEEK_SET);
            while (line--)
               viewed += readln(fp, buf);
         }
         else if (pageno > 1) {
            scrollup = b_lines - 1;
            line = (b_lines - 2) - local;
            fseek(fp, (off_t)(viewed = pagebreak[--pageno - 1]), SEEK_SET);
            while (line--)
               viewed += readln(fp, buf);
         }
         line = pos = 0;
      }
      else
      {
        pos = 0;
        fseek(fp, (off_t)(viewed = pagebreak[pageno]), SEEK_SET);
        clear();
      }
    }
  }

  fclose(fp);
  if (promptend)
  {
    pressanykey(NULL);
    clear();
  }
  else
    outs(reset_color);
    
  inmore = 0;
  return 0;
}


int
more_web(fpath, promptend)
  char *fpath;
  int promptend;
{
  char *ch;
  char genbuf[41]; 

  if(ch=strstr(fpath,"mailto:"))
    {
        if(!HAS_PERM(PERM_LOGINOK))
           {
             move(b_lines - 1,0);
             outs("[41m ±zªºÅv­­¤£¨¬µLªk¨Ï¥Îinternet mail... [m");
             refresh();
             return 0;
           }
        if(!not_addr(&ch[7]) &&
               getdata(b_lines - 1, 0, "[±H«H]¥DÃD¡G", genbuf, 40, DOECHO,0))
           {
                  do_send(&ch[7], genbuf);
           }
        else
           {
             move(b_lines - 1,0);
             outs("[41m ¦¬«H¤Hemail ©Î ¼ÐÃD ¦³»~... [m");
             refresh();
           }
        return 0;
    }
#if 0
  if(ch=strstr(fpath,"gopher://"))
    {
          ITEM item;
          strcpy(item.X.G.server, &ch[9]);
          strcpy(item.X.G.path, "1/");
          item.X.G.port = 70;
          gem(fpath , &item, 0);
        return 0;
    }
#endif
// wildcat : ¤ä´©ª½±µ¶i¤J¬ÝªO 
  if(ch=strstr(fpath,"board://"))
  {
    char bname[20],bpath[60], oldch[STRLEN];
    struct stat st;
    int mode0 = currutmp->mode;
    int stat0 = currstat;
    int pos;
    boardheader *bhdr,*getbcache();

    strcpy(oldch, ch);
    strcpy(bname, strtok(oldch + 8, "#"));
    setbpath(bpath,bname);   
    if ((*bname == '\0') || (stat (bpath, &st) == -1))
    {
      pressanykey (err_bid);
      return RC_FULL;
    }

    if(bhdr = getbcache(bname))
    {
      if(Ben_Perm(bhdr) != 1)
      {
        pressanykey("§A¨S¦³¶i¤J¸ÓªOªºÅv­­");
        return 0;
      }
    }
    else
    {
      pressanykey("§A¨S¦³¶i¤J¸ÓªOªºÅv­­");
      return 0;
    }
/*
    setbfile (buf, bname, FN_LIST);
    
    if ((currbrdattr & BRD_HIDE) && !belong_list (buf, cuser.userid))
    {
      pressanykey(P_BOARD);
      return 0;
    }
*/
/* shakalaca.000123: ¤ä´©¬Ý¬Y¤@½g */    
    if (ch = strstr(fpath, "#"))
    {
      fileheader fhdr;
      
      pos = atoi(ch + 1);
      setbdir(bpath, bname);
      rec_get(bpath, &fhdr, sizeof(fileheader), pos);
      setbfile(bpath, bname, fhdr.filename);
      more(bpath);
    }
    else
    {
    /* shakalaca.000124: ¸Ñ¨M "¥¼Åª" °ÝÃD.. */
      brc_initial (bname);
      Read();  
    }

    currutmp->mode = mode0;
    currstat = stat0;
    return 0;
  }
}
