/*

Copyright 1994, 1998  The Open Group
Copyright 2003 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Stephen Gildea, The Open Group
 *
 * generate Kerberos Version 5 authorization records
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <krb5/krb5.h>

static krb5_context ctx;

/*ARGSUSED*/
void
Krb5InitAuth( unsigned short name_len ATTR_UNUSED, const char *name ATTR_UNUSED )
{
	if (krb5_init_context( &ctx ))
		LogError( "Error while initializing Krb5 context\n" );
}

/*
 * Returns malloc'ed string that is the credentials cache name.
 * name should be freed by caller.
 */
static char *
Krb5CCacheName( const char *dname )
{
	char *name;
	const char *tmpdir;
	int dnl, nl;

	tmpdir = getenv( "TMPDIR" );
	if (!tmpdir)
		tmpdir = "/tmp";
	dnl = strlen( dname );
	name = Malloc( strlen( tmpdir ) + dnl + 20 );
	if (!name)
		return NULL;
	nl = sprintf( name, "FILE:%s/K5C", tmpdir );
	CleanUpFileName( dname, name + nl, dnl + 1 );
	return name;
}

Xauth *
Krb5GetAuthFor( unsigned short namelen, const char *name, const char *dname )
{
	Xauth *new;
	char *filename;

	if (!(new = (Xauth *)Malloc( sizeof(*new) )))
		return (Xauth *)0;
	new->family = FamilyWild;
	new->address_length = 0;
	new->address = 0;
	new->number_length = 0;
	new->number = 0;

	if (dname) {
		if (!(filename = Krb5CCacheName( dname ))) {
			free( (char *)new );
			return (Xauth *)0;
		}
		new->data = 0;
		if (!StrApp( &new->data, "UU:", filename, (char *)0 )) {
			free( filename );
			free( (char *)new );
			return (Xauth *)0;
		}
		free( filename );
		new->data_length = strlen( new->data );
	} else {
		new->data = NULL;
		new->data_length = 0;
	}

	if (!(new->name = (char *)Malloc( namelen ))) {
		free( (char *)new->data );
		free( (char *)new );
		return (Xauth *)0;
	}
	memmove( new->name, name, namelen );
	new->name_length = namelen;
	return new;
}


Xauth *
Krb5GetAuth( unsigned short namelen, const char *name )
{
	return Krb5GetAuthFor( namelen, name, NULL );
}


static krb5_error_code
Krb5DisplayCCache( const char *dname, krb5_ccache *ccache_return, char **name )
{
	char *ccname;
	krb5_error_code code;

	if (!(ccname = Krb5CCacheName( dname )))
		return ENOMEM;
	Debug( "resolving Kerberos cache %s\n", ccname );
	if ((code = krb5_cc_resolve( ctx, ccname, ccache_return )) || !name)
		free( ccname );
	else
		*name = ccname;
	return code;
}

char *
Krb5Init( const char *user, const char *passwd, const char *dname )
{
	krb5_error_code code;
	krb5_get_init_creds_opt options;
	krb5_principal me;
	krb5_creds my_creds;
	krb5_ccache ccache;
	char *ccname;

	if (!ctx)
		return 0;

	if ((code = krb5_parse_name( ctx, user, &me ))) {
		LogError( "%s while parsing Krb5 user %\"s\n",
		          error_message( code ), user );
		return 0;
	}

	krb5_get_init_creds_opt_init( &options );
	/*krb5_get_init_creds_opt_set_tkt_life (&options, 60*60*8);*/ /* 8 hours */

	if ((code = krb5_get_init_creds_password( ctx, &my_creds,
	                                          me, /* principal */
	                                          (char * /* for MIT */) passwd,
	                                          0, /* prompter */
	                                          0, /* prompter ctx */
	                                          0, /* start time delta */
	                                          0, /* service */
	                                          &options )))
	{
		char *my_name = NULL;
		int code2 = krb5_unparse_name( ctx, me, &my_name );
		if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY)
			LogError( "Password incorrect for Krb5 principal %\"s\n",
			          code2 ? user : my_name );
		else
			LogError( "%s while getting initial Krb5 credentials for %\"s\n",
			          error_message( code ), code2 ? user : my_name );
		if (my_name)
			free( my_name );
		goto err3;
	}

	if ((code = Krb5DisplayCCache( dname, &ccache, &ccname ))) {
		LogError( "%s while getting Krb5 ccache for %\"s\n",
		          error_message( code ), dname );
		goto err2;
	}

	if ((code = krb5_cc_initialize( ctx, ccache, me ))) {
		LogError( "%s while initializing Krb5 cache %\"s\n",
		          error_message( code ), ccname );
		goto err1;
	}

	if ((code = krb5_cc_store_cred( ctx, ccache, &my_creds ))) {
		LogError( "%s while storing Krb5 credentials to cache %\"s\n",
		          error_message( code ), ccname );
	  err1:
		krb5_cc_close( ctx, ccache );
		free( ccname );
	  err2:
		krb5_free_cred_contents( ctx, &my_creds );
	  err3:
		krb5_free_principal( ctx, me );
		return 0;
	}

	krb5_cc_close( ctx, ccache );
	krb5_free_cred_contents( ctx, &my_creds );
	krb5_free_principal( ctx, me );
	return ccname;
}

void
Krb5Destroy( const char *dname )
{
	krb5_error_code code;
	krb5_ccache ccache;

	if (!ctx)
		return;

	if ((code = Krb5DisplayCCache( dname, &ccache, 0 )))
		LogError( "%s while getting Krb5 ccache to destroy\n",
		          error_message( code ) );
	else {
		if ((code = krb5_cc_destroy( ctx, ccache ))) {
			if (code == KRB5_FCC_NOFILE)
				Debug( "no Kerberos ccache file found to destroy\n" );
			else
				LogError( "%s while destroying Krb5 credentials cache\n",
				          error_message( code ) );
		} else
			Debug( "kerberos ccache destroyed\n" );
	}
}
