/* ----------------------------------------------------- */
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : dynamic link modules library for maple bbs   */
/* create : 99/02/14                                     */
/* update :   /  /                                       */
/* ----------------------------------------------------- */

#include "dao.h"
#include <dlfcn.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define BBSHOME "/home/nation"

typedef struct
{
  char *path;
  void *handle;
}DL_list;

DL_list  *dl_pool;
int dl_size, dl_head; 

#define DL_ALLOC_MIN	20

void
log_dl(mode, msg)
  char *mode, *msg;
{
  char buf[512];
  time_t now;
  
  time(&now);
  sprintf(buf, "%s %s %s", Etime(&now), mode, msg);
  f_cat(BBSHOME"/log/dlog", buf);
}

void *
DL_get(name)
  char *name;
  /* format: "Xmodule_path:Xname" */
{
  char buf[512] = "\0", *t;
  DL_list *p, *tail;

  strcpy(buf,name);
  if (!(t = strchr(buf,':')))
  {
    log_dl("DL_NAME_ERROR", buf);
    return NULL;
  }
  log_dl("DL_NAME",t);
  *t++ = 0;
  
  if(!dl_pool) 
  {
    /* Initialize DL entries */
    dl_size = DL_ALLOC_MIN;
//    dl_head = 0;
    dl_head = 0;
    dl_pool = (DL_list *)malloc(dl_size * sizeof(DL_list));
    log_dl("DL_NEW",buf);
  }
  
  p = dl_pool;
  tail = p + dl_head;
  while(p < tail)
  {
    if(!strcmp(buf, p->path))
    {
      break;
      log_dl("DL_PATH",p->path);
    }
    p++;
  }

  if(p >= tail)
  { /* not found */
    if(dl_head >= dl_size)
    { /* not enough space */
      dl_size += DL_ALLOC_MIN;
      dl_pool = (DL_list *)realloc(dl_pool, dl_size * sizeof(DL_list));
      p = dl_pool + dl_head; /* Thor.991121: to a new place */
    }
    p->handle = dlopen(p->path = strdup(buf), RTLD_NOW);
    dl_head ++;
    log_dl("DL_RUN",buf);
  }
  if(!p->handle)
  {
    log_dl("DL_FAIL",buf);
    return NULL;
  }
  log_dl("DL_CLOSE",t);    
//  dlclose(p->handle);
  return dlsym(p->handle,t);
}

int 
DL_func(char* name, ...)
{
  va_list args;
  int (*f)(), ret;

  va_start(args, name);

  if(!(f = DL_get(name)))
  { /* not get func */
    ret = -1;
  }
  else
  {
    ret = (*f)(args);
  }

  va_end(args);

  return ret;
}
