//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.

#include <config.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qimage.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpainter.h>
#include <qlayout.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kwm.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>

#include <X11/Xlib.h>

#include "kaccel.h"
#include <klocale.h>

#include "keyconfig.h"
#include "keyconfig.moc"


//----------------------------------------------------------------------------

       extern bool global_switch  ;

KKeyConfig::KKeyConfig( QWidget *parent, const char *name )
	: KConfigWidget( parent, name )
{

       bool check_against_std_keys  = false ;

// global_switch allows this module to configure both
// standard and global keys
// (Duncan Haldane, 1998-12-19)


       if ( global_switch ) KeyType = "global" ;
       else KeyType = "standard" ;

	globalDict = new QDict<int> ( 37, false );
	globalDict->setAutoDelete( true );
	
	dict.setAutoDelete( false );
	keys = new KAccel( this );

        if ( KeyType == "global" ) {
            #include "../../kwm/kwmbindings.cpp"
	    KeyScheme = "Global Key Scheme " ;
	    KeySet    = "Global Keys" ;
            check_against_std_keys  = true ;
	 }

        if ( KeyType == "standard" ) {
            #include "./stdbindings.cpp"
	    KeyScheme = "Standard Key Scheme " ;
	    KeySet    = "Keys" ;
            check_against_std_keys  = false ;
	 }


	//debug("inserted keys");
	
	keys->setConfigGlobal( true );
	keys->setConfigGroup( KeySet );
	keys->readSettings();
	
	//debug("read settings");
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	topLayout->addLayout( pushLayout );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 4 );
	
	pushLayout->addLayout( stackLayout, 20 );
	
	sFileList = new QStringList();
	sList = new QListBox( this );
	
	sList->clear();
	sFileList->clear();
	sList->insertItem( i18n("Current scheme"), 0 );
	sFileList->append( "Not a  kcsrc file" );
	sList->insertItem( i18n("KDE default"), 1 );
	sFileList->append( "Not a kcsrc file" );
	readSchemeNames();
	sList->setCurrentItem( 0 );
	connect( sList, SIGNAL( highlighted( int ) ),
			SLOT( slotPreviewScheme( int ) ) );
	
	QLabel *label = new QLabel( sList, i18n("&Key scheme"), this );
	label->adjustSize();
	label->setFixedHeight( label->height() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( sList );
	
	stackLayout = new QVBoxLayout( 5 );
	
	pushLayout->addLayout( stackLayout, 10 );
	
	addBt = new QPushButton(  i18n("&Add ..."), this );
	addBt->adjustSize();
	addBt->setFixedHeight( addBt->height() );
	addBt->setMinimumWidth( addBt->width() );
	connect( addBt, SIGNAL( clicked() ), SLOT( slotAdd() ) );
	
	QBoxLayout *push2Layout = new QHBoxLayout( 5 );
	
	stackLayout->addStretch( 10 );
	stackLayout->addLayout( push2Layout );
	
	push2Layout->addWidget( addBt, 10 );
	
	removeBt = new QPushButton(  i18n("&Remove"), this );
	removeBt->adjustSize();
	removeBt->setFixedHeight( removeBt->height() );
	removeBt->setMinimumWidth( removeBt->width() );
	removeBt->setEnabled(FALSE);
	connect( removeBt, SIGNAL( clicked() ), SLOT( slotRemove() ) );
	
	push2Layout->addWidget( removeBt, 10 );
	
	saveBt = new QPushButton(  i18n("&Save changes"), this );
	saveBt->adjustSize();
	saveBt->setFixedHeight( saveBt->height() );
	saveBt->setMinimumWidth( saveBt->width());
	saveBt->setEnabled(FALSE);
	connect( saveBt, SIGNAL( clicked() ), SLOT( slotSave() ) );
	
	stackLayout->addWidget( saveBt );
	
	sList->setMinimumHeight( 2*saveBt->height() + 10 );
	
	QFrame* tmpQFrame;
	tmpQFrame = new QFrame( this );
	tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	tmpQFrame->setMinimumHeight( tmpQFrame->sizeHint().height() );
	
	topLayout->addWidget( tmpQFrame );
	
	dict = keys->keyDict();
	
	//debug("got key dict");
	
        kc =  new KKeyChooser( &dict, this, check_against_std_keys );
	connect( kc, SIGNAL( keyChange() ), this, SLOT( slotChanged() ) );

        	
	
        //if ( KeyType == "global" ) debug("Make key chooser global" );
        //if ( KeyType == "standard" ) debug("Make key chooser standard" );
	
	readScheme();

	//debug("read scheme done");

	
	topLayout->addWidget( kc, 10 );
	
	topLayout->activate();
	
}

KKeyConfig::~KKeyConfig (){
  //debug("KKeyConfig destructor");
  delete keys;
}

void KKeyConfig::slotRemove()
{
	QString kksPath = getenv( "HOME" );
	kksPath += "/.kde/share/apps/kcmkeys/";
        kksPath += KeyType ;
	
	QDir d( kksPath );
	if (!d.exists()) // what can we do?
	  return;

	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.kksrc");
	
	uint ind = sList->currentItem();
	
	if ( !d.remove( *sFileList->at( ind ) ) ) {
		KMessageBox::sorry( 0,
                            i18n("This key scheme could not be removed.\n"
                                 "Perhaps you do not have permission to alter the file\n"
                                 "system where the key scheme is stored." ));
		return;
	}
	
	sList->removeItem( ind );
	sFileList->remove( sFileList->at(ind) );
}

void KKeyConfig::slotChanged( )
{
	if ( removeBt->isEnabled() )
		saveBt->setEnabled( TRUE );
	else
		saveBt->setEnabled( FALSE );
}

void KKeyConfig::slotSave( )
{
	KSimpleConfig *config =
			new KSimpleConfig( *sFileList->at( sList->currentItem() ) );
				
	config->setGroup( KeyScheme );
	
	kc->aIt->toFirst();
	while ( kc->aIt->current() ) {
		config->writeEntry( kc->aIt->currentKey(),
			KAccel::keyToString( kc->aIt->current()->aConfigKeyCode ) );
		++ ( *kc->aIt );
	}
	
	config->sync();
	
	saveBt->setEnabled( FALSE );
}


void KKeyConfig::readScheme( int index )
{
	KConfigBase* config;
	
	if( index == 1 ) {
		kc->allDefault();
		return;
	} if ( index == 0 ) {
		config  = kapp->config();
	} else {
		config =
			new KSimpleConfig( *sFileList->at( index ), true );
	}

	QMap<QString, QString> tmpMap;
	if ( index == 0 )
	  tmpMap = config->entryMap(KeySet);
	else
	  tmpMap = config->entryMap(KeyScheme);
	QMap<QString, QString>::Iterator gIt(tmpMap.begin());
		
	int *keyCode;

	for (; gIt != tmpMap.end(); ++gIt) {
	  keyCode = new int;
	  *keyCode = KAccel::stringToKey( *gIt );
	  globalDict->insert( gIt.key(), keyCode);
	  //debug( " %s, %d", gIt->currentKey(), *keyCode );
	}
	
	kc->aIt->toFirst();
	while ( kc->aIt->current() ) {
	  if ( globalDict->find( kc->aIt->currentKey() ) ) {
	    kc->aIt->current()->aConfigKeyCode = *globalDict->find( kc->aIt->currentKey() );
	    kc->aIt->current()->aCurrentKeyCode = kc->aIt->current()->aConfigKeyCode;
	    // debug("Change: %s", kc->aIt->currentKey().ascii() );
	  }
	  ++ ( *kc->aIt );
	}
	
	kc->listSync();
}

void KKeyConfig::slotAdd()
{
	SaveScm *ss = new SaveScm( 0,  "save scheme" );
	
	bool nameValid;
	QString sName;
	QString sFile;
	
	do {
	
	nameValid = TRUE;
	
	if ( ss->exec() ) {
		sName = ss->nameLine->text();
		if ( sName.stripWhiteSpace().isEmpty() )
			return;
			
		sName = sName.simplifyWhiteSpace();
		sFile = sName;
		
		int ind = 0;
		while ( ind < (int) sFile.length() ) {
			
			// parse the string for first white space
			
			ind = sFile.find(" ");
			if (ind == -1) {
				ind = sFile.length();
				break;
			}
		
			// remove from string
		
			sFile.remove( ind, 1);
			
			// Make the next letter upper case
			
			QString s = sFile.mid( ind, 1 );
			s = s.upper();
			sFile.replace( ind, 1, s );
			
		}
		
		for ( int i = 0; i < (int) sList->count(); i++ ) {
			if ( sName == sList->text( i ) ) {
				nameValid = FALSE;
				KMessageBox::error( 0,
                                    i18n( "Please choose a unique name for the new key\n"\
                                          "scheme. The one you entered already appears\n"\
                                          "in the key scheme list." ));
			}
		}
	} else return;
	
	} while ( nameValid == FALSE );
	
	disconnect( sList, SIGNAL( highlighted( int ) ), this,
			SLOT( slotPreviewScheme( int ) ) );
	
	sList->insertItem( sName );
	sList->setFocus();
	sList->setCurrentItem( sList->count()-1 );
	
	QString kksPath( getenv( "HOME" ) );
	
	kksPath += "/.kde/share/apps/kcmkeys/";
	
	QDir d( kksPath );
	if ( !d.exists() )
		if ( !d.mkdir( kksPath ) ) {
			warning("KKeyConfig: Could not make directory to store user info.");
			return;
		}
	
	kksPath +=  KeyType ;
        kksPath += "/";
	
	d.setPath( kksPath );
	if ( !d.exists() )
		if ( !d.mkdir( kksPath ) ) {
			warning("KKeyConfig: Could not make directory to store user info.");
			return;
		}
	
	sFile.prepend( kksPath );
	sFile += ".kksrc";
	sFileList->append( sFile );
	
	KSimpleConfig *config =
			new KSimpleConfig( sFile );
			
	config->setGroup( KeyScheme );
	config->writeEntry( "SchemeName", sName );
	delete config;
	
	slotSave();
	
	connect( sList, SIGNAL( highlighted( int ) ), this,
			SLOT( slotPreviewScheme( int ) ) );
			
	slotPreviewScheme( sList->currentItem() );
}

void KKeyConfig::slotPreviewScheme( int indx )
{
	readScheme( indx );
	
	// Set various appropriate for the scheme
	
	if ( indx < nSysSchemes ) {
		removeBt->setEnabled( FALSE );
		saveBt->setEnabled( FALSE );
	} else {
		removeBt->setEnabled( TRUE );
		saveBt->setEnabled( FALSE );
	}
	changed = TRUE;
}

void KKeyConfig::readSchemeNames( )
 {
   // Always a current and a default scheme
   nSysSchemes = 2;

   QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/" + KeyType + "/*.kksrc");

   // This for system files
   for ( QStringList::ConstIterator it = schemes.begin(); it != schemes.end(); it++) {

     KSimpleConfig config( *it, true );
     config.setGroup( KeyScheme );
     QString str = config.readEntry( "SchemeName" );

     sList->insertItem( str );
     sFileList->append( *it );

     nSysSchemes++;

   }
		
}


void KKeyConfig::loadSettings()
{

}

void KKeyConfig::applySettings()
{
	//debug("apply settings");
	//debug("No. of items in dict %d", dict.count() );
	keys->setKeyDict( dict );
	//debug("set key dict");
	keys->writeSettings();
	//debug("done");
}
void KKeyConfig::defaultSettings()
{
  kc->allDefault();
}
