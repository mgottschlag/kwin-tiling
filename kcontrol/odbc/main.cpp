#include <stdlib.h>

#include <kcontrol.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include "classUserDSN.h"
#include "classSystemDSN.h"
#include "classDrivers.h"
#include "classAbout.h"

class KODBCControlApplication : public KControlApplication
{
public:

	KODBCControlApplication(int &argc, char **arg, const char *name);

	void init();
	void apply();
	void defaultValues();

private:
	classUserDSN    *pUserDSN;
	classSystemDSN  *pSystemDSN;
	classDrivers    *pDrivers;
	classAbout      *pAbout;

};


KODBCControlApplication::KODBCControlApplication( int &argc, char **argv, const char *name )
: KControlApplication( argc, argv, name )
{
	pUserDSN    = 0L;
	pSystemDSN    = 0L;
	pDrivers    = 0L;
	pAbout    = 0L;

	if ( !runGUI() )
		return;

	if ( !pages || pages->contains( "UserDSN" ) )
		addPage( pUserDSN = new classUserDSN( dialog, "UserDSN" ), i18n( "&User DSN" ), "odbc-1.html" );

	if ( !pages || pages->contains( "SystemDSN" ) )
		addPage( pSystemDSN = new classSystemDSN( dialog, "SystemDSN" ), i18n( "&System DSN" ), "odbc-2.html" );

	if ( !pages || pages->contains( "Drivers" ) )
		addPage( pDrivers = new classDrivers( dialog, "Drivers" ), i18n( "&Drivers" ), "odbc-3.html" );

	if ( !pages || pages->contains( "About" ) )
		addPage( pAbout = new classAbout( dialog, "About" ), i18n( "&About" ), "odbc-4.html" );

	if ( pUserDSN || pSystemDSN || pDrivers || pAbout )
		dialog->show();
	else
	{
		fprintf(stderr, i18n("usage: %s [-init | {UserDSN,SystemDSN,Drivers}]\n").ascii(), argv[0] );;
		justInit = true;
	}
}

void KODBCControlApplication::init()
{
	if ( pUserDSN )
		pUserDSN->loadSettings();

	if ( pSystemDSN )
		pSystemDSN->loadSettings();

	if ( pDrivers )
		pDrivers->loadSettings();

	if ( pAbout )
		pAbout->loadSettings();
}

void KODBCControlApplication::defaultValues()
{
	if ( pUserDSN )
		pUserDSN->defaultSettings();

	if ( pSystemDSN )
		pSystemDSN->defaultSettings();

	if ( pDrivers )
		pDrivers->defaultSettings();

	if ( pAbout )
		pAbout->defaultSettings();
}

void KODBCControlApplication::apply()
{
	if ( pUserDSN )
		pUserDSN->applySettings();

	if ( pSystemDSN )
		pSystemDSN->applySettings();

	if ( pDrivers )
		pDrivers->applySettings();

	if ( pAbout )
		pAbout->applySettings();
}

int main(int argc, char **argv )
{
	int ret = 0;
	KODBCControlApplication app(argc, argv, "odbcconfig");

	app.setTitle(i18n("ODBC Administrator"));

	if ( app.runGUI() )
		ret = app.exec();

	return ret;
}


