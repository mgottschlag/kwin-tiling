#include "miscfunc.h"

#ifdef USE_RDWR_WM

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <grp.h>

int
rdwr_wm (char *wm, int wml, const char *usr, int rd)
{
    int rv;
    char fname[256];
    FILE *file;

    /* read passwd */
    struct passwd *pwd = getpwnam( usr );
    endpwent();
    if (!pwd)
	return -2;

    /* Go user */
    rv = -1;
    setgroups( 0, 0);	/* take away root group - forever! */
    if( !setegid(pwd->pw_gid) ) {
	if( !seteuid(pwd->pw_uid) ) {

	    /* open file as user which is loging in */
	    sprintf(fname, "%s/" WMRC, pwd->pw_dir);
	    if (rd) {
		if ( (file = fopen(fname, "r")) != NULL ) {
		    fgets (wm, wml, file);
		    rv = strlen (wm);
		    if (rv && wm[rv - 1] == '\n')
			wm[--rv] = '\0';
		    fclose (file);
		}
	    } else {
		if ( (file = fopen(fname, "w")) != NULL ) {
		    fputs (wm, file);
		    rv = 1;
		    fclose (file);
		}
	    }

	    seteuid(0);
	}
	setegid(0);
    }

    return rv;
}

#endif /* USE_RDWR_WM */


int s_copy (char *dst, const char *src, int idx, int spc)
{
    int dp = 0;

    while (src[idx] == ' ')
	idx++;
    for (; src[idx] >= ' ' && (spc || src[idx] != ' '); idx++)
	if (dp < F_LEN - 1)
	    dst[dp++] = src[idx];
    dst[dp] = '\0';
    return idx;
}

