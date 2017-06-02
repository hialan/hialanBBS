/*-------------------------------------------------------*/
/* color.c      ( WD_hialan BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : 色彩選擇/光棒相關函數/                       */
/* create : 2003/07/04                                   */
/* update : 2003/07/04                                   */
/*-------------------------------------------------------*/
#include "bbs.h"

/*Add For LightBAR by hialan*/

/*-------------------*/
/*光棒 檢查-更動 函數*/
/*-------------------*/


#define LB_BG 0	//背景
#define LB_WD 1 //文字
#define LB_LT 2 //light 亮度
#define LB_BL 3 //blink 閃爍
#define LB_UL 4 //underline 底線

int
get_color(char *color, char cset[5])
{
  char buf[40];
  
  color[0] = '\0';
  
  
  if(cset[LB_LT] == 0)
    strcpy(color, "\033[m");
  else if(cset[LB_LT] == 1)
    strcpy(color, "\033[1m");

  if(cset[LB_BG])
  {
    sprintf(buf,"\033[4%dm",cset[LB_BG]);
    strcat(color, buf);
  }

  if(cset[LB_BL])
    strcat(color,"\033[5m");

  if(cset[LB_UL])
    strcat(color,"\033[4m");
  
  /*文字顏色*/
  sprintf(buf,"\033[3%dm",cset[LB_WD]);
  strcat(color,buf);

  return 0;
}

int
save_lightbar_color(char *color)
{
  int i, unum = do_getuser(cuser.userid, &xuser);
  
  if(unum > 0)
  {
    for(i=0;i<5;i++)
      xuser.lightbar[i]=color[i];
    substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
    update_data();
    return 1;
  }
  else
    return -1;
}

int 
get_lightbar_color(char *color)
{
  get_color(color, cuser.lightbar);
}



int
change_lightbar()
{
  char ch[5];
  int i;

  for(i=0;i<5;i++)
    ch[i]=cuser.lightbar[i];
  
  if(color_selector(ch) == 1)
  {
    if(save_lightbar_color(ch) == 1)
      pressanykey("修改完成!!");
    else
      pressanykey("修改失敗>_<");
  }
  else
    pressanykey("取消!!");
  
  return 0;
}  

/*---------------------------*/
/* 顏色選擇器		     */
/*---------------------------*/
static int
selector_item(char *prompt, int line, int pos, char *lightbar, int def)
{
  int bottom, num, i;
  
  char **choose;
  char *color[8]={"0)黑色","1)紅色","2)綠色","3)黃色","4)藍色","5)紫色","6)青色","7)白色"};
  char *underline[2]={"0)無底線","1)底線"};
  char *blink[2]={"0)不閃爍","1)閃爍"};
  char *light[3]={"0)低亮度","1)高亮度","2)不設定"};
  

  switch(line)
  {
    case 0:  //背景
      bottom=b_lines - 5;
      num=8;
      choose=color;
      break;
    case 1:  //文字
      bottom=b_lines - 4;
      num=8;
      choose=color;    
      break;
    case 2:  //亮度
      bottom=b_lines - 3;
      num=3;
      choose=light;
      break;
    case 3:  //閃爍
      bottom=b_lines - 2;
      num=2;
      choose=blink;
      break;
    case 4:  //底線
      bottom=b_lines - 1;
      num=2;
      choose=underline;
      break;
  }
  
  move(bottom, 0);
  clrtoeol();
  outs(prompt);

  if (pos < 0)		//hialan.030704:這邊偷懶, 把迴圈選擇寫在這邊XD
    pos = num-1;
  else if(pos >=num)
    pos = 0;
    
  for(i=0;i<num;i++)
  {
    if(i==pos)
      prints("[%s%s\033[m]", lightbar, choose[i]);
    else
      prints(" %s ", choose[i]);
  }
  return pos;
}


//hialan.030704:每一行的值由 color 記憶, 第幾列由 pos 記憶
int
color_selector(char *color)
{
  int i;
  int pos=0, ch;
  char *ques[5]={"  底色 ","  字色 ","  亮度 ","  閃爍 ","  底線 "}; 
  int def[5]={7, 4, 0, 0, 1};
  char lightbar[40];
  char preview[64];

  get_color(lightbar, cuser.lightbar);

  clear();
  
  sprintf(preview, "%s [線上 %d 人]", BOARDNAME, count_ulist());
  showtitle("顏色設定", preview);  

  for(i=0;i<5;i++)
    color[i] = selector_item(ques[i], i, color[i], lightbar, def[i]);

  move(b_lines, 0);
  prints("\033[1;33;44m  調 色 盤  \033[1;46m%-68.68s\033[m", 
    	 " [q]取消  [Enter]確定修改 [↑↓←→]改變修改選項");    
  
  while(ch !='\n' && ch !='q')
  {
    color[pos]= selector_item(ques[pos], pos, color[pos], lightbar, def[pos]);
    get_color(preview, color);
    move(b_lines/2, 34);
    clrtoeol();
    prints("%s這裡是預覽喔!!\033[m", preview);
    ch = cursor_key(b_lines-5+pos, 0);

    switch(ch)
    {
      case KEY_UP:
        pos--;
        if(pos < 0) pos = 4;
        break;
      case KEY_DOWN:
        pos++;
        if(pos > 4) pos = 0;
        break;
      case KEY_LEFT:
        color[pos]--;
        break;
      case KEY_RIGHT:
        color[pos]++;
        break;
      case '\r':
      case '\n':
        return 1;	//確定!!
      case 'q':
        return 0;	//取消!!
    }
  }
}