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

#include <qlabel.h>
#include <qdir.h> 
#include <qlayout.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kaccel.h>
#include <kwm.h>

#include "keyconfig.h"
#include "keyconfig.moc"

// export two factories
extern "C" {
  KCModule *create_global(QWidget *parent, const char *name)
  {
    return new KKeyModule(parent, true, name);
  }
  
  KCModule *create_standard(QWidget *parent, const char *name)
  {
    return new KKeyModule(parent, false, name);
  }
}

//----------------------------------------------------------------------------

KKeyModule::KKeyModule( QWidget *parent, bool isGlobal, const char *name )
  : KCModule( parent, name )
{
  bool check_against_std_keys  = false ;
  
  KeyType = isGlobal ? "global" : "standard";
  
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
  
  keys->setConfigGlobal( true );
  keys->setConfigGroup( KeySet );
  keys->readSettings();
  
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

  addBt = new QPushButton(  i18n("&Add ..."), this );
  connect( addBt, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  
  removeBt = new QPushButton(  i18n("&Remove"), this );
  removeBt->setEnabled(FALSE);
  connect( removeBt, SIGNAL( clicked() ), SLOT( slotRemove() ) );
  
  saveBt = new QPushButton(  i18n("&Save changes"), this );
  saveBt->setEnabled(FALSE);
  connect( saveBt, SIGNAL( clicked() ), SLOT( slotSave() ) );
  
  QFrame* tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight(15);

  dict = keys->keyDict();
  
  //debug("got key dict");
  
  kc =  new KKeyChooser( &dict, this, check_against_std_keys );
  connect( kc, SIGNAL( keyChange() ), this, SLOT( slotChanged() ) );
  
  readScheme();

  QGridLayout *topLayout = new QGridLayout( this, 5, 3, 5);
  topLayout->addWidget(label, 0, 0);
  topLayout->addMultiCellWidget(sList, 1, 2, 0, 0);
  topLayout->addWidget(addBt, 1, 1);
  topLayout->addWidget(removeBt, 1, 2);
  topLayout->addMultiCellWidget(saveBt, 2, 2, 1, 2);
  topLayout->addMultiCellWidget(tmpQFrame, 3, 3, 0, 2);
  topLayout->addMultiCellWidget(kc, 4, 4, 0, 2);
  topLayout->activate();
}

KKeyModule::~KKeyModule (){
  //debug("KKeyModule destructor");
  delete keys;
}

void KKeyModule::init()
{
  KKeyModule *tmp=new KKeyModule(0, true);
  tmp->keys->writeSettings();
  delete tmp;
}

void KKeyModule::load()
{
  // TODO
}

void KKeyModule::save()
{
  keys->setKeyDict( dict );
  keys->writeSettings();
  KWM::configureWm();
}

void KKeyModule::defaults()
{
  kc->allDefault();
}

void KKeyModule::slotRemove()
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

void KKeyModule::slotChanged( )
{
  if ( removeBt->isEnabled() )
    saveBt->setEnabled( TRUE );
  else
    saveBt->setEnabled( FALSE );
}

void KKeyModule::slotSave( )
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

void KKeyModule::readScheme( int index )
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

void KKeyModule::slotAdd()
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
      warning("KKeyModule: Could not make directory to store user info.");
      return;
    }
  
  kksPath +=  KeyType ;
  kksPath += "/";
  
  d.setPath( kksPath );
  if ( !d.exists() )
    if ( !d.mkdir( kksPath ) ) {
      warning("KKeyModule: Could not make directory to store user info.");
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

void KKeyModule::slotPreviewScheme( int indx )
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

void KKeyModule::readSchemeNames( )
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
