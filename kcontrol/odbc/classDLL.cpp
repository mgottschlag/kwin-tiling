/**************************************************
 * 
 *
 **************************************************
 * This code was created by Peter Harvey @ CodeByDesign. 
 * Released under GPL 31.JAN.99
 *
 * Contributions from...
 * -----------------------------------------------
 * Peter Harvey		- pharvey@codebydesign.com
 **************************************************/
#include "classDLL.h"

classDLL::classDLL( char *szFileName )
{
	szError[0] = '\0';
	hDLL = dlopen( szFileName, RTLD_LAZY );
	if ( !hDLL )
		strncpy( szError, dlerror(), 200 );
}


classDLL::~classDLL()
{
	if ( hDLL )
		dlclose( hDLL );
}

int classDLL::Symbol( HCBDPROC *hProc, char *szSymbol )
{
	char 		*pError;

	*hProc = dlsym( hDLL, szSymbol );

	pError = dlerror();
	if ( pError )
	{
		strncpy( szError, pError, 200 );
		return 0;
	}

	return 1;
}
