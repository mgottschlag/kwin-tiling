#ifndef classDLL_included
#define classDLL_included

#include <dlfcn.h>
#include <string.h>

typedef void	(*HCBDPROC)( void * );
typedef void	*HCBDDLL;

class classDLL
{

public:
    classDLL( char *szFileName );
    ~classDLL();

    int Symbol( HCBDPROC *hProc, char *szSymbol );
    char szError[501];

private:
	HCBDDLL	hDLL;

};
#endif

