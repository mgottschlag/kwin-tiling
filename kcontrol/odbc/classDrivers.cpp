#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>

#include <kmessagebox.h>
#include <kstddirs.h>

#include "classDrivers.h"
#include "classDrivers.moc"

classDrivers::classDrivers( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	pbAdd = new QPushButton( this, "pbAdd" );
	pbAdd->setGeometry( 290, 10, 100, 30 );
	pbAdd->setMinimumSize( 0, 0 );
	pbAdd->setMaximumSize( 32767, 32767 );
	pbAdd->setFocusPolicy( QWidget::TabFocus );
	pbAdd->setBackgroundMode( QWidget::PaletteBackground );
	pbAdd->setFontPropagation( QWidget::NoChildren );
	pbAdd->setPalettePropagation( QWidget::NoChildren );
	pbAdd->setText( "A&dd..." );
	pbAdd->setAutoRepeat( FALSE );
	pbAdd->setAutoResize( FALSE );

	pbRemove = new QPushButton( this, "pbRemove" );
	pbRemove->setGeometry( 290, 50, 100, 30 );
	pbRemove->setMinimumSize( 0, 0 );
	pbRemove->setMaximumSize( 32767, 32767 );
	pbRemove->setFocusPolicy( QWidget::TabFocus );
	pbRemove->setBackgroundMode( QWidget::PaletteBackground );
	pbRemove->setFontPropagation( QWidget::NoChildren );
	pbRemove->setPalettePropagation( QWidget::NoChildren );
	pbRemove->setText( "&Remove" );
	pbRemove->setAutoRepeat( FALSE );
	pbRemove->setAutoResize( FALSE );

	pbConfigure = new QPushButton( this, "pbConfigure" );
	pbConfigure->setGeometry( 290, 90, 100, 30 );
	pbConfigure->setMinimumSize( 0, 0 );
	pbConfigure->setMaximumSize( 32767, 32767 );
	pbConfigure->setFocusPolicy( QWidget::TabFocus );
	pbConfigure->setBackgroundMode( QWidget::PaletteBackground );
	pbConfigure->setFontPropagation( QWidget::NoChildren );
	pbConfigure->setPalettePropagation( QWidget::NoChildren );
	pbConfigure->setText( "&Configure..." );
	pbConfigure->setAutoRepeat( FALSE );
	pbConfigure->setAutoResize( FALSE );

	lvwDrivers = new QListView( this, "lvwDrivers" );
	lvwDrivers->setGeometry( 10, 10, 270, 190 );
	lvwDrivers->setMinimumSize( 0, 0 );
	lvwDrivers->setMaximumSize( 32767, 32767 );
	lvwDrivers->setFocusPolicy( QWidget::TabFocus );
	lvwDrivers->setBackgroundMode( QWidget::PaletteBackground );
	lvwDrivers->setFontPropagation( QWidget::NoChildren );
	lvwDrivers->setPalettePropagation( QWidget::NoChildren );
	lvwDrivers->setFrameStyle( 17 );
	lvwDrivers->setResizePolicy( QScrollView::Manual );
	lvwDrivers->setVScrollBarMode( QScrollView::Auto );
	lvwDrivers->setHScrollBarMode( QScrollView::Auto );
	lvwDrivers->setTreeStepSize( 20 );
	lvwDrivers->setMultiSelection( FALSE );
	lvwDrivers->setAllColumnsShowFocus( FALSE );
	lvwDrivers->setItemMargin( 1 );
	lvwDrivers->setRootIsDecorated( FALSE );
	lvwDrivers->addColumn( "Name", -1 );
	lvwDrivers->setColumnWidthMode( 0, QListView::Maximum );
	lvwDrivers->setColumnAlignment( 0, 1 );
	lvwDrivers->addColumn( "Description", -1 );
	lvwDrivers->setColumnWidthMode( 1, QListView::Maximum );
	lvwDrivers->setColumnAlignment( 1, 1 );
	lvwDrivers->addColumn( "Driver Lib", -1 );
	lvwDrivers->setColumnWidthMode( 2, QListView::Maximum );
	lvwDrivers->setColumnAlignment( 2, 1 );
	lvwDrivers->addColumn( "Setup Lib", -1 );
	lvwDrivers->setColumnWidthMode( 3, QListView::Maximum );
	lvwDrivers->setColumnAlignment( 3, 1 );

	QFrame* qtarch_Frame_2;
	qtarch_Frame_2 = new QFrame( this, "Frame_2" );
	qtarch_Frame_2->setGeometry( 10, 204, 380, 90 );
	qtarch_Frame_2->setMinimumSize( 0, 0 );
	qtarch_Frame_2->setMaximumSize( 32767, 32767 );
	qtarch_Frame_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Frame_2->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Frame_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Frame_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Frame_2->setFrameStyle( 33 );

	QLabel* qtarch_Label_1;
	qtarch_Label_1 = new QLabel( this, "Label_1" );
	qtarch_Label_1->setGeometry( 70, 210, 310, 70 );
	qtarch_Label_1->setMinimumSize( 0, 0 );
	qtarch_Label_1->setMaximumSize( 32767, 32767 );
	qtarch_Label_1->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_1->setBackgroundMode( QWidget::PaletteBackground );
	qtarch_Label_1->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_1->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_1->setText( "These drivers facilitate communication between the Driver Manager and the data server. Many ODBC drivers  for Linux can be downloaded from the Internet while others are obtained from your database vendor." );
	qtarch_Label_1->setAlignment( 1313 );
	qtarch_Label_1->setMargin( -1 );

	QLabel* qtarch_Label_2;
	qtarch_Label_2 = new QLabel( this, "Label_2" );
	qtarch_Label_2->setGeometry( 30, 230, 32, 32 );
	qtarch_Label_2->setMinimumSize( 32, 32 );
	qtarch_Label_2->setMaximumSize( 32, 32 );
	qtarch_Label_2->setBackgroundPixmap( QPixmap( locate("icon", "driver.png") ) );
	qtarch_Label_2->setFocusPolicy( QWidget::NoFocus );
	qtarch_Label_2->setFontPropagation( QWidget::NoChildren );
	qtarch_Label_2->setPalettePropagation( QWidget::NoChildren );
	qtarch_Label_2->setText( "" );
	qtarch_Label_2->setAlignment( 289 );
	qtarch_Label_2->setMargin( -1 );

	resize( 400,300 );
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );

	lvwDrivers->setAllColumnsShowFocus( true );

	connect( pbAdd, SIGNAL(clicked()), this, SLOT(Add()) );
	connect( pbRemove, SIGNAL(clicked()), this, SLOT(Delete()) );
	connect( pbConfigure, SIGNAL(clicked()), this, SLOT(Edit()) );

	Load();
}


classDrivers::~classDrivers()
{
	if ( hIni != NULL )
		iniClose( hIni );
	
}


void classDrivers::Add()
{
	QString				qsError					= "";

	classProperties		*pProperties;
	HODBCINSTPROPERTY	hFirstProperty	= NULL;
	HODBCINSTPROPERTY	hCurProperty	= NULL;
	HODBCINSTPROPERTY	hLastProperty;
	char				szINI[FILENAME_MAX+1];

	strcpy( szINI, "/etc/odbcinst.ini" );

	// SET UP PROPERTIES LIST
	hFirstProperty 						= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	memset( hFirstProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hFirstProperty->nPromptType			= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hFirstProperty->pNext				= NULL;
    hFirstProperty->bRefresh			= 0;
    hFirstProperty->hDLL				= NULL;
    hFirstProperty->pWidget				= NULL;
    hFirstProperty->pszHelp				= NULL;
	hFirstProperty->aPromptData			= NULL;
	strncpy( hFirstProperty->szName, "Name", INI_MAX_PROPERTY_NAME );
	strcpy( hFirstProperty->szValue, "" );
	hLastProperty = hFirstProperty;

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType				= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hLastProperty->pNext					= NULL;
    hLastProperty->bRefresh					= 0;
    hLastProperty->hDLL						= NULL;
    hLastProperty->pWidget					= NULL;
    hFirstProperty->pszHelp					= NULL;
	hFirstProperty->aPromptData				= NULL;
	strncpy( hLastProperty->szName, "Description", INI_MAX_PROPERTY_NAME );
	strcpy( hFirstProperty->szValue, "" );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType			= ODBCINST_PROMPTTYPE_FILENAME;
	strncpy( hLastProperty->szName, "Driver", INI_MAX_PROPERTY_NAME );
	strncpy( hLastProperty->szValue, "", INI_MAX_PROPERTY_VALUE );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType			= ODBCINST_PROMPTTYPE_FILENAME;
	strncpy( hLastProperty->szName, "Setup", INI_MAX_PROPERTY_NAME );
	strncpy( hLastProperty->szValue, "libodbcdrvcfg1S.so", INI_MAX_PROPERTY_VALUE );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType				= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hLastProperty->pNext					= NULL;
    hLastProperty->bRefresh					= 0;
    hLastProperty->hDLL						= NULL;
    hLastProperty->pWidget					= NULL;
    hFirstProperty->pszHelp					= NULL;
	hFirstProperty->aPromptData				= NULL;
	strncpy( hLastProperty->szName, "FileUsage", INI_MAX_PROPERTY_NAME );
	strcpy( hLastProperty->szValue, "1" );


	// ALLOW USER TO EDIT
	pProperties = new classProperties( this, "Properties", hFirstProperty );
	pProperties->setCaption( "New Driver" );
	pProperties->resize( 200, 400 );
	if ( pProperties->exec() )
	{
		/* DELETE ENTIRE SECTION IF IT EXISTS (no entry given) */
		if ( SQLWritePrivateProfileString( hFirstProperty->szValue, NULL, NULL, szINI ) == FALSE )
		{
			delete pProperties;
			FreeProperties( &hFirstProperty );
			qsError.sprintf( "Could not write to (%s). Try running this program as root when working with Drivers or System DSN's.", szINI );
			KMessageBox::information( this, qsError );
			return;
		}

		/* ADD ENTRIES; SECTION CREATED ON FIRST CALL */
		for ( hCurProperty = hFirstProperty->pNext; hCurProperty != NULL; hCurProperty = hCurProperty->pNext )
		{
			SQLWritePrivateProfileString( hFirstProperty->szValue, hCurProperty->szName, hCurProperty->szValue, szINI );
		}
	}
	delete pProperties;
	FreeProperties( &hFirstProperty );

	// RELOAD (slow but safe)
	Load();
}

void classDrivers::Edit()
{
	QString				qsName					= "";
	QString				qsError					= "";

	classProperties		*pProperties;
	HODBCINSTPROPERTY	hFirstProperty	= NULL;
	HODBCINSTPROPERTY	hCurProperty	= NULL;
	HODBCINSTPROPERTY	hLastProperty;
	char				szINI[FILENAME_MAX+1];
	QListViewItem		*pListViewItem;

#ifdef SYSTEM_FILE_PATH
    sprintf( szINI, "%s/odbcinst.ini", SYSTEM_FILE_PATH );
#else
    strcpy( szINI, "/etc/odbcinst.ini" );
#endif

	// HAS THE USER SELECTED SOMETHING
    pListViewItem = lvwDrivers->currentItem();
	if ( pListViewItem )
		qsName		= pListViewItem->text( 0 );
	else
	{
		KMessageBox::information( this, "Please select a Driver from the list first." );
		return;
	}


	// SET UP PROPERTIES LIST
	hFirstProperty 						= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	memset( hFirstProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hFirstProperty->nPromptType			= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hFirstProperty->pNext				= NULL;
    hFirstProperty->bRefresh			= 0;
    hFirstProperty->hDLL				= NULL;
    hFirstProperty->pWidget				= NULL;
    hFirstProperty->pszHelp				= NULL;
	hFirstProperty->aPromptData			= NULL;
	strncpy( hFirstProperty->szName, "Name", INI_MAX_PROPERTY_NAME );
	strcpy( hFirstProperty->szValue, qsName.data() );
	hLastProperty = hFirstProperty;

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType				= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hLastProperty->pNext					= NULL;
    hLastProperty->bRefresh					= 0;
    hLastProperty->hDLL						= NULL;
    hLastProperty->pWidget					= NULL;
    hFirstProperty->pszHelp					= NULL;
	hFirstProperty->aPromptData				= NULL;
	strncpy( hLastProperty->szName, "Description", INI_MAX_PROPERTY_NAME );
	strcpy( hLastProperty->szValue, "" );
	SQLGetPrivateProfileString( qsName.data(), hLastProperty->szName, "", hLastProperty->szValue, sizeof(hLastProperty->szValue)-1, szINI );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType			= ODBCINST_PROMPTTYPE_FILENAME;
	strncpy( hLastProperty->szName, "Driver", INI_MAX_PROPERTY_NAME );
	strncpy( hLastProperty->szValue, "", INI_MAX_PROPERTY_VALUE );
	SQLGetPrivateProfileString( qsName.data(), hLastProperty->szName, "", hLastProperty->szValue, sizeof(hLastProperty->szValue)-1, szINI );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType			= ODBCINST_PROMPTTYPE_FILENAME;
	strncpy( hLastProperty->szName, "Setup", INI_MAX_PROPERTY_NAME );
	strncpy( hLastProperty->szValue, "", INI_MAX_PROPERTY_VALUE );
	SQLGetPrivateProfileString( qsName.data(), hLastProperty->szName, "", hLastProperty->szValue, sizeof(hLastProperty->szValue)-1, szINI );

	hLastProperty->pNext 				= (HODBCINSTPROPERTY)malloc( sizeof(ODBCINSTPROPERTY) );
	hLastProperty 						= hLastProperty->pNext;
	memset( hLastProperty, 0, sizeof(ODBCINSTPROPERTY) );
	hLastProperty->nPromptType				= ODBCINST_PROMPTTYPE_TEXTEDIT;
	hLastProperty->pNext					= NULL;
    hLastProperty->bRefresh					= 0;
    hLastProperty->hDLL						= NULL;
    hLastProperty->pWidget					= NULL;
    hFirstProperty->pszHelp					= NULL;
	hFirstProperty->aPromptData				= NULL;
	strncpy( hLastProperty->szName, "FileUsage", INI_MAX_PROPERTY_NAME );
	strcpy( hLastProperty->szValue, "1" );
	SQLGetPrivateProfileString( qsName.data(), hLastProperty->szName, "", hLastProperty->szValue, sizeof(hLastProperty->szValue)-1, szINI );


	// ALLOW USER TO EDIT
	pProperties = new classProperties( this, "Properties", hFirstProperty );
	pProperties->setCaption( "New Driver" );
	pProperties->resize( 200, 400 );
	if ( pProperties->exec() )
	{
		/* DELETE ENTIRE SECTION IF IT EXISTS (no entry given) */
		if ( SQLWritePrivateProfileString( qsName.ascii(), NULL, NULL, szINI ) == FALSE )
		{
			delete pProperties;
			FreeProperties( &hFirstProperty );
			qsError.sprintf( "Could not write to (%s)", szINI );
			KMessageBox::information( this, qsError );
			return;
		}

		/* ADD ENTRIES; SECTION CREATED ON FIRST CALL */
		for ( hCurProperty = hFirstProperty->pNext; hCurProperty != NULL; hCurProperty = hCurProperty->pNext )
		{
			SQLWritePrivateProfileString( hFirstProperty->szValue, hCurProperty->szName, hCurProperty->szValue, szINI );
		}
	}
	delete pProperties;
	FreeProperties( &hFirstProperty );

	// RELOAD (slow but safe)
	Load();
}

void classDrivers::Delete()
{
	QListViewItem		*pListViewItem;
	char 				szINI[FILENAME_MAX+1];
	char 				*pszName;
	QString				qsError;
	DWORD				nErrorCode;
	char				szErrorMsg[FILENAME_MAX+1];

	strcpy( szINI, "/etc/odbcinst.ini" );

	// GET SELECT DATA SOURCE NAME
    pListViewItem = lvwDrivers->currentItem();
	if ( pListViewItem )
	{
		pszName = (char *)pListViewItem->text( 0 ).ascii();
	}
	else
	{
		KMessageBox::information( this, "Please select a Driver from the list first" );
		return;
	}

	// DELETE ENTIRE SECTION IF IT EXISTS (given NULL entry)
	if ( SQLWritePrivateProfileString( pszName, NULL, NULL, szINI ) == FALSE )
	{
		qsError.sprintf( "Could not write property list for (%s)", pszName );
		KMessageBox::information( this, qsError );
		//while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, FILENAME_MAX, NULL ) == SQL_SUCCESS )
		//	KMessageBox::information( this, szErrorMsg );
	}
	
	// RELOAD (slow but safe)
	Load();
}



void classDrivers::Load()
{
    lvwDrivers->clear();

	/***************************************
	 * METHOD 1 - Using Driver Manager
	 ***************************************/
	
			
	/***************************************
	 * METHOD 2 - Using ODBCINST lib (also see SQLGetInstalledDrivers)
	 ***************************************/
	/*
	QListViewItem	*pListViewItem;
	QString			qsError;
	DWORD			nErrorCode;
	char   			szErrorMsg[101];

	char			szINI[FILENAME_MAX+1];
	char			szSectionNames[4096];
	char			szSectionName[INI_MAX_OBJECT_NAME+1];
	int				nElement;
	
	char 			szDriverName[INI_MAX_OBJECT_NAME+1];
	char 			szDescription[INI_MAX_PROPERTY_VALUE+1];
	char 			szDriver[INI_MAX_PROPERTY_VALUE+1];
	char 			szSetup[INI_MAX_PROPERTY_VALUE+1];

	strcpy( szINI, "/etc/odbcinst.ini" );

	memset( szSectionNames, 0, sizeof(szSectionNames) );
	if ( SQLGetPrivateProfileString( NULL, NULL, NULL, szSectionNames, 4090, szINI ) >= 0 )
	{
		for ( nElement = 0; iniElement( szSectionNames, '\0', '\0', nElement, szSectionName, INI_MAX_OBJECT_NAME ) == INI_SUCCESS ; nElement++ )
		{
			szDriverName[0]		= '\0';
			szDescription[0]	= '\0';
			szDriver[0]			= '\0';
			szSetup[0]			= '\0';
			SQLGetPrivateProfileString( szSectionName, "Driver", "", szDriverName, INI_MAX_PROPERTY_VALUE, szINI );
			SQLGetPrivateProfileString( szSectionName, "Description", "", szDescription, INI_MAX_PROPERTY_VALUE, szINI );
			SQLGetPrivateProfileString( szSectionName, "Driver", "", szDriver, INI_MAX_PROPERTY_VALUE, szINI );
			SQLGetPrivateProfileString( szSectionName, "Setup", "", szSetup, INI_MAX_PROPERTY_VALUE, szINI );
			pListViewItem = new QListViewItem( lvwDrivers, szDriverName, szDescription, szDriver, szSetup );
		}
	}
	else
	{
		qsError.sprintf( "Could not load %s", szINI );
		KMessageBox::information(	this, "ODBC Config",  qsError );
		while ( SQLInstallerError( 1, &nErrorCode, szErrorMsg, 100, NULL ) == SQL_SUCCESS )
			KMessageBox::information( this, "ODBC Config",  szErrorMsg );
	}
	*/

	/***************************************
	 * METHOD 3 - Using INI lib
	 ***************************************/
	char 			szDriverName[INI_MAX_OBJECT_NAME+1];
	char			szPropertyName[INI_MAX_PROPERTY_NAME+1];
	char 			szDescription[INI_MAX_PROPERTY_VALUE+1];
	char 			szDriver[INI_MAX_PROPERTY_VALUE+1];
	char 			szSetup[INI_MAX_PROPERTY_VALUE+1];
	QListViewItem	*pListViewItem;
	QString			qsError;

    strcpy( szINI, "/etc/odbcinst.ini" );

	if ( iniOpen( &hIni, szINI, '#', '[', ']', '=', TRUE ) != INI_ERROR )
	{
		iniObjectFirst( hIni );
		while ( iniObjectEOL( hIni ) == FALSE )
		{
			szDriverName[0] = '\0';
			szDescription[0] 	= '\0';
			szDriver[0] 	= '\0';
			szSetup[0] 		= '\0';
			iniObject( hIni, szDriverName );
			iniPropertyFirst( hIni );
			while ( iniPropertyEOL( hIni ) == FALSE )
			{
				iniProperty( hIni, szPropertyName );
				iniToUpper( szPropertyName );

				if ( strncmp( szPropertyName, "DESCRIPTION", INI_MAX_PROPERTY_NAME ) == 0 )
					iniValue( hIni, szDescription );
				if ( strncmp( szPropertyName, "DRIVER", INI_MAX_PROPERTY_NAME ) == 0 )
					iniValue( hIni, szDriver );
				if ( strncmp( szPropertyName, "SETUP", INI_MAX_PROPERTY_NAME ) == 0 )
					iniValue( hIni, szSetup );

				iniPropertyNext( hIni );
			}

			pListViewItem = new QListViewItem( lvwDrivers, szDriverName, szDescription, szDriver, szSetup );
			iniObjectNext( hIni );
		}
	}
	else
	{
		qsError.sprintf( "Could not open System odbc.ini file at %s", szINI );
		KMessageBox::information(	this, qsError );
	}
	
}

void classDrivers::FreeProperties( HODBCINSTPROPERTY *hFirstProperty )
{
	HODBCINSTPROPERTY	hNextProperty;
	HODBCINSTPROPERTY	hCurProperty;

	/* SANITY CHECKS */
	if ( (*hFirstProperty) == NULL )
		return;

	/* FREE MEMORY */
	for ( hCurProperty = (*hFirstProperty); hCurProperty != NULL; hCurProperty = hNextProperty )
	{
		hNextProperty = hCurProperty->pNext;

		/* FREE ANY PROMPT DATA (ie pick list options and such) */
		if ( hCurProperty->aPromptData != NULL )
			free( hCurProperty->aPromptData );

		/* FREE OTHER STUFF */
        if ( hCurProperty->pszHelp != NULL )
			free( hCurProperty->pszHelp );

		free( hCurProperty );
	}
    (*hFirstProperty) = NULL;
}



