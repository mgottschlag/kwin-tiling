//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.
// Layout fixes copyright (c) 2000 Preston Brown <pbrown@kde.org>

#include <config.h>
#include <stdlib.h>

#include <unistd.h>

#include <qlabel.h>
#include <qdir.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <kaccel.h>
#include <kwin.h>
#include <kdialog.h>
#include <dcopclient.h>
#include <kapp.h>

#include "keyconfig.h"
#include "keyconfig.moc"


//----------------------------------------------------------------------------

KKeyModule::KKeyModule( QWidget *parent, bool isGlobal, const char *name )
  : KCModule( parent, name )
{
  QString wtstr;
  bool check_against_std_keys  = false ;

  KeyType = isGlobal ? "global" : "standard";

  globalDict = new QDict<int> ( 37, false );
  globalDict->setAutoDelete( true );

  keys = new KAccel( this );

  if ( KeyType == "global" ) {
#include "../../klipper/klipperbindings.cpp"
#include "../../kwin/kwinbindings.cpp"
#include "../../kicker/core/kickerbindings.cpp"
#include "../../kdesktop/kdesktopbindings.cpp"
    KeyScheme = "Global Key Scheme " ;
    KeySet    = "Global Keys" ;
    check_against_std_keys  = true ;
  }

  if ( KeyType == "standard" ) {
    for(uint i=0; i<KStdAccel::NB_STD_ACCELS; i++) {
      KStdAccel::StdAccel id = (KStdAccel::StdAccel)i;
      keys->insertItem( KStdAccel::description(id),
                        KStdAccel::action(id),
                        KStdAccel::defaultKey(id),
                        true );
    }

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
  sFileList->append( "Not a kcsrc file" );
  sList->insertItem( i18n("KDE default"), 1 );
  sFileList->append( "Not a kcsrc file" );
  readSchemeNames();
  sList->setCurrentItem( 0 );
  connect( sList, SIGNAL( highlighted( int ) ),
           SLOT( slotPreviewScheme( int ) ) );

  QLabel *label = new QLabel( sList, i18n("&Key scheme"), this );

  wtstr = i18n("Here you can see a list of the existing key binding schemes with 'Current scheme'"
    " referring to the settings you are using right now. Select a scheme to use it, remove or change"
    " it.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( sList, wtstr );

  addBt = new QPushButton(  i18n("&Save scheme..."), this );
  connect( addBt, SIGNAL( clicked() ), SLOT( slotAdd() ) );
  QWhatsThis::add(addBt, i18n("Click here to add a new key bindings scheme. You will be prompted for a name."));

  removeBt = new QPushButton(  i18n("&Remove scheme"), this );
  removeBt->setEnabled(FALSE);
  connect( removeBt, SIGNAL( clicked() ), SLOT( slotRemove() ) );
  QWhatsThis::add( removeBt, i18n("Click here to remove the selected key bindings scheme. You can not"
    " remove the standard system wide schemes, 'Current scheme' and 'KDE default'.") );

  QFrame* tmpQFrame = new QFrame( this );
  tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  tmpQFrame->setMinimumHeight(15);

  dict = keys->keyDict();

  //kdDebug() << "got key dict" << endl;

  kc =  new KKeyChooser( &dict, this, check_against_std_keys );
  connect( kc, SIGNAL( keyChange() ), this, SLOT( slotChanged() ) );

  readScheme();

  QGridLayout *topLayout = new QGridLayout( this, 5, 3,
                                            KDialog::marginHint(),
                                            KDialog::spacingHint());
  topLayout->addWidget(label, 0, 0);
  topLayout->addMultiCellWidget(sList, 1, 2, 0, 0);
  topLayout->addMultiCellWidget(addBt, 1, 1, 1, 2);
  topLayout->addMultiCellWidget(removeBt, 2, 2, 1, 2);
  topLayout->addMultiCellWidget(tmpQFrame, 3, 3, 0, 2);
  topLayout->addMultiCellWidget(kc, 4, 4, 0, 2);

  setMinimumSize(topLayout->sizeHint());
}

KKeyModule::~KKeyModule (){
  //kdDebug() << "KKeyModule destructor" << endl;
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
  if ( KeyType == "global" ) {
    if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
    kapp->dcopClient()->send("kwin", "", "reconfigure()", "");
    kapp->dcopClient()->send("kdesktop", "", "configure()", "");
    kapp->dcopClient()->send("kicker", "Panel", "configure()", "");
  }
}

void KKeyModule::defaults()
{
  kc->allDefault();
}

void KKeyModule::slotRemove()
{
  QString kksPath =
        KGlobal::dirs()->saveLocation("data", "kcmkeys/" + KeyType);

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
  emit changed(true);
}

void KKeyModule::slotSave( )
{
    KSimpleConfig config(*sFileList->at( sList->currentItem() ) );

    config.setGroup( KeyScheme );
    for (KKeyEntryMap::ConstIterator it = dict.begin();
         it != dict.end(); ++it) {
        config.writeEntry( it.key(),
                           KAccel::keyToString( (*it).aConfigKeyCode, false ) );
    }
}

void KKeyModule::readScheme( int index )
{
  KConfigBase* config;

  if( index == 1 ) {
    kc->allDefault();
    return;
  } if ( index == 0 ) {
    config  = KGlobal::config();
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
    //kdDebug() << gIt->currentKey() << ", " << *keyCode << endl;
  }

  for (KKeyEntryMap::Iterator it = dict.begin(); it != dict.end(); ++it) {
      if ( globalDict->find( it.key() ) ) {
      (*it).aConfigKeyCode = *globalDict->find( it.key() );
      (*it).aCurrentKeyCode = (*it).aConfigKeyCode;
      // kdDebug() << "Change: " << kc->aIt->currentKey() << endl;
    }
  }

  kc->listSync();
}

void KKeyModule::slotAdd()
{
  QString sName;

  if ( sList->currentItem() >= nSysSchemes )
     sName = sList->currentText();
  SaveScm ss( 0,  "save scheme", sName );

  bool nameValid;
  QString sFile;
  int exists = -1;

  do {

    nameValid = TRUE;

    if ( ss.exec() ) {
      sName = ss.nameLine->text();
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

      exists = -1;
      for ( int i = 0; i < (int) sList->count(); i++ ) {
        if ( sName.lower() == (sList->text(i)).lower() ) {
          exists = i;

          int result = KMessageBox::warningContinueCancel( 0,
               i18n("A key scheme with the name '%1' already exists.\n"
                    "Do you want to overwrite it?\n").arg(sName),
		   i18n("Save key scheme"),
                   i18n("Overwrite"));
          if (result == KMessageBox::Continue)
             nameValid = true;
          else
             nameValid = false;
        }
      }
    } else return;

  } while ( nameValid == FALSE );

  disconnect( sList, SIGNAL( highlighted( int ) ), this,
              SLOT( slotPreviewScheme( int ) ) );


  QString kksPath = KGlobal::dirs()->saveLocation("data", "kcmkeys/");

  QDir d( kksPath );
  if ( !d.exists() )
    if ( !d.mkdir( kksPath ) ) {
      qWarning("KKeyModule: Could not make directory to store user info.");
      return;
    }

  kksPath +=  KeyType ;
  kksPath += "/";

  d.setPath( kksPath );
  if ( !d.exists() )
    if ( !d.mkdir( kksPath ) ) {
      qWarning("KKeyModule: Could not make directory to store user info.");
      return;
    }

  sFile.prepend( kksPath );
  sFile += ".kksrc";
  if (exists == -1)
  {
     sList->insertItem( sName );
     sList->setFocus();
     sList->setCurrentItem( sList->count()-1 );
     sFileList->append( sFile );
  }
  else
  {
     sList->setFocus();
     sList->setCurrentItem( exists );
  }

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
  } else {
    removeBt->setEnabled( TRUE );
  }
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

    // bug?  this makes it impossible to delete any scheme.
    //nSysSchemes++;

  }

}
