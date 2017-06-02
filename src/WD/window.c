/*-------------------------------------------------------*/
/* window.c     ( Athenaeum BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : window form                                  */
/* create : 2002/08/24                                   */
/* update : 2002/08/24                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

#if 0
show_winline(x, y, 視窗長度/2, 字串, 背景顏色, 光棒顏色);
show_winbox(直,寬,標題,提示字串,顯示模式);
msgbox(直,寬,標題,提示字串,顯示模式);
win_select(標題,提示字串,選項,選項數,預設字元);

EX:  win_select("加密文章", "是否編輯可看見名單? ", 0, 2, 'n')
#endif

typedef struct Win_form
{
  char title[5];	//標題顏色
  char body[5];		//內文顏色
  char border[8][3];	//外框
} Win_form;

static char win_load=1;		//是否已讀取? 1:未讀取 0:已讀取
static Win_form winform;


/*------------------------------*/
/*  使用者設定檔案存取          */
/*------------------------------*/
static int load_winform(Win_form *woutput)
{
  char fpath[PATHLEN];

  sethomefile(fpath, cuser.userid, FN_WINFORM);
  
  if(rec_get(fpath, woutput, sizeof(Win_form), 1) == -1 || !currutmp)
  {
    woutput->title[0]=4;
    woutput->title[1]=7;
    woutput->title[2]=1;
    woutput->title[3]=0;
    woutput->title[4]=0;
    
    woutput->body[0]=7;
    woutput->body[1]=0;
    woutput->body[2]=0;
    woutput->body[3]=0;
    woutput->body[4]=0;
    
    strcpy(woutput->border[0], "─");
    strcpy(woutput->border[1], "╭");
    strcpy(woutput->border[2], "╮");
    strcpy(woutput->border[3], "├");
    strcpy(woutput->border[4], "┤");    
    strcpy(woutput->border[5], "╰");
    strcpy(woutput->border[6], "╯");
    strcpy(woutput->border[7], "│");
  }
  if(currutmp && (woutput == &winform))
    win_load=0;	//設成已讀取
    
  return 1;
}

static int save_winform(Win_form *winput)
{
  char fpath[PATHLEN];
  int fd;
  
  sethomefile(fpath, cuser.userid, FN_WINFORM);
  
  if((fd = open(fpath, O_WRONLY | O_CREAT, 0644)) == -1)
    return -1;
  else
  {
    write(fd, winput, sizeof(Win_form));
    close(fd);
    win_load=1;	//設成未讀取
  }
  return 1;
}

/*------------------------------*/
/*  視窗產生                    */
/*------------------------------*/
static void 
show_winline(int x, int y, int win_len, char *words, 
		char *bgcolor, char *barcolor, Win_form *form)
{
  char buf[128];

  sprintf(buf, " %s%s %s %-*s\033[m%s ",
  	  form->border[7],
          (barcolor != 0) ? barcolor : bgcolor, 
          (barcolor != 0) ? "→" : "  ", 
          2*(win_len-4), words,
          form->border[7]);
                                                                                
  move(x,y);
  clrtoeol();
  outs(buf);
  move(b_lines, 0);
}

static int
show_winbox(int x, int y, int line, int width, char *title, char *prompt)
{
  int win_len;  /*win_len 是有幾個二位元字!!*/
  int i,j;
  char buf[256], bgcolor[40], title_color[40];

  //check
  if(win_load) load_winform(&winform);
  if(!line || line < 0) line = 1;
  
  if(width%2)
    win_len = (width / 2) + 1;
  else
    win_len = width / 2;

  get_color(bgcolor, winform.body);
  get_color(title_color, winform.title);  

  for(i = 0;i <= line+3;i++)
  {
    move(x + i, 0);
    clrtoeol();
    prints("%80s","");
  }
  /*上部分*/  

  sprintf(buf, " %s" ,winform.border[1]);

  j = win_len-1;

  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[2]);

  move(x,y);
  clrtoeol();
  outs(buf);
  
  /*標題*/
  show_winline(x+1, y, win_len, title, title_color, 0, &winform);  

  /*標題下橫槓*/
  sprintf(buf, " %s",winform.border[3]);
  j = win_len -1;                                                                                
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[4]);
                                                                                
  move(x+2,y);
  clrtoeol();
  outs(buf);
  
  /*提示*/
    show_winline(x+3, y, win_len, prompt, bgcolor, 0, &winform);

  /*我的屁股*/
  sprintf(buf," %s", winform.border[5]);
  j = win_len -1;
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[6]);
                                                                                
  move(x + 3 + line,y);
  clrtoeol();
  outs(buf);

  return win_len;
}

/*------------------------------*/
/*  視窗應用                    */
/*------------------------------*/
int msgbox(int line, int width, char *title, char *prompt)
{
  int x,y, win_len;

  /*init window*/
  x = (b_lines - line - 5) / 2;
  y = (80 - width) / 2;

  win_len = show_winbox(x, y, line, width, title, prompt);
}

int
win_select(char *title, char *prompt, char **choose, int many, char def)
{
  int x, y, i;
  int win_len, ch;
  int width;
  char *p, leave=-1;
  char barcolor[50], bgcolor[40];
  
  if(!choose)
    choose = msg_choose;

  /*init window*/
  width = strlen(title);
  i = strlen(prompt);
  if(i > width) width = i;
  for(i = 0;i < many;i++) /*ch暫時當作暫存變數..:pp*/
  {
    ch = strlen(choose[i]);
    if(ch > width) width = ch;
  }
    
  width = width + 12;
  x = (b_lines - many - 6) / 2;
  y = (80 - width) / 2;

  get_lightbar_color(barcolor);
  win_len = show_winbox(x, y, many+1, width, title, prompt);
  get_color(bgcolor, winform.body);

  for(i = 0;i < many;i++)
  {
    p = choose[i];
    if(def == *p) def = i;
    if(p == msg_choose_cancel) leave=i;
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, 0, &winform);
  }

  i = def;
  
  do
  {
    p = choose[i];
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, barcolor, &winform);
    ch = igetkey();
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, 0, &winform);
    
    switch(ch)
    {
      case KEY_UP:
        i--;
        if(i < 0) i = many -1;
        break;

      case KEY_DOWN:
        i++;
        if(i >= many) i = 0;
        break;
        
      case KEY_RIGHT:
        ch = '\r';
        break;
      
      case KEY_LEFT:
        if(leave<0) 
          i=def;
        else
          i=leave;
        break;

      case KEY_PGUP:
      case KEY_HOME:
        i=0;
        break;
        
      case KEY_PGDN:
      case KEY_END:
        i=many-1;
        break;
      
      default:
      {
        int j;

        ch = word_bigsmall(ch);
        for(j = 0;j < many;j++)
          if(ch == word_bigsmall(*(choose[j])))
          {
            i = j;
            break;
          }
        break;
      }
    }
  }while(ch != '\r');
  
  return *p;
}  

/*------------------------------*/
/*  使用者設定視窗外貌          */
/*------------------------------*/
int win_formchange()
{
  Win_form preview;
  int x=0,y=0, i, j,redraw, ch;
  const int winlen = 20;
  const int top=8;
  char buf[40], lightbar[40], *tmp;
  char *choose[2][5]={{"標題顏色", "直棒", "上左", "中左", "下左"}, 
      		      {"背景顏色", "橫棒", "上右", "中右", "下右"}};
  		 
  load_winform(&preview);
  get_lightbar_color(lightbar);

  redraw = 1;
  while(1)
  {
    if(redraw)//重畫 window preview
    {
      clear();
      sprintf(buf, "%s [線上 %d 人]", BOARDNAME, count_ulist());
      showtitle("顏色設定", buf);  
      
      move(b_lines-4, 0);
      prints("\033[36m%s\033[m", msg_seperator);
      
      move(b_lines, 0);
      clrtoeol();
      prints("\033[1;33;44m  樣式選擇  \033[1;46m%-68.68s\033[m",
             " [↑↓←→]選擇想要修改的項目  [q]離開");
      redraw = top;
      move(redraw++, 20);
      clrtoeol();
      outs(preview.border[1]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[2]);
      
      get_color(buf, preview.title);
      show_winline(redraw++, 19, winlen, "這是標題", buf, 0, &preview);
      
      move(redraw++,20);
      clrtoeol();
      outs(preview.border[3]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[4]);      

      get_color(buf, preview.body);
      show_winline(redraw++, 19, winlen, "這是內文", buf, 0, &preview);
      
      move(redraw++,20);
      clrtoeol();
      outs(preview.border[5]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[6]);      

      redraw=0;
    }
    
    for(i=0;i<2;i++)
    {
      move(b_lines - 3 + i, 0);
      clrtoeol();
      for(j=0;j<5;j++)
      {
        if(x==j && y==i)
          prints("[%s%s\033[m]", lightbar, choose[i][j]);
        else
          prints(" %s\033[m ", choose[i][j]);
      }
    }
    
    ch = igetkey();
    switch(ch)
    {
      case KEY_UP:
        y--;
        if(y<0) y=1;
        break;
      case KEY_DOWN:
        y++;
        if(y>1) y=0;
        break;
      case KEY_LEFT:
        x--;
        if(x<0) x=4;
        break;
      case KEY_RIGHT:
        x++;
        if(x>4) x=0;
        break;
      case '\n':
      case '\r':
      {
        switch(x)
        {
          case 0:
            if(!y)
              tmp=preview.title;
            else
              tmp=preview.body;

            for(i=0;i<5;i++)
                buf[i]=tmp[i];
              
            if(color_selector(buf) == 1)
              for(i=0;i<5;i++)
                tmp[i]=buf[i];
            break;
          case 1:
            if(!y)
              tmp=preview.border[7];
            else
              tmp=preview.border[0];
            break;
          case 2:
          case 3:
          case 4:
            i=2*(x-1);          
            if(!y)
              tmp=preview.border[i-1];
            else
              tmp=preview.border[i];
            break;
        }
        
        if(x)
        {
          getdata(b_lines-1, 0, "請輸入新的框:", buf, 3, DOECHO, tmp);
          if(strlen(buf) == 2)
            strcpy(tmp, buf);
        }
        redraw=1;
        break;
      }
      case 'q':
        i = getans2(b_lines-1, 0,"請問是否要存檔離開? ", 0, 3, 'q');
        if(i=='y')
        {
          if (save_winform(&preview) == 1)
          {
            pressanykey("修改成功\!!");
            return 1;
          }
          else
          {
            pressanykey("修改失敗 >_<");
            return -1;
          }
        }
        else if(i=='n')
          return -1;
        
        redraw=1;
        break;
    }
  }
}