/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *   Copyright (c) 1998 Stefan Taferner <taferner@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <stdlib.h>

#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3textedit.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFrame>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include "installer.h"

#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktrader.h>
#include <k3urldrag.h>
#include <kio/netaccess.h>

ThemeListBox::ThemeListBox(QWidget *parent)
  : KListBox(parent)
{
   setAcceptDrops(true);
   connect(this, SIGNAL(mouseButtonPressed(int, Q3ListBoxItem *, const QPoint &)),
           this, SLOT(slotMouseButtonPressed(int, Q3ListBoxItem *, const QPoint &)));
}

void ThemeListBox::dragEnterEvent(QDragEnterEvent* event)
{
   event->setAccepted((event->source() != this) && K3URLDrag::canDecode(event));
}

void ThemeListBox::dropEvent(QDropEvent* event)
{
   KUrl::List urls;
   if (K3URLDrag::decode(event, urls))
   {
      emit filesDropped(urls);
   }
}

void ThemeListBox::slotMouseButtonPressed(int button, Q3ListBoxItem *item, const QPoint &p)
{
   if ((button & Qt::LeftButton) == 0) return;
   mOldPos = p;
   mDragFile.clear();
   int cur = index(item);
   if (cur >= 0)
      mDragFile = text2path[text(cur)];
}

void ThemeListBox::mouseMoveEvent(QMouseEvent *e)
{
   if (((e->state() & Qt::LeftButton) != 0) && !mDragFile.isEmpty())
   {
      int delay = KGlobalSettings::dndEventDelay();
      QPoint newPos = e->globalPos();
      if(newPos.x() > mOldPos.x()+delay || newPos.x() < mOldPos.x()-delay ||
         newPos.y() > mOldPos.y()+delay || newPos.y() < mOldPos.y()-delay)
      {
         KUrl url;
         url.setPath(mDragFile);
         KUrl::List urls;
         urls.append(url);
         K3URLDrag *d = new K3URLDrag(urls, this);
         d->dragCopy();
      }
   }
   KListBox::mouseMoveEvent(e);
}

//-----------------------------------------------------------------------------
SplashInstaller::SplashInstaller (QWidget *aParent, const char *aName, bool aInit)
  : QWidget(aParent), mGui(!aInit)
{
  setObjectName(aName);
  KGlobal::dirs()->addResourceType("ksplashthemes", KStandardDirs::kde_default("data") + "ksplash/Themes");

  if (!mGui)
    return;

  QHBoxLayout* hbox = new QHBoxLayout( this );
  hbox->setMargin( 0 );
  hbox->setSpacing( KDialog::spacingHint() );

  QVBoxLayout* leftbox = new QVBoxLayout(  );
  hbox->addItem( leftbox );
  leftbox->setSpacing( KDialog::spacingHint() );
  hbox->setStretchFactor( leftbox, 1 );

  mThemesList = new ThemeListBox(this);
  mThemesList->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
  connect(mThemesList, SIGNAL(highlighted(int)), SLOT(slotSetTheme(int)));
  connect(mThemesList, SIGNAL(filesDropped(const KUrl::List&)), SLOT(slotFilesDropped(const KUrl::List&)));
  leftbox->addWidget(mThemesList);

  mBtnAdd = new KPushButton( i18n("Add..."), this );
  leftbox->addWidget( mBtnAdd );
  connect(mBtnAdd, SIGNAL(clicked()), SLOT(slotAdd()));

  mBtnRemove = new KPushButton( i18n("Remove"), this );
  leftbox->addWidget( mBtnRemove );
  connect(mBtnRemove, SIGNAL(clicked()), SLOT(slotRemove()));

  mBtnTest = new KPushButton( i18n("Test"), this );
  leftbox->addWidget( mBtnTest );
  connect(mBtnTest, SIGNAL(clicked()), SLOT(slotTest()));

  QVBoxLayout* rightbox = new QVBoxLayout(  );
  hbox->addItem( rightbox );
  rightbox->setSpacing( KDialog::spacingHint() );
  hbox->setStretchFactor( rightbox, 3 );

  mPreview = new QLabel(this);
  mPreview->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  mPreview->setMinimumSize(QSize(320,240));
  mPreview->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  rightbox->addWidget(mPreview);
  rightbox->setStretchFactor( mPreview, 3 );

  mText = new Q3TextEdit(this);
  mText->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  mText->setMinimumSize(mText->sizeHint());
  mText->setReadOnly(true);
  rightbox->addWidget(mText);
  rightbox->setStretchFactor( mText, 1 );

  readThemesList();
  load();
}


//-----------------------------------------------------------------------------
SplashInstaller::~SplashInstaller()
{
}

int SplashInstaller::addTheme(const QString &path, const QString &name)
{
  //kDebug() << "SplashInstaller::addTheme: " << path << " " << name << endl;
  QString tmp(i18n( name.toUtf8() ));
  int i = mThemesList->count();
  while((i > 0) && (mThemesList->text(i-1) > tmp))
    i--;
  if ((i > 0) && (mThemesList->text(i-1) == tmp))
    return i-1;
  mThemesList->insertItem(tmp, i);
  mThemesList->text2path.insert( tmp, path+"/"+name );
  return i;
}

// Copy theme package into themes directory
void SplashInstaller::addNewTheme(const KUrl &srcURL)
{
  QString dir = KGlobal::dirs()->saveLocation("ksplashthemes");
  KUrl url;
  QString filename = srcURL.fileName();
  int i = filename.lastIndexOf('.');
  // Convert extension to lower case.
  if (i >= 0)
     filename = filename.left(i)+filename.mid(i).toLower();
  url.setPath(locateLocal("tmp",filename));

  // Remove file from temporary directory if it aleady exists - usually the result of a failed install.
  if ( KIO::NetAccess::exists( url, true, 0 ) )
    KIO::NetAccess::del( url, 0 );

  bool rc = KIO::NetAccess::copy(srcURL, url, 0);
  if (!rc)
  {
    kWarning() << "Failed to copy theme " << srcURL.fileName()
        << " into temporary directory " << url.path() << endl;
    return;
  }

  // Extract into theme directory: we may have multiple themes in one tarball!
  KTar tarFile(url.path());
  if (!tarFile.open(QIODevice::ReadOnly))
  {
    kDebug() << "Unable to open archive: " << url.path() << endl;
    return;
  }
  KArchiveDirectory const *ad = tarFile.directory();
  // Find first directory entry.
  QStringList entries = ad->entries();
  QString themeName( entries.first() );
#if 0
  // The isDirectory() call always returns false; why?
  for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
  {
    if ( ad->entry( *it )->isDirectory() )
    {
      themeName = *it;
      break;
    }
  }
#endif
  // TODO: Make sure we put the entries into a subdirectory if the tarball does not.
  // TODO: Warn the user if we overwrite something.
  ad->copyTo(locateLocal("ksplashthemes","/"));
  tarFile.close();
  KIO::NetAccess::del( url, 0 );

  // TODO: Update only the entries from this installation.
  readThemesList();
  mThemesList->setCurrentItem(findTheme(themeName));
  mThemesList->setSelected(mThemesList->currentItem(), true);
}

//-----------------------------------------------------------------------------
void SplashInstaller::readThemesList()
{
  mThemesList->clear();

  // Read local themes
  QStringList entryList = KGlobal::dirs()->resourceDirs("ksplashthemes");
  //kDebug() << "readThemesList: " << entryList << endl;
  QDir dir;
  QStringList subdirs;
  QStringList::ConstIterator name;
  for(name = entryList.begin(); name != entryList.end(); name++)
  {
    dir = *name;
    if (!dir.exists())
      continue;
    subdirs = dir.entryList( QDir::Dirs );
    // kDebug() << "readThemesList: " << subdirs << endl;
    // TODO: Make sure it contains a *.rc file.
    for (QStringList::Iterator l = subdirs.begin(); l != subdirs.end(); l++ )
      if ( !(*l).startsWith(QString(".")) )
      {
        mThemesList->blockSignals( true ); // Don't activate any theme until all themes are loaded.
        addTheme(dir.path(),*l);
        mThemesList->blockSignals( false );
      }
  }
}

//-----------------------------------------------------------------------------
void SplashInstaller::defaults()
{
  mThemesList->setCurrentItem(findTheme("Default"));
  emit changed( true );
}

void SplashInstaller::load()
{
  KConfig cnf("ksplashrc");
  cnf.setGroup("KSplash");
  QString curTheme = cnf.readEntry("Theme","Default");
  mThemesList->setCurrentItem(findTheme(curTheme));
  emit changed( false );
}

//-----------------------------------------------------------------------------
void SplashInstaller::save()
{
  KConfig cnf("ksplashrc");
  cnf.setGroup("KSplash");
  int cur = mThemesList->currentItem();
  if (cur < 0)
    return;
  QString path = mThemesList->text(cur);
  if ( mThemesList->text2path.contains( path ) )
    path = mThemesList->text2path[path];
  cur = path.lastIndexOf('/');
  cnf.writeEntry("Theme", path.mid(cur+1) );
  cnf.sync();
  emit changed( false );
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotRemove()
{
  int cur = mThemesList->currentItem();
  if (cur < 0)
    return;

  bool rc = false;
  QString themeName = mThemesList->text(cur);
  QString themeDir = mThemesList->text2path[themeName];
  if (!themeDir.isEmpty())
  {
     KUrl url;
     url.setPath(themeDir);
     if (KMessageBox::warningContinueCancel(this,i18n("Delete folder %1 and its contents?", themeDir),"",KGuiItem(i18n("&Delete"),"editdelete"))==KMessageBox::Continue)
       rc = KIO::NetAccess::del(url,this);
     else
       return;
  }
  if (!rc)
  {
    KMessageBox::sorry(this, i18n("Failed to remove theme '%1'", themeName));
    return;
  }
  //mThemesList->removeItem(cur);
  readThemesList();
  cur = ((unsigned int)cur >= mThemesList->count())?mThemesList->count()-1:cur;
  mThemesList->setCurrentItem(cur);
}


//-----------------------------------------------------------------------------
void SplashInstaller::slotSetTheme(int id)
{
  bool enabled;
  QString path = QString();
  QString infoTxt;

  if (id < 0)
  {
    mPreview->setText(QString());
    mText->setText(QString());
    enabled = false;
  }
  else
  {
    QString error = i18n("(Could not load theme)");
    path = mThemesList->text(id);
    if ( mThemesList->text2path.contains( path ) )
        path = mThemesList->text2path[path];
    enabled = false;
    KUrl url;
    QString themeName;
    if (!path.isEmpty())
    {
      // Make sure the correct plugin is installed.
      int i = path.lastIndexOf('/');
      if (i >= 0)
        themeName = path.mid(i+1);
      url.setPath( path + "/Theme.rc" );
      if (!KIO::NetAccess::exists(url, true, 0))
      {
        url.setPath( path + "/Theme.RC" );
        if (!KIO::NetAccess::exists(url, true, 0))
        {
          url.setPath( path + "/theme.rc" );
          if (!KIO::NetAccess::exists(url, true, 0))
            url.setPath( path + "/" + themeName + ".rc" );
        }
      }
      if (KIO::NetAccess::exists(url, true, 0))
      {
        KConfig cnf(url.path());
        cnf.setGroup( QString("KSplash Theme: %1").arg(themeName) );

        // Get theme information.
        infoTxt = "<qt>";
        if ( cnf.hasKey( "Name" ) )
          infoTxt += i18n( "<b>Name:</b> %1<br>", cnf.readEntry( "Name", i18n( "Unknown" ) ) );
        if ( cnf.hasKey( "Description" ) )
          infoTxt += i18n( "<b>Description:</b> %1<br>", cnf.readEntry( "Description", i18n( "Unknown" ) ) );
        if ( cnf.hasKey( "Version" ) )
          infoTxt += i18n( "<b>Version:</b> %1<br>", cnf.readEntry( "Version", i18n( "Unknown" ) ) );
        if ( cnf.hasKey( "Author" ) )
          infoTxt += i18n( "<b>Author:</b> %1<br>", cnf.readEntry( "Author", i18n( "Unknown" ) ) );
        if ( cnf.hasKey( "Homepage" ) )
          infoTxt += i18n( "<b>Homepage:</b> %1<br>", cnf.readEntry( "Homepage", i18n( "Unknown" ) ) );
        infoTxt += "</qt>";

        QString pluginName( cnf.readEntry( "Engine", "Default" ) ); // Perhaps no default is better?
        if ((KTrader::self()->query("KSplash/Plugin", QString("[X-KSplash-PluginName] == '%1'").arg(pluginName))).isEmpty())
        {
          enabled = false;
          error = i18n("This theme requires the plugin %1 which is not installed.", pluginName);
        }
        else
          enabled = true; // Hooray, there is at least one plugin which can handle this theme.
      }
      else
      {
        error = i18n("Could not load theme configuration file.");
      }
    }
    mBtnTest->setEnabled(enabled && themeName != "None" );
    mText->setText(infoTxt);
    if (!enabled)
    {
      url.setPath( path + "/" + "Preview.png" );
      if (KIO::NetAccess::exists(url, true, 0))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("(Could not load theme)"));
      KMessageBox::sorry(this, error);
    }
    else
    {
      url.setPath( path + "/" + "Preview.png" );
      if (KIO::NetAccess::exists(url, true, 0))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("No preview available."));
      emit changed(true);
    }
  }
  mBtnRemove->setEnabled( !path.isEmpty() && QFileInfo(path).isWritable());
}


//-----------------------------------------------------------------------------
void SplashInstaller::slotAdd()
{
  static QString path;
  if (path.isEmpty()) path = QDir::homePath();

  KFileDialog dlg(path, "*.tgz *.tar.gz *.tar.bz2|" + i18n( "KSplash Theme Files" ), 0);
  dlg.setCaption(i18n("Add Theme"));
  if (!dlg.exec())
    return;

  path = dlg.baseURL().url();
  addNewTheme(dlg.selectedURL());
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotFilesDropped(const KUrl::List &urls)
{
  for(KUrl::List::ConstIterator it = urls.begin();
      it != urls.end();
      ++it)
      addNewTheme(*it);
}

//-----------------------------------------------------------------------------
int SplashInstaller::findTheme( const QString &theme )
{
  // theme is untranslated, but the listbox contains translated items
  QString tmp(i18n( theme.toUtf8() ));
  int id = mThemesList->count()-1;

  while (id >= 0)
  {
    if (mThemesList->text(id) == tmp)
      return id;
    id--;
  }

  return 0;
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotTest()
{
  int i = mThemesList->currentItem();
  if (i < 0)
    return;
  QString themeName = mThemesList->text2path[mThemesList->text(i)];
  int r = themeName.lastIndexOf('/');
  if (r >= 0)
    themeName = themeName.mid(r+1);

  // special handling for none and simple splashscreens
  if( themeName == "None" )
    return;
  if( themeName == "Simple" )
  {
    KProcess proc;
    proc << "ksplashsimple" << "--test";
    if (!proc.start(KProcess::Block))
      KMessageBox::error(this,i18n("Unable to start ksplashsimple."));
    return;
  }
  KProcess proc;
  proc << "ksplash" << "--test" << "--theme" << themeName;
  if (!proc.start(KProcess::Block))
    KMessageBox::error(this,i18n("Unable to start ksplash."));
}

//-----------------------------------------------------------------------------
#include "installer.moc"
