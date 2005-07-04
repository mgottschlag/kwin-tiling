/*

KDM remote control application

Copyright (C) 2004 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>

static int
openctl( int fd, int err, const char *ctl, const char *dpy )
{
	struct sockaddr_un sa;

	sa.sun_family = AF_UNIX;
	if (dpy)
		snprintf( sa.sun_path, sizeof(sa.sun_path),
		          "%s/dmctl-%s/socket", ctl, dpy );
	else
		snprintf( sa.sun_path, sizeof(sa.sun_path),
		          "%s/dmctl/socket", ctl );
	if (!connect( fd, (struct sockaddr *)&sa, sizeof(sa) ))
		return 1;
	if (err)
		fprintf( stderr, "Cannot connect socket '%s'.\n", sa.sun_path );
	return 0;
}

static const char *
readcfg( const char *cfg )
{
	FILE *fp;
	const char *ctl;
	char *ptr, *ptr2;
	char buf[1024];

	if (!(fp = fopen( cfg, "r" ))) {
		fprintf( stderr,
		         "Cannot open kdm config file '%s'.\n",
		         cfg );
		return 0;
	}
	ctl = "/var/run/xdmctl";
	while (fgets( buf, sizeof(buf), fp ))
		if (!strncmp( buf, "FifoDir", 7 )) {
			ptr = buf + 7;
			while (*ptr && isspace( *ptr ))
				ptr++;
			if (*ptr++ != '=')
				continue;
			while (*ptr && isspace( *ptr ))
				ptr++;
			for (ptr2 = buf + strlen( buf );
			     ptr2 > ptr && isspace( *(ptr2 - 1) );
			     ptr2--);
			*ptr2 = 0;
			ctl = strdup( ptr );
			break;
		}
	fclose( fp );
	return ctl;
}

static int
exe( int fd, const char *in, int len )
{
	char buf[4096];

	if (write( fd, in, len ) != len) {
		fprintf( stderr, "Cannot send command\n" );
		return 1;
	}
	if ((len = read( fd, buf, sizeof(buf) )) <= 0) {
		fprintf( stderr, "Cannot receive reply\n" );
		return 1;
	}
	if (len == sizeof(buf) && buf[sizeof(buf) - 1] != '\n')
		fprintf( stderr, "Warning: reply is too long\n" );
	fwrite( buf, 1, len, stdout );
	if (len == sizeof(buf) && buf[sizeof(buf) - 1] != '\n')
		puts( "[...]" );
	return 0;
}

static int
run( int fd, char **argv )
{
	unsigned len, l;
	char buf[1024];

	if (!*argv)
		return exe( fd, "caps\n", 5 );
	if (!strcmp( *argv, "-" )) {
		for (;;) {
			if (isatty( 0 )) {
				fputs( "> ", stdout );
				fflush( stdout );
			}
			if (!fgets( buf, sizeof(buf), stdin ))
				return 0;
			if (exe( fd, buf, strlen( buf ) ))
				return 1;
		}
	} else {
		len = strlen( *argv );
		if (len >= sizeof(buf))
			goto bad;
		memcpy( buf, *argv, len );
		while (*++argv) {
			l = strlen( *argv );
			if (len + l + 1 >= sizeof(buf))
				goto bad;
			buf[len++] = '\t';
			memcpy( buf + len, *argv, l );
			len += l;
		}
		buf[len++] = '\n';
		return exe( fd, buf, len );
	  bad:
		fprintf( stderr, "Command too long\n" );
		return 1;
	}
}

int
main( int argc, char **argv )
{
	char *dpy = getenv( "DISPLAY" );
	const char *ctl = getenv( "DM_CONTROL" );
	const char *cfg = KDE_CONFDIR "/kdm/kdmrc";
	char *ptr;
	int fd;

	(void)argc;
	while (*++argv) {
		ptr = *argv;
		if (*ptr != '-' || !*(ptr + 1))
			break;
		ptr++;
		if (*ptr == '-')
			ptr++;
		if (!strcmp( ptr, "h" ) || !strcmp( ptr, "help" )) {
			puts(
"Usage: kdmctl [options] [command [command arguments]]\n"
"\n"
"Options are:\n"
" -h -help     This help message.\n"
" -g -global   Use global control socket even if $DISPLAY is set\n"
" -d -display  Override $DISPLAY\n"
" -s -sockets  Override $DM_CONTROL\n"
" -c -config   Use alternative kdm config file\n"
"\n"
"The directory in which the sockets are located is determined this way:\n"
"- the -s option is examined\n"
"- the $DM_CONTROL variable is examined\n"
"- the kdm config file is searched for the FifoDir key\n"
"- /var/run/xdmctl and /var/run are tried\n"
"\n"
"If $DISPLAY is set (or -d was specified) and -g was not specified, the\n"
"display-specific control socket will be used, otherwise the global one.\n"
"\n"
"Tokens in the command and the reply are tab-separated.\n"
"Command arguments can be specified as separate command line parameters,\n"
"in which case they are simply concatenated with tabs in between.\n"
"\n"
"If the command is '-', kdmctl reads commands from stdin.\n"
"The default command is 'caps'.\n"
			);
			return 0;
		} else if (!strcmp( ptr, "g" ) || !strcmp( ptr, "global" ))
			dpy = 0;
		else if (!strcmp( ptr, "d" ) || !strcmp( ptr, "display" )) {
			if (!argv[1])
				goto needarg;
			dpy = *++argv;
		} else if (!strcmp( ptr, "s" ) || !strcmp( ptr, "sockets" )) {
			if (!argv[1])
				goto needarg;
			ctl = *++argv;
		} else if (!strcmp( ptr, "c" ) || !strcmp( ptr, "config" )) {
			if (!argv[1]) {
			  needarg:
				fprintf( stderr, "Option '%s' needs argument.\n",
				         ptr );
				return 1;
			}
			cfg = *++argv;
		} else {
			fprintf( stderr, "Unknown option '%s'.\n", ptr );
			return 1;
		}
	}
	if ((!ctl || !*ctl) && *cfg)
		ctl = readcfg( cfg );
	if ((fd = socket( PF_UNIX, SOCK_STREAM, 0 )) < 0) {
		fprintf( stderr, "Cannot create UNIX socket\n" );
		return 1;
	}
	if (dpy && (ptr = strchr( dpy, ':' )) && (ptr = strchr( ptr, '.' )))
		*ptr = 0;
	if (ctl && *ctl) {
		if (!openctl( fd, 1, ctl, dpy ))
			return 1;
	} else {
		if (!openctl( fd, 0, "/var/run/xdmctl", dpy ) &&
		    !openctl( fd, 0, "/var/run", dpy ))
		{
			fprintf( stderr, "No command socket found.\n" );
			return 1;
		}
	}
	return run( fd, argv );
}
