/*
////////////////////////////////////////////////////////////////////////////////
//
// File Name     : GetPid.c
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 19/03/2003
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__DragonFly__)
#include <sys/param.h>
#endif

#include <sys/types.h>

#ifndef __cplusplus
#define bool unsigned int
#define false 0
#define true (!false)
#endif

#define BUFSIZE 1024
#define PROCDIR "/proc"

/*
    Get process ID - using name of exe and parent process ID

    Implemented for:

        Linux         Tested on Linux 2.4
        FreeBSD       Tested on FreeBSD 5.1 by Brian Ledbetter <brian@shadowcom.net>
        NetBSD
        Irix
        Solaris       Tested on Solaris 8 x86 by Torsten Kasch <tk@Genetik.Uni-Bielefeld.DE>
        HP-UX         Tested on HP-UX B.11.11 U 9000/800
        AIX
        ...else parse output of "ps -eaf"


    Some sections of this code are copied from / inspired by ksysguard,
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    To test this file, do the following:

    1. Compile this file as follows:

           gcc GetPid.c -DTEST_GETPID -DOS_Linux -o tst

       ...replace OS_Linux with your particular OS type: OS_FreeBSD, OS_NetBSD, OS_Irix, OS_Solaris,
       OS_HPUX, or OS_AIX

    2. Start a program - such as "vi"
    3. Do a "ps -eaf" to ensure there is *only one* process called "vi"
    4. Get the parent process ID of your "vi" above
    5. Call tst with that value -e.g. vi ppid=23 then ./tst vi 23
       ...this should then print out the process ID of "vi"
    6. Email me and let me know if it works!
*/

#if defined OS_Linux || defined __Linux__

#include <dirent.h>
#include <ctype.h>

#define FOUND_NAME 1
#define FOUND_PPID 2
#define FOUND_ALL (FOUND_NAME+FOUND_PPID)

unsigned int kfi_getPid(const char *proc, unsigned int ppid)
{
    bool           error=false;
    unsigned int   pid=0;
    DIR           *dir;
    struct dirent *entry;

    /* read in current process list via the /proc filesystem entry */
    if(NULL!=(dir=opendir(PROCDIR)))
    {
        while((entry=readdir(dir)) && !error)
            if(isdigit(entry->d_name[0]))
            {
                char buf[BUFSIZE];
                FILE *fd;

                snprintf(buf, BUFSIZE-1, PROCDIR"/%d/status", atoi(entry->d_name));

                if(NULL!=(fd=fopen(buf, "r")))
                {
                    char format[32],
                         tagformat[32],
                         tag[32],
                         name[64];
                    int  found=0;

                    found=0;
                    sprintf(format, "%%%d[^\n]\n", (int) sizeof(buf) - 1);
                    sprintf(tagformat, "%%%ds", (int) sizeof(tag) - 1);
                    for(;found<FOUND_ALL;)
                    {
                        if (fscanf(fd, format, buf)!=1)
                            break;
                        buf[sizeof(buf)-1]='\0';
                        sscanf(buf, tagformat, tag);
                        tag[sizeof(tag) - 1] = '\0';
                        if(0==strcmp(tag, "Name:"))
                        {
                            sscanf(buf, "%*s %63s", name);
                            if(NULL==name || 0!=strcmp(name, proc))
                                break;
                            found|=FOUND_NAME;
                        }
                        else if(0==strcmp(tag, "PPid:"))
                        {
                            unsigned int proc_ppid;

                            sscanf(buf, "%*s %u", &proc_ppid);
                            if(ppid!=proc_ppid)
                                break;
                            found|=FOUND_PPID;
                        }
                    }
                    if(FOUND_ALL==found)
                    {
                        if(pid)
                            error=true;
                        else
                            pid=atoi(entry->d_name);
                    }
                    fclose(fd);
                }
            }
        closedir(dir);
    }

    return error ? 0 : pid;
}

#elif defined OS_FreeBSD || defined OS_NetBSD || defined __FreeBSD__ || defined __NetBSD__ || defined OS_Darwin

#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/param.h>
#if __FreeBSD_version > 500015
#include <sys/priority.h>
#endif
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/user.h>
#include <unistd.h>
unsigned int kfi_getPid(const char *proc, unsigned int ppid)
{
    bool              error=false;
    unsigned int      pid=0;
    int               mib[4];
    size_t            len,
                      num;
    struct kinfo_proc *p;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    sysctl(mib, 3, NULL, &len, NULL, 0);
    p=(struct kinfo_proc*)malloc(len);
    sysctl(mib, 3, p, &len, NULL, 0);

    for(num=0; num < len / sizeof(struct kinfo_proc)  && !error; num++)
    {
        struct kinfo_proc proc_p;
        size_t            len;

        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
#if __FreeBSD_version >= 500015
        mib[3] = p[num].ki_pid;
#else
        mib[3] = p[num].kp_proc.p_pid;
#endif

        len=sizeof(proc_p);
        if(-1==sysctl(mib, 4, &proc_p, &len, NULL, 0) || !len)
            break;
        else
        {
#if __FreeBSD_version >= 500015
            if(proc_p.ki_ppid==ppid && p[num].ki_comm && 0==strcmp(p[num].ki_comm, proc))
                if(pid)
                    error=true;
                else
                    pid=p[num].ki_pid;
#else
#if defined(__DragonFly__)
	    if(proc_p.kp_eproc.e_ppid==ppid && p[num].kp_thread.td_comm && 0==strcmp(p[num].kp_thread.td_comm, proc))
#else
            if(proc_p.kp_eproc.e_ppid==ppid && p[num].kp_proc.p_comm && 0==strcmp(p[num].kp_proc.p_comm, proc))
#endif
                if(pid)
                    error=true;
                else
                    pid=p[num].kp_proc.p_pid;
#endif
        }
    }
    free(p);

    return error ? 0 : pid;
}

#elif defined OS_Irix || defined OS_Solaris

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/resource.h>
#ifdef OS_Solaris

#if (!defined(_LP64)) && (_FILE_OFFSET_BITS - 0 == 64)
#define PROCFS_FILE_OFFSET_BITS_HACK 1
#undef _FILE_OFFSET_BITS
#else
#define PROCFS_FILE_OFFSET_BITS_HACK 0
#endif

#include <procfs.h>

#if (PROCFS_FILE_OFFSET_BITS_HACK - 0 == 1)
#define _FILE_OFFSET_BITS 64
#endif

#else
#include <sys/procfs.h>
#include <sys/sysmp.h>
#endif 
#include <sys/sysinfo.h>

unsigned int kfi_getPid(const char *proc, pid_t ppid)
{
    DIR	         *procdir;
    bool         error=false;
    pid_t	 pid=(pid_t)0;

    if(NULL!=(procdir=opendir(PROCDIR)))
    {
        struct dirent *de;

        rewinddir(procdir);
        while((de=readdir(procdir)) && !error)
            if('.'==de->d_name[0])
                continue;
            else
            {
                int        fd;
                char       buf[BUFSIZE];
#ifdef OS_Solaris
                psinfo_t   psinfo;

                snprintf(buf, BUFSIZE - 1, "%s/%s/psinfo", PROCDIR, de->d_name);
#else
                prpsinfo_t psinfo;

                sprintf(buf, PROCDIR"/pinfo/%ld", pid);
#endif

                if((fd=open(buf, O_RDONLY))<0)
                    continue;

#ifdef OS_Solaris
                if(sizeof(psinfo_t)!=read(fd, &psinfo, sizeof(psinfo_t)))
#else
                if(ioctl(fd, PIOCPSINFO, &psinfo)<0)
#endif
                {
                    close(fd);
                    continue;
                }
                close(fd);

                if(psinfo.pr_ppid==ppid && psinfo.pr_fname && 0==strcmp(psinfo.pr_fname, proc))
                    if(pid)
                        error=true;
                    else
                        pid=psinfo.pr_pid;
	    }
        closedir(procdir);
    }

    return error ? 0 : pid;
}

#elif defined OS_HPUX

#include <sys/pstat.h>
#define MAX_PROCS 50

unsigned int kfi_getPid(const char *proc, unsigned int ppid)
{
    bool              error=false;
    unsigned int      pid=0;
    int               i,
                      count,
                      idx=0; 
    struct pst_status pst[MAX_PROCS];

    while((count=pstat_getproc(&pst[0], sizeof(pst[0]), MAX_PROCS, idx)) > 0 && !error)
    {
        for (i = 0; i<count && !error; i++) 
            if(pst[i].pst_ppid==ppid && pst[i].pst_ucomm && 0==strcmp(pst[i].pst_ucomm, proc))
                if(pid)
                    error=true;
                else
                    pid=pst[i].pst_pid;

        idx=pst[count-1].pst_idx+1;
    }

    return error ? 0 : pid;
}

#elif defined OS_AIX

#include <procinfo.h>
#define MAX_PROCS 50

unsigned int kfi_getPid(const char *proc, unsigned int ppid)
{
    bool             error=false;
    unsigned int     pid=0;
    int              i,
                     count,
                     idx=0;
    struct procsinfo pi[MAX_PROCS];

    while((count=getprocs(&pi, sizeof(pi[0]), 0, 0, &pid, 1)) >0 && !error)
    {
        for (i = 0; i<count && !error; i++)
            if(pi[i].pi_ppid==ppid && pi[i].pi_comm && 0==strcmp(pi[i].pi_comm, proc))
                if(pid)
                    error=true;
                else
                    pid=pi[i].pi_pid;

        idx=pi[count-1].pi_idx+1;
    }

    return error ? 0 : pid;
}

#else
#warning "Unable to determine operating system version!  This may cause the getPid() function to fail at random!"

/* Default to reading "ps -eaf" output */

#include <pwd.h>
#include <limits.h>
#include <ctype.h>

#define FOUND_PID  1
#define FOUND_PPID 2
#define FOUND_CMD  4
#define FOUND_ALL  (FOUND_PID+FOUND_PPID+FOUND_CMD)

static int checkCmd(const char *proc, const char *cmd)
{
    int len=(int)strlen(cmd),
        ch;

    if(len>1)
        for(ch=len-2; ch>=0; --ch)
            if('/'==cmd[ch])
                return strcmp(proc, &cmd[ch+1]);

    return strcmp(proc, cmd);
}

unsigned int kfi_getPid(const char *proc, unsigned int ppid)
{
    bool         error=false;
    unsigned int pid=0;
    static int   pid_c=-1,
                 ppid_c=-1,
                 time_c=-1,
                 cmd_c=-1;

    char         cmd[BUFSIZE+1];
    FILE         *p;

    /* If this function has been run before, and we know the column positions, then we can grep for just our command */
    if(-1!=pid_c && -1!=ppid_c && -1!=time_c && -1!=cmd_c)
        snprintf(cmd, BUFSIZE, "ps -eaf | grep %s", proc);
    else
        strcpy(cmd, "ps -eaf");

    if(NULL!=(p=popen(cmd, "r")))
    {
        char line[BUFSIZE+1];
        int  c=0;
        char *linep=NULL,
             *token=NULL;

        /* Read 1st line to determine columns... */
        if((-1==pid_c || -1==ppid_c || -1==time_c || -1==cmd_c) && NULL!=fgets(line, BUFSIZE, p))
        {
            for(linep=line; -1==pid_c || -1==ppid_c || -1==time_c || -1==cmd_c; linep=NULL)
                if(NULL!=(token=strtok(linep, " \t\n")))
                {
                    if(0==strcmp("PID", token))
                        pid_c=c;
                    else if(0==strcmp("PPID", token))
                        ppid_c=c;
                    else if(NULL!=strstr("TIME", token))
                        time_c=c;
                    else if(0==strcmp("COMMAND", token) || 0==strcmp("CMD", token))
                        cmd_c=c;
                    c++;
                }
                else
                    break;
        }

        /* If all column headings read, then look for details... */ 
        if(-1!=pid_c && -1!=ppid_c && -1!=time_c && -1!=cmd_c)
            while(NULL!=fgets(line, BUFSIZE, p) && !error)
            {
                int found=0,
                    ps_pid=0,
                    offset=0;

                c=0;
                for(linep=line; FOUND_ALL!=found; linep=NULL)
                    if(NULL!=(token=strtok(linep, " \t\n")))
                    {
                        if(c==pid_c)
                        {
                            found|=FOUND_PID;
                            ps_pid=atoi(token);
                        }
                        else if(c==ppid_c)
                        {
                            if(((unsigned int)atoi(token))!=ppid)
                                break;
                            found|=FOUND_PPID;
                        }
                        else if(c==time_c)
                            offset=isdigit(token[0]) ? 0 : 1;
                        else if(c==(cmd_c+offset))
                        {
                            if(0!=checkCmd(proc, token))
                                break;
                            found|=FOUND_CMD;
                        }
                        c++;
                    }
                    else
                        break;

                if(FOUND_ALL==found)
                {
                    if(pid)
                        error=true;
                    else
                        pid=ps_pid;
                }
            }
        pclose(p);
    }

    return error ? 0 : pid;
}

#endif

#ifdef TEST_GETPID
int main(int argc, char *argv[])
{
    if(3==argc)
        printf("PID %u\n", kfi_getPid(argv[1], atoi(argv[2])));
    else
        printf("Usage: %s <process> <parent-process-id>\n", argv[0]);
    return 0;
}
#endif
