#include <kmessagebox.h>

#include "classDSNList.h"
#include "classDSNList.moc"

classDSNList::classDSNList( QWidget* parent, const char* name )
	: QListView( parent, name )
{
	resize( 310,230 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
	setFocusPolicy( QWidget::TabFocus );
	setBackgroundMode( QWidget::PaletteBackground );
	setFontPropagation( QWidget::NoChildren );
	setPalettePropagation( QWidget::NoChildren );
	setFrameStyle( 17 );
	setResizePolicy( QScrollView::Manual );
	setVScrollBarMode( QScrollView::Auto );
	setHScrollBarMode( QScrollView::Auto );
	setTreeStepSize( 20 );
	setMultiSelection( FALSE );
	setAllColumnsShowFocus( TRUE );
	setItemMargin( 1 );
	setRootIsDecorated( FALSE );
	addColumn( "Name", -1 );
	setColumnWidthMode( 0, QListView::Maximum );
	setColumnAlignment( 0, 1 );
	addColumn( "Description", -1 );
	setColumnWidthMode( 1, QListView::Maximum );
	setColumnAlignment( 1, 1 );
	addColumn( "Driver", -1 );
	setColumnWidthMode( 2, QListView::Maximum );
	setColumnAlignment( 2, 1 );

}


classDSNList::~classDSNList()
{
}

void classDSNList::Load( int nSource )
{
	QListViewItem	*pListViewItem;
	QString			qsError;
	DWORD			nErrorCode;
	char   			szErrorMsg[101];
	
	char			szINI[FILENAME_MAX+1];
	char			szSectionNames[4096];
	char			szSectionName[INI_MAX_OBJECT_NAME+1];
	char			szDriver[INI_MAX_PROPERTY_VALUE+1];
	char			szDescription[INI_MAX_PROPERTY_VALUE+1];
	int				nElement;
	
	clear();
	this->nSource = nSource;

	// GET SECTION NAMES (Data Sources)
	strcpy( szINI, ".odbc.ini" );
	memset( szSectionNames, 0, sizeof(szSectionNames) );
	SQLSetConfigMode( nSource );
	if ( SQLGetPrivateProfileString( NULL, NULL, NULL, szSectionNames, 4090, szINI ) >= 0 )
	{
		for ( nElement = 0; iniElement( szSectionNames, '\0', '\0', nElement, szSectionName, INI_MAX_OBJECT_NAME ) == INI_SUCCESS ; nElement++ )
		{
			// GET DRIVER AND DESCRIPTION
			szDriver[0]			= '\0';
			szDescription[0]	= '\0';
			SQLGetPrivateProfileString( szSectionName, "Driver", "", szDriver, INI_MAX_PROPERTY_VALUE, szINI );
			SQLGetPrivateProfileString( szSectionName, "Description", "", szDescription, INI_MAX_PROPERTY_VALUE, szINI );
			pListViewItem = new QListViewItem( this, szSectionName, szDescription, szDriver );
		}
		SQLSetConfigMode( ODBC_BOTH_DSN );
	}
	else
	{
		SQLSetConfigMode( ODBC_BOTH_DSN );
		qsError.sprintf( "Could not load (%s) This is most likely because of a lack of privs.\nTry running as root user the first time you run this program,\nwhen you are working with System DSN's\nor when you are adding/removing Drivers.", szINI );
		KMessageBox::information(this, qsError );

		while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, 100, NULL ) == SQL_SUCCESS )
			KMessageBox::information( this, szErrorMsg );
	}
}

void classDSNList::Add()
{
	// odbc.ini INFO
	QString				qsDataSourceName		= "";
	QString				qsDataSourceDescription	= "";
	QString				qsDataSourceDriver		= "";
	// odbcinst.ini INFO
	QString				qsDriverName			= "";
	QString				qsDriverDescription		= "";
	QString				qsDriverFile			= "";
	QString				qsSetupFile				= "";
	QString				qsError					= "";
	DWORD				nErrorCode;
	char				szErrorMsg[101];

	classDriverPrompt	*pDriverPrompt;
	classProperties		*pProperties;
	HODBCINSTPROPERTY	hFirstProperty	= NULL;
	HODBCINSTPROPERTY	hCurProperty	= NULL;
	char				szINI[FILENAME_MAX+1];

	pDriverPrompt = new classDriverPrompt( this, "DriverPrompt", TRUE );
	if ( pDriverPrompt->exec() )
	{
		qsDriverName		= pDriverPrompt->qsDriverName;
		qsDriverDescription		= pDriverPrompt->qsDescription;
		qsDriverFile		= pDriverPrompt->qsDriver;
		qsSetupFile			= pDriverPrompt->qsSetup;
		qsDataSourceDriver	= qsDriverName;
		delete pDriverPrompt;

		// GET PROPERTY LIST FROM DRIVER
		if ( ODBCINSTConstructProperties( (char*)qsDataSourceDriver.data(), &hFirstProperty ) != ODBCINST_SUCCESS )
		{
			qsError.sprintf( "Could not construct a property list for (%s)... most often because the driver has not been given a valid setup lib. Each driver has a setup lib such as libodbcminiS.so.1.0.0", qsDataSourceDriver.data() );
			KMessageBox::information( this, qsError );
			KMessageBox::information( this, "Please specify a Setup lib in the Drivers section. Use libodbcdrvcfg1S.so or libodbcdrvcfg2S.so if you are unsure of what setup lib to use." );
			return;
		}

		// ALLOW USER TO EDIT
		pProperties = new classProperties( this, "Properties", hFirstProperty );
        pProperties->setCaption( "New Data Source" );
        pProperties->resize( 200, 400 );
		if ( pProperties->exec() )
		{
			/* DELETE ENTIRE SECTION IF IT EXISTS (no entry given) */
			SQLSetConfigMode( nSource );
			if ( SQLWritePrivateProfileString( hFirstProperty->szValue, NULL, NULL, szINI ) == FALSE )
			{
				SQLSetConfigMode( ODBC_BOTH_DSN );
				delete pProperties;
				ODBCINSTDestructProperties( &hFirstProperty );

				qsError.sprintf( "Could not write to (%s), try running this program as root.", szINI );
				KMessageBox::information( this, qsError );
				while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, 100, NULL ) == SQL_SUCCESS )
					KMessageBox::information( this, szErrorMsg );

				return;
			}
			qsDataSourceName = hFirstProperty->szValue;
			/* ADD ENTRIES; SECTION CREATED ON FIRST CALL */
			for ( hCurProperty = hFirstProperty->pNext; hCurProperty != NULL; hCurProperty = hCurProperty->pNext )
			{
				if ( strncasecmp( hCurProperty->szName, "Description", INI_MAX_PROPERTY_NAME ) == 0 )
					qsDataSourceDescription = hCurProperty->szValue;

				SQLWritePrivateProfileString( hFirstProperty->szValue, hCurProperty->szName, hCurProperty->szValue, szINI );
			}
			SQLSetConfigMode( ODBC_BOTH_DSN );
		}
		delete pProperties;
		ODBCINSTDestructProperties( &hFirstProperty );
	}
	else
		delete pDriverPrompt;

	// RELOAD (slow but safe)
	Load( nSource );
}

void classDSNList::Edit()
{
	// odbc.ini INFO
	QString				qsDataSourceName		= "";
	QString				qsDataSourceDescription		= "";
	QString				qsDataSourceDriver		= "";
	// odbcinst.ini INFO
	QString				qsDriverFile			= "";
	QString				qsSetupFile				= "";
	QString				qsError					= "";

	classProperties		*pProperties;
	HODBCINSTPROPERTY	hFirstProperty	= NULL;
	HODBCINSTPROPERTY	hCurProperty	= NULL;
	QListViewItem		*pListViewItem;

	char				szEntryNames[4096];
	char				szProperty[INI_MAX_PROPERTY_NAME+1];
	char				szValue[INI_MAX_PROPERTY_VALUE+1];
	
	DWORD				nErrorCode;
	char				szErrorMsg[101];
	char				szINI[FILENAME_MAX+1];
	int					nElement;	

	// HAS THE USER SELECTED SOMETHING
    pListViewItem = currentItem();
	if ( pListViewItem )
	{
		qsDataSourceName		= pListViewItem->text( 0 );
		qsDataSourceDescription	= pListViewItem->text( 1 );
		qsDataSourceDriver		= pListViewItem->text( 2 );
	}
	else
	{
		KMessageBox::information( this, "Please select a Data Source from the list first" );
		return;
	}

	// GET PROPERTY LIST FROM DRIVER
	if ( ODBCINSTConstructProperties( (char*)qsDataSourceDriver.data(), &hFirstProperty ) != ODBCINST_SUCCESS )
	{
		qsError.sprintf( "Could not construct a property list for (%s). Ensure that the Driver has a valid Setup lib specified.", qsDataSourceDriver.data() );
		KMessageBox::information( this, qsError );
		while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, 100, NULL ) == SQL_SUCCESS )
			KMessageBox::information( this, szErrorMsg );

		return;
	}

	// COPY EXISTING VALUES INTO PROPERTIES LIST
	SQLSetConfigMode( nSource );
	ODBCINSTSetProperty( hFirstProperty, "Name", (char*)qsDataSourceName.data() );
	SQLGetPrivateProfileString( qsDataSourceName.data(), NULL, NULL, szEntryNames, 4090, szINI ); // GET ALL ENTRY NAMES FOR THE SELCTED DATA SOURCE
	for ( nElement = 0; iniElement( szEntryNames, '\0', '\0', nElement, szProperty, 1000 ) == INI_SUCCESS ; nElement++ )
	{
		SQLGetPrivateProfileString( qsDataSourceName.data(), szProperty, "", szValue, INI_MAX_PROPERTY_VALUE, szINI ); // GET VALUE FOR EACH ENTRY
		ODBCINSTSetProperty( hFirstProperty, szProperty, szValue );
	}
	SQLSetConfigMode( ODBC_BOTH_DSN );

	// ALLOW USER TO EDIT
	pProperties = new classProperties( this, "Properties", hFirstProperty );
	pProperties->setCaption( "Configure Data Source" );
	pProperties->resize( 200, 400 );
	if ( pProperties->exec() )
	{
		SQLSetConfigMode( nSource );
		/* DELETE ENTIRE SECTION IF IT EXISTS (given NULL entry) */
		if ( SQLWritePrivateProfileString( qsDataSourceName.ascii(), NULL, NULL, szINI ) == FALSE )
		{
			SQLSetConfigMode( ODBC_BOTH_DSN );
			delete pProperties;
			ODBCINSTDestructProperties( &hFirstProperty );

			qsError.sprintf( "Could not write to (%s), you may want to try running this program as root ( ie when dealing with System DSN's.", szINI );
			KMessageBox::information( this, qsError );
			while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, 100, NULL ) == SQL_SUCCESS )
				KMessageBox::information( this, szErrorMsg );

			return;
		}
		qsDataSourceName = hFirstProperty->szValue;
		/* ADD ENTRIES; SECTION CREATED ON FIRST CALL */
		for ( hCurProperty = hFirstProperty->pNext; hCurProperty != NULL; hCurProperty = hCurProperty->pNext )
		{
			if ( strncasecmp( hCurProperty->szName, "Description", INI_MAX_PROPERTY_NAME ) == 0 )
				qsDataSourceDescription = hCurProperty->szValue;

			SQLWritePrivateProfileString( hFirstProperty->szValue, hCurProperty->szName, hCurProperty->szValue, szINI );
		}
		SQLSetConfigMode( ODBC_BOTH_DSN );
	}
	delete pProperties;
	ODBCINSTDestructProperties( &hFirstProperty );

	// RELOAD (slow but safe)
	Load( nSource );
}

void classDSNList::Delete()
{
	QListViewItem		*pListViewItem;
	char 				szINI[FILENAME_MAX+1];
	char 				*pDataSourceName;
	QString				qsError;
	DWORD				nErrorCode;
	char				szErrorMsg[FILENAME_MAX+1];

	// GET SELECT DATA SOURCE NAME
    pListViewItem = currentItem();
	if ( pListViewItem )
	{
		pDataSourceName = (char *)pListViewItem->text( 0 ).ascii();
	}
	else
	{
		KMessageBox::information( this, "Please select a Data Source from the list first" );
		return;
	}

	// DELETE ENTIRE SECTION IF IT EXISTS (given NULL entry)
	SQLSetConfigMode( nSource );
	if ( SQLWritePrivateProfileString( pDataSourceName, NULL, NULL, szINI ) == FALSE )
	{
		qsError.sprintf( "Could not write property list for (%s). Try running this program as root.", pDataSourceName );
		KMessageBox::sorry( this, qsError );
		while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, FILENAME_MAX, NULL ) == SQL_SUCCESS )
			KMessageBox::information( this, szErrorMsg );
	}
	SQLSetConfigMode( ODBC_BOTH_DSN );
	
	// RELOAD (slow but safe)
	Load( nSource );
}


