#include "miscfunc.h"

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)

#define WMRC ".wmrc"

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <grp.h>

int
rdwr_wm (char *wm, int wml, const char *usr, int rd)
{
    int rv, gidset_size;
    gid_t *gidset;
    char fname[256];
    FILE *file;

    // read passwd
    struct passwd *pwd = getpwnam( usr );
    endpwent();
    if (!pwd)
	return 0;

    // Go user
    gidset_size = getgroups(0, 0);
    if (!(gidset = malloc (sizeof(gid_t) * gidset_size)))
	return 0;
    if( getgroups( gidset_size, gidset) == -1 ||
	initgroups(pwd->pw_name, pwd->pw_gid) != 0 ||
	setegid(pwd->pw_gid) != 0 ||
        seteuid(pwd->pw_uid) != 0
    ) {
	// Error, back out
	seteuid(0);
        setegid(0);
	setgroups( gidset_size, gidset);
	free(gidset);
	return 0;
    }

    // open file as user which is loging in
    sprintf(fname, "%s/" WMRC, pwd->pw_dir);
    rv = 0;
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

    // Go root
    seteuid(0);
    setegid(0);
    setgroups(gidset_size, gidset);
    free(gidset);

    return rv;
}

#endif /* HAVE_INITGROUPS && HAVE_GETGROUPS && HAVE_SETGROUPS */


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

