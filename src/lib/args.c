/*  initsetproctitle() 是從 sendmail 的 source code 裡面挖來的...
 *  setproctitle() 我重寫過... countproctitle() 是寫好玩的...
 *  試試看... 有毛病記得告訴我... :)
 *                                      -- Beagle
 */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define newstr(s)    strcpy(malloc(strlen(s) + 1), s)

/*
**  Pointers for setproctitle.
**This allows "ps" listings to give more useful information.
*/

char            **Argv = NULL;          /* pointer to argument vector */
char            *LastArgv = NULL;       /* end of argv */

void
initsetproctitle(argc, argv, envp)
        int argc;
        char **argv;
        char **envp;
{
        register int i;
        extern char **environ;

        /*
        **  Move the environment so setproctitle can use the space at
        **  the top of memory.
        */

        for (i = 0; envp[i] != NULL; i++)
                continue;
        environ = (char **) malloc(sizeof (char *) * (i + 1));
        for (i = 0; envp[i] != NULL; i++)
                environ[i] = newstr(envp[i]);
        environ[i] = NULL;

        /*
        **  Save start and extent of argv for setproctitle.
        */

        Argv = argv;
        if (i > 0)
                LastArgv = envp[i - 1] + strlen(envp[i - 1]);
        else
                LastArgv = argv[argc - 1] + strlen(argv[argc - 1]);
}

void
setproctitle(const char* cmdline) {
        char buf[256], *p;
        int i;

        strncpy(buf, cmdline, 256);
        buf[255] = '\0';
        i = strlen(buf);
        if (i > LastArgv - Argv[0] - 2) {
                i = LastArgv - Argv[0] - 2;
        }
        strcpy(Argv[0], buf);
        p=&Argv[0][i];
        while (p < LastArgv) *p++='\0';
        Argv[1] = NULL;
}

void
printpt(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format,args);
    setproctitle(buf);
    va_end(args);
}

int countproctitle() {
  return (LastArgv - Argv[0] - 2);
}

