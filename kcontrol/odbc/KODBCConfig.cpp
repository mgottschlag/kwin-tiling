#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kwm.h>
#include <dcopclient.h>

#include "KODBCConfig.h"


extern "C" 
{
    KCModule *create_odbc( QWidget *pParent, const char *pszName ) 
    {
	return new KODBCConfig( pParent, pszName );
    }
}



KODBCConfig::KODBCConfig( QWidget *pParent, const char *pszName )
	: KCModule( pParent, pszName )
{
	pTabs	= new QTabWidget( this );

	pTabs->addTab( pUserDSN = new classUserDSN( pTabs, "UserDSN" ), i18n( "&User DSN" ) );
	pTabs->addTab( pSystemDSN = new classSystemDSN( pTabs, "SystemDSN" ), i18n( "&System DSN" ) );
	pTabs->addTab( pDrivers = new classDrivers( pTabs, "Drivers" ), i18n( "&Drivers" ) );
	pTabs->addTab( pTracing = new classTracing( pTabs, "Tracing" ), i18n( "&Tracing" ) );
	pTabs->addTab( pAbout = new classAbout( pTabs, "About" ), i18n( "&About" ) );

	pTabs->resize( 400, 325 );

	setButtons( 0 );
//	setButtons( KCModule::Ok | KCModule::Apply );
}

void KODBCConfig::load()
{
	emit changed( false );
}

void KODBCConfig::save()
{
	pTracing->apply();
	emit changed( false );
}

void KODBCConfig::defaults()
{
	emit changed( true );
}


