#include "bbs.h"

/*
  程式設計：wsyfish
  自己評語：寫得很爛，亂寫一通，沒啥深度:)
  相容程度：Ptt板本應該都行吧，就用inmoney和demoney，其他如Sob就要改一下囉
  附帶一提：這程式裡面有耍賤，期望值不是 0
*/
char *dice[6][3] = {"        ",
                    "   ●   ",
                    "        ",
                    "   ●   ",
                    "        ",
                    "   ●   ",
                    "   ●   ",
                    "   ●   ",
                    "   ●   ",
                    "●    ●",
                    "        ",
                    "●    ●",
                    "●    ●",
                    "   ●   ",
                    "●    ●",
                    "●    ●",
                    "●    ●",
                    "●    ●"
};

int
x_dice()
{
  char choice[11],buf[60];
  int  i, money; 
  char tmpchar;    /* 紀錄選項 */
  char tmpdice[3]; /* 三個骰子的值 */
  char totaldice;
  time_t now = time(0);

  time(&now);

  setutmpmode(DICE);
  while(1)
  {
    stand_title("ㄒㄧ ㄅㄚ ㄌㄚ 賭場");
    getdata(1, 0, "請問要下注多少呢？(最少 1 元 /最多 250000 元) ",
            choice, 7, LCECHO, 0);
    money = atoi(choice);
    if(money < 1 || money > 250000 || money > cuser.silvermoney)
    {
      pressanykey("下注金額輸入有誤！離開賭場");
      break;
    }
    outs("\n┌────────────────────────────────────┐\n"
         "│ ２倍  1. 大      2. 小                                                 │\n"
         "│ ５倍  3. 三點    4. 四點     5. 五點    6. 六點    7. 七點    8. 八點  │\n"
         "│       9. 九點   10. 十點    11. 十一點 12. 十二點 13. 十三點 14. 十四點│\n"
         "│      15. 十五點 16. 十六點  17. 十七點 18. 十八點                      │\n"
         "│ ９倍 19. 一一一 20. 二二二  21. 三三三 22. 四四四 23. 五五五 24. 六六六│\n"
         "└────────────────────────────────────┘\n");
    getdata(11, 0, "要押哪一項呢？(請輸入號碼) ", choice, 3, LCECHO, 0);
    tmpchar = atoi(choice);
    if(tmpchar <= 0 || tmpchar > 24)
    {
      pressanykey("要押的項目輸入有誤！離開賭場");
      break;
    }
    demoney(money);
    outs("\n按任一鍵擲出骰子....\n");
    igetkey();

    do
    {
      totaldice = 0;
      for(i = 0; i < 3; i++)
      {
        tmpdice[i] = rand() % 6 + 1;
        totaldice += tmpdice[i];
      }

      if (((tmpchar == 1) && totaldice > 10) ||
          ((tmpchar == 2) && totaldice <= 10))
      {
        if ((rand() % 10) < 6) /* 作弊用，中獎率為原來之60% */
          break;
      }
      else
        break;

    }while(tmpchar <= 2);

    outs("\n╭────╮╭────╮╭────╮\n");

    for(i = 0; i < 3; i++)
      prints("│%s││%s││%s│\n",
             dice[tmpdice[0] - 1][i],
             dice[tmpdice[1] - 1][i],
             dice[tmpdice[2] - 1][i]);

    outs("╰────╯╰────╯╰────╯\n\n");

    if((tmpchar == 1 && totaldice > 10)
       || (tmpchar == 2 && totaldice <= 10)) /* 處理大小 */
    {
      sprintf(buf,"中了！得到２倍獎金 %d 元", money * 2);
      inmoney(money*2); 
    } 
    else if(tmpchar <= 18 && totaldice == tmpchar) /* 處理總和 */
    {
      sprintf(buf,"中了！得到５倍獎金 %d 元", money * 5); 
      inmoney(money * 5);
    } 
    else if((tmpchar - 18) == tmpdice[0] && (tmpdice[0] == tmpdice[1])
            && (tmpdice[1] == tmpdice[2])) /* 處理三個一樣總和 */
    {
      sprintf(buf,"中了！得到９倍獎金 %d 元", money * 9);
      inmoney(money * 9);
    } 

    else /* 處理沒中 */
      sprintf(buf,"很可惜沒有押中！");
    pressanykey(buf);
    game_log("DICE","%s",buf); 
  }
  return 0;
}
