//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
// 
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
// 
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
// 
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
// 
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
/* LINTLIBRARY */
/*
 * This is the file with all the library routines in it
 *
 * Author information:
 * Matt Bishop
 * Department of Computer Science
 * University of California at Davis
 * Davis, CA  95616-8562
 * phone (916) 752-8060
 * email bishop@cs.ucdavis.edu
 *
 * This code is placed in the public domain.  I do ask that
 * you keep my name associated with it, that you not represent
 * it as written by you, and that you preserve these comments.
 * This software is provided "as is" and without any guarantees
 * of any sort.
 *
 * Version information:
 * 1.0          May 25, 1994            Matt Bishop
 * 1.1      July 5, 1994        Matt Bishop
 *  added TZ to the list of environment variables to be
 *  passed on by default; you get what the environment
 *  gives you (as required by System V based systems)
 * 1.2      October 4, 1994     Matt Bishop
 *  added mxfpopen, mxfpclose; also cleaned up le_set(),
 *  in that before if you added a predefined environment
 *  variable as the first variable, it would process it,
 *  initialize the environment list (first call), and
 *  then append the name; now if le_set() is called, it
 *  initializes the environment and then does the checking
 * 1.3      October 31, 1994    Matt Bishop
 *  made the globals static for better modularity
 */

#include "stdcore.h"

/*
 * set, reset environment to be passed to mpopem
 */
#include "config.h"
#include <stdio.h>
#include <sys/types.h>

#if !IS_SORTIX && HAVE_SYS_FILE_H
#    include <sys/file.h>
#endif

#include <sys/stat.h>

#if HAVE_SYS_WAIT_H
#    include <sys/wait.h>
#endif

#include <time.h>

#if HAVE_MALLOC_H && !IS_AROS
#    include <malloc.h>
#endif

#include <string.h>
//#include <signal.h>
#include "tw_signal.h"

#ifdef __STDC__
#    include <unistd.h>
#    include <stdlib.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#   include <sys/param.h>
#endif

#include <assert.h>
#include "msystem.h"

#if USES_MSYSTEM
/*
 * signal type
 */
#ifndef SIG_TYPE
#   define SIG_TYPE void
#endif

/*
 * define error message printer
 */
#define ERMSG(x)    if (le_verbose){                \
                (void) fprintf(stderr, "SE internal error: ");\
                (void) fprintf(stderr, "%s(%d): %s\n",  \
                    __FILE__, __LINE__-4, x);   \
            }

/*
 * define limits
 *
 * for the popen/pclose clones, we need to store PIDs in an array;
 * how big should it be?  answer: since each popen call requires 1
 * pipe, it can only be as big as the maximim number of pipes allowed
 * that number is MAX_MPOPEN
 */
#ifndef MAX_MPOPEN
#   define MAX_MPOPEN   20
#endif
/*
 * all environment variable arrays are dynamically allocated; if they are
 * too small, they grow by PTR_INC to accommodate the new variable
 * changing this just causes more (or less) allocations; it's an efficiency
 * consideration, not a security or system one
 */
#define PTR_INC     1024    /* how much to increment pointer arrays */
/*
 * this is the maximum number of signals; we use NSIG if that's
 * defined, otherwise 32 (which seems to be right for most systems)
 */
#ifdef NSIG
#   define MAX_SIGNAL   NSIG
#else
#   define MAX_SIGNAL   32
#endif
/*
 * this is the maximum number of file descriptors (NFILES if 
 * defined, otherwise 256 (which seems to be right for most systems)
 */
#ifdef NFILES
#   define MAX_DESC     NFILES
#else
#   define MAX_DESC     256
#endif

#ifdef NOFILE
#   define TW_NO_DESC          NOFILE
#else
#   define TW_NO_DESC          MAX_DESC+1
#endif

/*
 * in case the subprocess fails to exec the command properly
 */
#define EXIT_BAD    -1  /* oops! */

/*
 * now, the environment
 *
 * the default environment; you get the bare bones here.
 * to add to it, just stick the new environment variables
 * at the end of the array; the program does the rest automatically
 */
const char* nvfix[] = {         /* these MUST be set or reset */
    DEF_PATH,               /* a safe path */
    DEF_SHELL,              /* a safe shell */
    DEF_IFS,                /* a safe IFS */
    DEF_TZ,                 /* the current time zone */
    NULL                /* add new ones here */
};
#define SZ_NVFIX    (sizeof(nvfix)/sizeof(char *))  /* size of nvfix */
static int octmask = DEF_UMASK;     /* default umask */
static int mresetgid = UID_RESET;   /* reset EGID to RGID by default */
static int mresetuid = GID_RESET;   /* reset EUID to RUID by default */
static int fdleave[MAX_DESC];       /* 1 to keep file descriptor open */
static char **envp = NULL;      /* environment passed to child */
static int sz_envp = 0;
static int nend = 0;            /* # entries in envp */
static int le_verbose = 1;      /* 1 to print error messages */

/*
 * structure for malloc
 */
union xyzzy {
    char **cpp;     /* doubly-indirect pointer */
#ifdef __STDC__
    void *vp;       /* generic pointer */
#else
    char *vp;       /* generic pointer */
#endif
};          /* used to cast malloc properly */

/*
 * library functions
 */
#ifndef __STDC__
char *getenv();             /* get variable from environment */
#endif
/************* U T I L I T Y    F U N C T I O N S *******************/

/*
 * string duplication into private memory
 * on some systems, this is a library function, so define STRDUP
 * if it is on yours
 */
#if 0
#ifdef STRDUP
#   ifndef __STDC__
    char *strdup();
#   endif
#else
#   ifdef __STDC__
    static char *strdup(char* str)
#   else
    static char *strdup(str)
    char *str;
#   endif
    {
        char *p;   /* temp pointer */

        /*
         * allocate space for the string, and copy if successful
         */
        size_t p_size = (strlen(str)+1)*sizeof(char);
        if ((p = (char*)malloc((unsigned)(p_size)))
                                    != NULL)
            (void) strncpy(p, str, p_size);
        return(p);
    }
#endif
#endif //0
    
/*
 * allocate space for an array of pointers, OR
 * (if space already allocated) increase the allocation by PTR_INC
 */
#ifdef __STDC__
static char **c2alloc(const char**old, int *sz_alloc)
#else
static char **c2alloc(old, sz_alloc)
char **old;
int *sz_alloc;
#endif
{
    int i;     /* counter in a for loop */
    union xyzzy x;      /* used to cast malloc properly */

    /*
     * allocate space for the new (expanded) array
     */
    x.vp = malloc((unsigned) ((*sz_alloc + PTR_INC) * sizeof(char *)));
    if (x.vp != NULL){
        /* success! copy the old and free it, if appropriate */
        if (old != NULL){
            for(i = 0; i < *sz_alloc; i++)
                x.cpp[i] = (char*)old[i];
            x.cpp = (char**)old;
            (void) free(x.vp);
        }
        /* now have PTR_INC more room */
        *sz_alloc += PTR_INC;
    }

    /*
     * return pointer to new space
     */
    return(x.cpp);
}

#ifdef __STDC__
static int initenv(void)
#else
static int initenv()
#endif
{
    int i;
    int rval;

    if (envp != NULL)
        le_clobber();
    for(i = 0; nvfix[i] != NULL; i++)
        if ((rval = le_set(nvfix[i])) != SE_NONE)
            return(rval);
    return(SE_NONE);
}

/************* E N V I R O N M E N T  C O N T R O L *******************/

/*
 * clobber the internal environment
 */
#ifdef __STDC__
void le_clobber(void)
#else
void le_clobber()
#endif
{
    int i;     /* counter in a for loop */
    union {
        char **ep;
        char *p;
    } x;

    /*
     * if the environment is defined and not fixed, clobber it
     */
    if (envp != NULL){
        /* it's defined -- is it fixed? */
        if (envp != (char**)nvfix){
            /* no -- usual walk the list crud */
            for(i = 0; envp[i] != NULL; i++)
                (void) free(envp[i]);
            x.ep = envp;
            (void) free(x.p);
        }
        /* say there's not anything there any more */
        envp = NULL;
    }

    /*
     * now clobber the sizes
     */
    nend = sz_envp = 0;
}

/*
 * get a pointer to the environment element
 */
#ifdef __STDC__
static int le_getenv(const char* var)
#else
static int le_getenv(var)
char *var;
#endif
{
    int i;         /* counter in a for loop */
    char *p, *q;       /* used to compare two strings */

    /*
     * check for no environment
     */
    if (envp == NULL)
        return(-1);
    
    /*
     * there is one -- now walk the environment list
     */
    for(i = 0; envp[i] != NULL; i++){
        /* compare */
        p = envp[i];
        q = (char*)var;
        while(*p && *q && *p == *q)
            p++, q++;
        /* have we a match? */
        if ((*p == '=' || *p == '\0') && (*q == '=' || *q == '\0')){
            /* YES -- return its index */
            return(i);
        }
    }

    /*
     * no match
     */
    return(-1);
}

/*
 * set an environment variable
 */ 
#ifdef __STDC__
int le_set(const char* env)
#else
int le_set(env)
char *env;
#endif
{
    char *p, *q;       /* what is to be put into env */
    int n;         /* where a previous definition is */

    /*
     * see if you need to create the environment list
     */
    if (sz_envp == 0){
            if ((envp = c2alloc((const char**)envp, &sz_envp)) == NULL){
            ERMSG("ran out of memory");
            return(SE_NOMEM);
        }
        for(nend = 0; nvfix[nend] != NULL; nend++)
            if ((envp[nend] = strdup(nvfix[nend])) == NULL){
                ERMSG("ran out of memory");
                return(SE_NOMEM);
            }
        envp[nend] = NULL;
    }

    /*
     * if there is an = sign,
     * it's a redefinition; if not,
     * just include it from the current environment
     * (if not defined there, don't define it here)
     */
    if (strchr(env, '=') == NULL) {

        q = getenv(env);

        /* is it defined locally? */
        if (NULL == q) {
            /* no -- don't define it here */
            return(SE_NONE);
        }

        size_t p_size = strlen(env) + strlen(q) + 2;

        if ((p = (char*)malloc((unsigned)(p_size))) == NULL) {
            ERMSG("ran out of memory");
            return(SE_NOMEM);
        }
        else {
            (void) strncpy(p, env, p_size);
            (void) strncat(p, "=", p_size);
            (void) strncat(p, q, p_size);
        }
    }
    else if ((p = strdup(env)) == NULL){
        ERMSG("ran out of memory");
        return(SE_NOMEM);
    }

    /*
     * if it isn't defined, see if you need to create the environment list
     */
    if (nend == sz_envp && (envp = c2alloc((const char**)envp, &sz_envp)) == NULL){
        ERMSG("ran out of memory");
        return(SE_NOMEM);
    }

    /*
     * add it to the environment
     * if it is already defined, delete the old definition
     * and replace it with the new definition
     */
    if ((n = le_getenv(env)) > -1){
        (void) free(envp[n]);
        envp[n] = p;
        return(SE_NONE);
    }

    envp[nend++] = p;
    envp[nend] = NULL;

    /*
     * all done
     */
    return(SE_NONE);
}

/*
 * clear a current environment variable
 */
#ifdef __STDC__
int le_unset(const char* env)
#else
int le_unset(env)
char *env;
#endif
{
    int i;     /* counter in a for loop */

    /*
     * delete it from the environment
     */
    if ((i = le_getenv(env)) > -1){
        (void) free(envp[i]);
        for( ; envp[i] != NULL; i++)
            envp[i] = envp[i+1];
        return(SE_NONE);
    }

    /*
     * no such variable
     */
    return(SE_NOVAR);
}

/*
 * set the default umask
 */
#ifdef __STDC__
int le_umask(int umak)
#else
int le_umask(umak)
int umak;
#endif
{
    /*
     * reset the umask
     */
    octmask = umak;
    return(SE_NONE);
}

/*
 * leave a file descriptor open
 */
#ifdef __STDC__
int le_openfd(int fd)
#else
int le_openfd(fd)
int fd;
#endif
{
    /*
     * check args
     */
    if (0 > fd || fd >= MAX_DESC)
        return(SE_BADFD);
    /*
     * mark the descriptor for leaving open
     */
    fdleave[fd] = 1;
    return(SE_NONE);
}

/*
 * mark a file descriptor closed
 */
#ifdef __STDC__
int le_closefd(int fd)
#else
int le_closefd(fd)
int fd;
#endif
{
    /*
     * check args
     */
    if (0 > fd || fd >= MAX_DESC)
        return(SE_BADFD);
    /*
     * mark the descriptor for closing
     */
    fdleave[fd] = 0;
    return(SE_NONE);
}

/************* P R I V I L E G E   C O N T R O L *******************/

/*
 * say how to handle the effective (and real) UIDs
 */
#ifdef __STDC__
int le_euid(int uid)
#else
int le_euid( uid)
int uid;
#endif
{
    mresetuid = uid;
    return(SE_NONE);
}

/*
 * say how to handle the effective (and real) GIDs
 */
#ifdef __STDC__
int le_egid(int gid)
#else
int le_egid(gid)
int gid;
#endif
{
    mresetgid = gid;
    return(SE_NONE);
}

/************* S U B C O M M A N D   E X E C U T I O N *******************/

/*
 * get the shell to use for the subcommand
 */
#ifdef __STDC__
static const char *shellenv(void)
#else
static const char *shellenv()
#endif
{
    int i;     /* counter in a for loop */
    const char *shptr; /* points to shell name */

    /*
     * error check; should never happen
     */
    if (envp == NULL && (i = initenv()) != SE_NONE)
        return(NULL);

    /*
     * get the shell environment variable
     */
        for(i = 0; envp[i] != NULL; i++)
                if (strncmp(envp[i], "SHELL=", strlen("SHELL=")) == 0)
                        break;
    /*
     * not defined; use the default shell
     */
        if (envp[i] == NULL)
                shptr = NOSHELL;
        else
                shptr = strchr(envp[i], '=') + 1;
    return(shptr);
}


#if USES_MSYSTEM
/*
 * like system but A LOT safer
 */ 
#ifdef __STDC__
int msystem(const char* cmd)
#else
int msystem(cmd)
char *cmd;
#endif
{
    const char *argv[5];            /* argument list */
    const char *p;     /* temoporary pointers */
    const char* shptr;     /* the program to be run */
    int i;         /* index number of child */

    /*
     * if it's NULL, initialize it
     */
    if (envp == NULL && (i = initenv()) != SE_NONE)
        return(i);

    /*
     * get the SHELL variable (if any)
     */
    shptr = shellenv();

    /*
     * set it up, just like popen
     */
        argv[0] = ((p = strrchr(shptr, '/')) == NULL) ? shptr : p+1;
        argv[1] = "-c";
        argv[2] = cmd;
        argv[3] = NULL;

    /*
     * run it
     */
        if ((i = schild(shptr, (const char**)argv, (const char**)envp, (FILE **) NULL, octmask)) < 0)
                return(127);
    return(echild(i));
}
#endif // USES_MSYSTEM

/*
 * this structure holds the information associating
 * file descriptors and PIDs.  It ks needed as the mpopen/mpclose interface
 * uses file pointers but the wait call needs a PID
 */
static struct popenfunc {   /* association of pid, file pointer */
    int pid;            /* the process identifier */
    FILE *fp;           /* the file pointer */
} pfunc[MAX_MPOPEN];

#if USES_MPOPEN
/*
 * like popen but A LOT safer
 */ 
#ifdef __STDC__
FILE *mpopen(const char* cmd, const char* mode)
#else
FILE *mpopen(cmd, mode)
char *cmd;
char *mode;
#endif
{
    const char *argv[5];            /* argument list */
    const char *p;     /* temoporary pointers */
    const char *shptr;     /* the program to be run */
    FILE *fpa[3];           /* process communication descriptors */
    int indx;      /* index number of child */

    /*
     * see if anything is available
     */
    for(indx = 0; indx < MAX_MPOPEN; indx++)
        if (pfunc[indx].pid == 0)
            break;
    if (indx == MAX_MPOPEN)
        return(NULL);

    /*
     * now get the SHELL variable (if any)
     */
    shptr = shellenv();

    /*
     * set it up, just like popen
     */
        argv[0] = ((p = strrchr(shptr, '/')) == NULL) ? shptr : p+1;
        argv[1] = "-c";
        argv[2] = cmd;
        argv[3] = NULL;

        fpa[0] = (*mode == 'w') ? stdin : NULL;
        fpa[1] = (*mode == 'r') ? stdout : NULL;
        fpa[2] = NULL;

    /*
     * run it
     */
        if ((pfunc[indx].pid = schild(shptr, (const char**)argv, (const char**)envp, fpa, octmask)) < 0)
                return(NULL);
    return(pfunc[indx].fp = ((*mode == 'w') ? fpa[0] : fpa[1]));
}
#endif

/*
 * close the pipe
 */ 
#ifdef __STDC__
int mpclose(FILE *fp)
#else
int mpclose(fp)
FILE *fp;
#endif
{
    int indx;  /* used to look for corresponding pid */
    int rstatus;   /* return status of command */

    /*
     * loop until you find the right process
     */
    for(indx = 0; indx < MAX_MPOPEN; indx++)
        if (pfunc[indx].fp == fp){
            /* got it ... flush and close the descriptor */
            (void) fflush(fp);
            (void) fclose(fp);
            /* get the status code fo the child */
            rstatus = echild(pfunc[indx].pid);
            /* clear the entry and return the code */
            pfunc[indx].pid = 0;
            return(rstatus);
        }
    
    /*
     * no such process - signal no child
     */
    return(-1);
}

/*
 * like popen but A LOT safer
 * uses file descriptors for all three files
 * (0, 1, 2)
 */ 
#ifdef __STDC__
int mfpopen(const char* cmd, FILE *fpa[])
#else
int mfpopen(cmd, fpa)
char *cmd;
FILE *fpa[];
#endif
{
    const char *argv[5];            /* argument list */
    const char *p;     /* temoporary pointers */
    const char *shptr;     /* the program to be run */
    int indx;      /* index number of child */

    /*
     * see if anything is available
     */
    for(indx = 0; indx < MAX_MPOPEN; indx++)
        if (pfunc[indx].pid == 0)
            break;
    if (indx == MAX_MPOPEN)
        return(-1);

    /*
     * now get the SHELL variable (if any)
     */
    shptr = shellenv();

    /*
     * set it up, just like popen
     */
        argv[0] = ((p = strrchr(shptr, '/')) == NULL) ? shptr : p+1;
        argv[1] = "-c";
        argv[2] = cmd;
        argv[3] = NULL;

    /*
     * run it
     */
        if ((pfunc[indx].pid = schild(shptr, (const char**)argv, (const char**)envp, fpa, octmask)) < 0)
                return(-1);
    return(indx);
}

/*
 * close the pipe
 */ 
#ifdef __STDC__
int mfpclose(int indx, FILE *fp[3])
#else
int mfpclose(indx, fp)
int indx;
FILE *fp[];
#endif
{
    int rstatus;   /* return status of command */

    /*
     * loop until you find the right process
     */
    if (pfunc[indx].pid == 0)
        return(-1);
    /* got it ... flush and close the descriptor */
    if (fp[0] != NULL)
        (void) fclose(fp[0]);
    /* get the status code fo the child */
    rstatus = echild(pfunc[indx].pid);
    /* clear the entry and return the code */
    pfunc[indx].pid = 0;
    /* got it ... flush and close the descriptor */
    if (fp[1] != NULL)
        (void) fclose(fp[1]);
    if (fp[2] != NULL)
        (void) fclose(fp[2]);
    return(rstatus);
}

/*
 * like popen but A LOT safer
 * uses arg vector, not command, and file descriptors 0, 1, 2
 */
#ifdef __STDC__
int mxfpopen(const char* argv[], FILE *fpa[])
#else
int mxfpopen(argv, fpa)
char *argv[];
FILE *fpa[];
#endif
{
        int indx;              /* index number of child */

        /*
         * see if anything is available
         */
        for(indx = 0; indx < MAX_MPOPEN; indx++)
                if (pfunc[indx].pid == 0)
                        break;
        if (indx == MAX_MPOPEN)
                return(-1);

        /*
         * run it
         */
        if ((pfunc[indx].pid = schild(argv[0], argv, (const char**)envp, fpa, octmask)) < 0)
                return(-1);
        return(indx);
}

/*
 * close the pipe
 */
#ifdef __STDC__
int mxfpclose(int indx, FILE *fp[3])
#else
int mxfpclose(indx, fp)
int indx;
FILE *fp[];
#endif
{
        return(mfpclose(indx, fp));
}

#if HAVE_FORK
#  define tss_fork() fork()
#elif HAVE_VFORK
#  define tss_fork() vfork()
#endif


/*
 * signal values
 */
static tw_sighandler_t savesig[MAX_SIGNAL];

/*
 * spawn a child; the child's args and environment are as indicated,
 * the file descriptors 0/1/2 are redirected to the open files fp[0]/
 * fp[1]/fp[2] if they are non-NULL, and the umask of the child is set
 * to omask
 */
#ifdef __STDC__
int schild(const char* cmd, const char** argp, const char** envptr, FILE *fp[], int mask)
#else
int schild(cmd, argp, envptr, fp, mask)
char *cmd;
char **argp;
char **envptr;
FILE *fp[];
int mask;
#endif
{
    int p[3][2];            /* pipes to/from child */
    int i;         /* counter in for loop */
    int ch_pid;        /* child PID */
    int euid, egid;    /* in case reset[gu]id is -1 */
    /*
     * create 1 pipe for each of standard input, output, error
     */
    if (fp != NULL){
        if (pipe(p[0]) < 0 || pipe(p[1]) < 0 || pipe(p[2]) < 0){
            ERMSG("pipes couldn't be made");
            return(SE_NOPIPE);
        }
    }

    /*
     * remember the effective uid
     */
    euid = geteuid();
    egid = getegid();

    /*
     * spawn the child and make the pipes the subprocess stdin, stdout
     */
    if ((ch_pid = tss_fork()) == 0){
        /* now reset the uid and gid if desired */
#if HAVE_SETGID      
        if (mresetgid < -1)
            (void) setgid(getgid());
        else if (mresetgid == -1)
            (void) setgid(egid);
        else if (mresetgid > -1)
            (void) setgid(mresetgid);
#endif

#if HAVE_SETUID
        if (mresetuid < -1)
            (void) setuid(getuid());
        else if (mresetuid == -1)
            (void) setuid(euid);
        else if (mresetuid > -1)
            (void) setuid(mresetuid);
#endif
	
        /* reset the umask */
        (void) umask(mask);
        /* close the unused ends of the pipe  */
        /* and all other files except 0, 1, 2 */
        for(i = 3; i < TW_NO_DESC; i++)
            if (fp == NULL || (!fdleave[i] && i != p[0][0]
                        && i != p[1][1] && i != p[2][1]))
                (void) close(i);
        /* if the parent wants to read/write to the child,  */
        /* dup the descriptor; we tell this if the input fp */
        /* array has a NULL in the slot (no interest)       */
        if (fp != NULL){
            if (fp[0] != NULL){
                (void) dup2(p[0][0], 0);
            }
            (void) close(p[0][0]);
                if (fp[1] != NULL){
                (void) dup2(p[1][1], 1);
            }
            (void) close(p[1][1]);
                if (fp[2] != NULL){
                (void) dup2(p[2][1], 2);
            }
            (void) close(p[2][1]);
            }
            /* exec the command and environment */
            (void) execve(cmd, (char* const*)argp, (char* const*)envptr);
            /* should never happen ... */
            _exit(EXIT_BAD);
     }
     /*
      * parent process: if couldn't create child, error
      */
     if (ch_pid != -1)
     {
            /*
             * ignore any signals until child dies
             */
            for(i = 0; i < MAX_SIGNAL; i++)
#ifdef SIGCHLD
                if (i != SIGCHLD)
#endif
                    savesig[i] = tw_sigign(i);
            /*
                 * close unused end of pipes
             */
            if (fp != NULL){
                (void) close(p[0][0]);
                (void) close(p[1][1]);
                (void) close(p[2][1]);
            }
            /*
             * use a stdio interface for uniformity
             */
            if (fp != NULL)
            {
                if (fp[0] != NULL)
                    fp[0] = fdopen(p[0][1], "w");
                else
                    (void) close(p[0][1]);

                if (fp[1] != NULL)
                    fp[1] = fdopen(p[1][0], "r");
                else
                    (void) close(p[1][0]);

                if (fp[2] != NULL)
                    fp[2] = fdopen(p[2][0], "r");
                else
                    (void) close(p[2][0]);
            }
    }
    /*
     * return child's PID
     */
    return(ch_pid);
}

/*
 * wait for child to die
 */
#ifdef __STDC__
int echild(int pid)
#else
int echild(pid)
int pid;
#endif
{
    int r;     /* PID of process just exited */
    int status;     /* status of wait call */

    /*
     * done; wait for child to terminate
     */
    while((r = wait(&status)) != pid && r != -1) ;
    /*
     * if child already dead, assume an exit status of -1
     */
    if (r == -1) status = -1;
    /*
     * restore signal traps
     */
    for(r = 0; r < MAX_SIGNAL; r++)
        (void) tw_signal(r, savesig[r]);
    /*
     * return exit status
     */
    return(status);
}

#endif // SUPPORTS_POSIX_FORK_EXEC
