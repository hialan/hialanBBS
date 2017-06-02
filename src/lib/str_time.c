#include <stdio.h>
#include <time.h>


static char datemsg[32];


char *
Btime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);

  sprintf(datemsg, "%02d/%02d/%02d%3d:%02d:%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
  return (datemsg);
}


char *
Ctime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);
  static char week[] = "日一二三四五六";

  sprintf(datemsg, "%d年%2d月%2d日%3d:%02d:%02d 星期%.2s",
    t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec, &week[t->tm_wday << 1]);
  return (datemsg);
}


char *
Etime(clock)
  time_t *clock;
{
  strftime(datemsg, 22, "%D %T %a", localtime(clock));
  return (datemsg);
}


char *
Now()
{
  time_t now;

  time(&now);
  return Btime(&now);
}
