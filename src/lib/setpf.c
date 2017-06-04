#include <stdio.h>
void
setadir(buf, path)
  char *buf, *path;
{
  sprintf(buf, "%s/%s", path, ".DIR");
}
void
setapath(buf, boardname)
  char *buf, *boardname;
{
  sprintf(buf, "man/boards/%s", boardname);
}
void
setbfile(buf, boardname, fname)
  char *buf, *boardname, *fname;
{
  sprintf(buf, "boards/%s/%s", boardname, fname);
}
void                                                          
setbgdir(buf, boardname)                                      
  char *buf, *boardname;                                      
{                                                             
sprintf(buf, "boards/%s/%s", boardname, ".Names");
}
void
setbpath(buf, boardname)
  char *buf, *boardname;
{
  sprintf(buf, "boards/%s", boardname);
}
void
sethomedir(buf, userid)
  char *buf, *userid;
{
  sprintf(buf, "home/%s/%s", userid, ".DIR");
}
void
sethomefile(buf, userid, fname)
  char *buf, *userid, *fname;
{
  sprintf(buf, "home/%s/%s", userid, fname);
}
void
sethomeman(buf, userid)
  char *buf, *userid;
{
  sprintf(buf, "home/%s/%s", userid, "man");
}
void
sethomepath(buf, userid)
  char *buf, *userid;
{
  sprintf(buf, "home/%s", userid);
}

