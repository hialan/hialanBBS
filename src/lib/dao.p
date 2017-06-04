#include <time.h>
#include <stdio.h>
#include "hdr.h"
/* is_alnum.c */
int is_alnum(int ch);
/* is_alpha.c */
int is_alpha(int ch);
/* is_fname.c */
int is_fname(char *str);
/* is_fpath.c */
int is_fpath(char *path);
/* not_addr.c */
int not_addr(char *addr);
/* str_cat.c */
void str_cat(char *dst, char *s1, char *s2);
/* str_cmp.c */
int str_cmp(char *s1, char *s2);
/* str_decode.c */
void str_decode(unsigned char *str);
/* str_dup.c */
char *str_dup(char *src, int pad);
/* str_folder.c */
void str_folder(char *fpath, char *folder, char *fname);
/* str_fpath.c */
void setdirpath(char *fpath, char *direct, char *fname);
/* str_from.c */
int str_from(char *from, char *addr, char *nick);
/* str_hash.c */
int str_hash(char *str, int seed);
/* str_lower.c */
void str_lower(char *dst, char *src);
/* str_ncmp.c */
int str_ncmp(char *s1, char *s2, int n);
/* str_ncpy.c */
void str_ncpy(char *dst, char *src, int n);
/* str_passwd.c */
char *genpasswd(char *pw);
int chkpasswd(char *passwd, char *test);
/* str_pat.c */
int str_pat(const char *str, const char *pat);
/* str_rle.c */
int str_rle(unsigned char *str);
/* str_stamp.c */
void str_stamp(char *str, time_t *chrono);
/* str_str.c */
char *str_str(char *str, char *tag);
/* str_tail.c */
char *str_tail(char *str);
/* str_time.c */
char *Now(void);
char *Btime(time_t *clock);
char *Ctime(time_t *clock);
char *Etime(time_t *clock);
/* str_trim.c */
void str_trim(char *buf);
/* str_ttl.c */
char *str_ttl(char *title);
/* archiv32.c */
void archiv32(time_t chrono, char *fname);
/* archiv32m.c */
void archiv32m(time_t chrono, char *fname);
/* chrono32.c */
time_t chrono32(char *str);
/* hash32.c */
int hash32(unsigned char *str);
/* radix32.c */
/* f_cat.c */
void f_cat(char *fpath, char *msg);
/* f_cp.c */
int f_cp(char *src, char *dst, int mode);
char *f_img(char *fpath, int *fsize);
/* f_ln.c */
int f_ln(char *src, char *dst);
/* f_lock.c */
int f_exlock(int fd);
int f_unlock(int fd);
/* f_map.c */
char *f_map(char *fpath, int *fsize);
/* f_mode.c */
int f_mode(char *fpath);
/* f_mv.c */
int f_mv(char *src, char *dst);
/* f_new.c */
FILE *f_new(char *fold, char *fnew);
/* f_path.c */
void brd_fpath(char *fpath, char *board, char *fname);
void gem_fpath(char *fpath, char *board, char *fname);
void usr_fpath(char *fpath, char *user, char *fname);
/* f_rm.c */
int f_rm(char *fpath);
/* f_suck.c */
void f_suck(FILE *fp, char *fpath);
/* mak_dirs.c */
void mak_dirs(char *fpath);
/* rec_add.c */
/* int rec_add(char *fpath, void *data, int size); */
/* rec_num.c */
//int rec_num(char *fpath, int size);
/* rec_del.c */
//int rec_del(char *data, int size, int pos, int (*fchk)(), int (*fdel)());
/* rec_get.c */
int rec_get(char *fpath, void *data, int size, int pos);
/* rec_ins.c */
//int rec_ins(char *fpath, void *data, int size, int pos, int num);
/* rec_mov.c */
//int rec_mov(char *data, int size, int from, int to);
/* rec_put.c */
//int rec_put(char *fpath, void *data, int size, int pos);
/* hdr_fpath.c */
void hdr_fpath(char *fpath, char *folder, HDR *hdr);
/* hdr_stamp.c */
int hdr_stamp(char *folder, int token, HDR *hdr, char *fpath);
/* xsort.c */
void xsort(void *a, size_t n, size_t es, int (*cmp)());
/* xwrite.c */
int xwrite(int fd, char *data, int size);
/* acl_addr.c */
int acl_addr(char *acl, char *addr);
/* acl_has.c */
int acl_has(char *acl, char *user, char *host);
/* shm.c */
void *shm_new(int shmkey, int shmsize);
/* sem.c */
void sem_init(int semkey,int *semid);
void sem_lock(int op,int semid);
/* setpf.c */
void setadir(char *buf, char *path);
void setapath(char *buf, char *boardname);
//void setbfile(char *buf, char *boardname, char *fname);
void setbgdir(char *buf, char *boardname);
void setbpath(char *buf, char *boardname);
void sethomedir(char *buf, char *userid);
void sethomefile(char *buf, char *userid, char *fname);
void sethomeman(char *buf, char *userid);
void sethomepath(char *buf, char *userid);
/* strip_ansi.c */
int
strip_ansi(char *buf,char *str ,int mode);
/* args.c */
void initsetproctitle(int argc, char **argv,char **envp);
//void setproctitle(const char* cmdline);
void printpt(const char* format, ...);
int countproctitle();
/* ci_strncmp.c */
int ci_strncmp(register char *s1,register char *s2,register int n);
/* cut_ansistr.c */
int cut_ansistr(char *buf, char *str, int len);
/* isprint2.c */
int isprint2(char ch);
/* not_alnum.c */
int not_alnum(register char ch);
/* not_alpha.c */
int not_alpha(register char ch);
/* strstr_lower.c */
char* strstr_lower(char *str,char *tag);
/* Link.c */
int Link(char* src, char* dst);
/* Rename.c */
int Rename(char* src, char* dst);
/* friend_count.c */
int friend_count(char *fname);
// nextfield.c
char *nextfield(register char *data,register char *field);
// bad_user.c
int bad_user(char* name);
/* counter.c */
void counter(char *filename,char *modes,int n);
