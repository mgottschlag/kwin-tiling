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
#include <qmultilineedit.h>
#include <qpushbutton.h>

#include "installer.h"

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktrader.h>
#include <kurldrag.h>
#include <kio/netaccess.h>

ThemeListBox::ThemeListBox(QWidget *parent)
  : KListBox(parent)
{
   setAcceptDrops(true);
   connect(this, SIGNAL(mouseButtonPressed(int, QListBoxItem *, const QPoint &)),
           this, SLOT(slotMouseButtonPressed(int, QListBoxItem *, const QPoint &)));
}

void ThemeListBox::dragEnterEvent(QDragEnterEvent* event)
{
   event->accept((event->source() != this) && KURLDrag::canDecode(event));
}

void ThemeListBox::dropEvent(QDropEvent* event)
{
   KURL::List urls;
   if (KURLDrag::decode(event, urls))
   {
      emit filesDropped(urls);
   }
}

void ThemeListBox::slotMouseButtonPressed(int button, QListBoxItem *item, const QPoint &p)
{
   if ((button & LeftButton) == 0) return;
   mOldPos = p;
   mDragFile = QString::null;
   int cur = index(item);
   if (cur >= 0)
      mDragFile = text2path[text(cur)];
}

void ThemeListBox::mouseMoveEvent(QMouseEvent *e)
{
   if (((e->state() & LeftButton) != 0) && !mDragFile.isEmpty())
   {
      int delay = KGlobalSettings::dndEventDelay();
      QPoint newPos = e->globalPos();
      if(newPos.x() > mOldPos.x()+delay || newPos.x() < mOldPos.x()-delay ||
         newPos.y() > mOldPos.y()+delay || newPos.y() < mOldPos.y()-delay)
      {
         KURL url;
         url.setPath(mDragFile);
         KURL::List urls;
         urls.append(url);
         KURLDrag *d = KURLDrag::newDrag(urls, this);
         d->dragCopy();
      }
   }
   KListBox::mouseMoveEvent(e);
}

//-----------------------------------------------------------------------------
SplashInstaller::SplashInstaller (QWidget *aParent, const char *aName, bool aInit)
  : QWidget(aParent, aName), mGui(!aInit)
{
  KGlobal::dirs()->addResourceType("ksplashthemes", KStandardDirs::kde_default("data") + "ksplash/Themes");
  KButtonBox* bbox;

  if (!mGui)
    return;

  mGrid = new QGridLayout(this, 2, 3, 6, 6);
  mThemesList = new ThemeListBox(this);
  connect(mThemesList, SIGNAL(highlighted(int)), SLOT(slotSetTheme(int)));
  connect(mThemesList, SIGNAL(filesDropped(const KURL::List&)), SLOT(slotFilesDropped(const KURL::List&)));
  mGrid->addMultiCellWidget(mThemesList, 0, 1, 0, 0);

  mPreview = new QLabel(this);
  mPreview->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  mPreview->setMinimumSize(QSize(320,240));
  mGrid->addWidget(mPreview, 0, 1);

  bbox = new KButtonBox(this, KButtonBox::Vertical, 0, 6);
  mGrid->addMultiCellWidget(bbox, 0, 1, 2, 2);

  mBtnAdd = bbox->addButton(i18n("Add..."));
  connect(mBtnAdd, SIGNAL(clicked()), SLOT(slotAdd()));

  mBtnRemove = bbox->addButton(i18n("Remove"));
  connect(mBtnRemove, SIGNAL(clicked()), SLOT(slotRemove()));

  mBtnTest = bbox->addButton(i18n("Test"));
  connect(mBtnTest, SIGNAL(clicked()), SLOT(slotTest()));

  bbox->layout();

  mText = new QMultiLineEdit(this);
  mText->setMinimumSize(mText->sizeHint());
  mText->setReadOnly(true);
  mGrid->addWidget(mText, 1, 1);

  mGrid->setColStretch(0, 1);
  mGrid->setColStretch(1, 3);
  mGrid->setRowStretch(0, 3);
  mGrid->setRowStretch(1, 1);

  KConfig cnf("ksplashrc");
  cnf.setGroup("KSplash");
  mDefaultTheme = cnf.readEntry("Theme","Default");
  readThemesList();
  defaults();
}


//-----------------------------------------------------------------------------
SplashInstaller::~SplashInstaller()
{
}

int SplashInstaller::addTheme(const QString &path, const QString &name)
{
  //kdDebug() << "SplashInstaller::addTheme: " << path << " " << name << endl;
    QString tmp(i18n( name.utf8() ));
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
void SplashInstaller::addNewTheme(const KURL &srcURL)
{
  QString dir = KGlobal::dirs()->saveLocation("ksplashthemes");
  KURL url;
  QString filename = srcURL.fileName();
  int i = filename.findRev('.');
  // Convert extension to lower case.
  if (i >= 0)
     filename = filename.left(i)+filename.mid(i).lower();
  url.setPath(locateLocal("tmp",filename));

  bool rc = KIO::NetAccess::copy(srcURL, url);
  if (!rc)
  {
    kdWarning() << "Failed to copy theme " << srcURL.fileName()
        << " into temporary directory " << url.path() << endl;
    return;
  }

  // Extract into theme directory: we may have multiple themes in one tarball!
  KTar tarFile(url.path());
  if (!tarFile.open(IO_ReadOnly))
  {
    kdDebug() << "Unable to open archive: " << url.path() << endl;
    return;
  }
  KArchiveDirectory const *ad = tarFile.directory();
  // TODO: Make sure we put the entries into a subdirectory if the tarball does not.
  // TODO: Warn the user if we overwrite something.
  ad->copyTo(locateLocal("ksplashthemes","/"));
  tarFile.close();

  // TODO: Update only the entries from this installation.
  readThemesList();
}

//-----------------------------------------------------------------------------
void SplashInstaller::readThemesList()
{
  mThemesList->clear();

  // Read local themes
  QStringList entryList = KGlobal::dirs()->resourceDirs("ksplashthemes");
  kdDebug() << entryList << endl;
  QDir dir;
  QStringList subdirs;
  QStringList::ConstIterator name;
  for(name = entryList.begin(); name != entryList.end(); name++)
  {
    dir = *name;
    if (!dir.exists())
      continue;
    subdirs = dir.entryList( QDir::Dirs );
    kdDebug() << subdirs << endl;
    // TODO: Make sure it contains a *.rc file.
    for (QStringList::Iterator l = subdirs.begin(); l != subdirs.end(); l++ )
      if ( !(*l).startsWith(QString(".")) )
        addTheme(dir.path(),*l);
  }
}

//-----------------------------------------------------------------------------
void SplashInstaller::defaults()
{
  mThemesList->setCurrentItem(findTheme(mDefaultTheme));
}

void SplashInstaller::load()
{
  kdDebug() << "SplashInstaller::load() called" << endl;
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
  cur = path.findRev('/');
  cnf.writeEntry("Theme", path.mid(cur+1) );
  cnf.sync();
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
     KURL url;
     url.setPath(themeDir);
     if (KMessageBox::questionYesNo(this,i18n("Delete directory %1 and its contents?").arg(themeDir))==KMessageBox::Yes)
       rc = KIO::NetAccess::del(url,this);
  }
  if (!rc)
  {
    KMessageBox::sorry(this, i18n("Failed to remove theme '%1'").arg(themeName));
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
  QString path(QString::null);

  if (id < 0)
  {
    mPreview->setText("");
    mText->setText("");
    enabled = false;
  }
  else
  {
    QString error = i18n("(Could not load theme)");
    path = mThemesList->text(id);
    if ( mThemesList->text2path.contains( path ) )
        path = mThemesList->text2path[path];
    enabled = false;
    KURL url;
    if (!path.isEmpty())
    {
      // Make sure the correct plugin is installed.
      QString themeName;
      int i = path.findRev('/');
      if (i >= 0)
        themeName = path.mid(i+1);
      url.setPath( path + "/Theme.rc" );
      if (!KIO::NetAccess::exists(url))
      {
        url.setPath( path + "/Theme.RC" );
        if (!KIO::NetAccess::exists(url))
        {
          url.setPath( path + "/theme.rc" );
          if (!KIO::NetAccess::exists(url))
            url.setPath( path + "/" + themeName + ".rc" );
        }
      }
      if (KIO::NetAccess::exists(url))
      {
        KConfig cnf(url.path());
        cnf.setGroup( QString("KSplash Theme: %1").arg(themeName) );
        QString pluginName( cnf.readEntry( "Engine", "Default" ) ); // Perhaps no default is better?
        if ((KTrader::self()->query("KSplash/Plugin", QString("[X-KSplash-PluginName] == '%1'").arg(pluginName))).isEmpty())
        {
          enabled = false;
          error = QString(i18n("This theme requires the plugin %1 which is not installed.")).arg(pluginName);
        }
        else
          enabled = true; // Hooray, there is at least one plugin which can handle this theme.
      }
      else
      {
        error = i18n("Could not load theme configuration file.");
      }
    }
    mBtnTest->setEnabled(enabled);
    if (!enabled)
    {
      url.setPath( path + "/" + "Preview.png" );
      if (KIO::NetAccess::exists(url))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("(Could not load theme)"));
      mText->setText("");
      KMessageBox::sorry(this, error);
    }
    else
    {
      url.setPath( path + "/" + "Preview.png" );
      if (KIO::NetAccess::exists(url))
        mPreview->setPixmap(QPixmap(url.path()));
      else
        mPreview->setText(i18n("No preview available."));
      // TODO: Show information about the theme in the text box at the bottom.
      emit changed(true);
    }
  }
  mBtnRemove->setEnabled( !path.isEmpty() && QFileInfo(path).isWritable());
}


//-----------------------------------------------------------------------------
void SplashInstaller::slotAdd()
{
  static QString path;
  if (path.isEmpty()) path = QDir::homeDirPath();

  KFileDialog dlg(path, "*.tgz *.tar.gz *.tar.bz2|KSplash theme files", 0, 0, true);
  dlg.setCaption(i18n("Add Theme"));
  if (!dlg.exec())
    return;

  path = dlg.baseURL().url();
  addNewTheme(dlg.selectedURL());
}

//-----------------------------------------------------------------------------
void SplashInstaller::slotFilesDropped(const KURL::List &urls)
{
  for(KURL::List::ConstIterator it = urls.begin();
      it != urls.end();
      ++it)
      addNewTheme(*it);
}

//-----------------------------------------------------------------------------
int SplashInstaller::findTheme( const QString &theme )
{
  int id = mThemesList->count()-1;

  while (id >= 0)
  {
    if (mThemesList->text(id) == theme)
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
  int r = themeName.findRev('/');
  if (r >= 0)
    themeName = themeName.mid(r+1);

  KProcess proc;
  proc << "ksplash" << "--test" << "--theme" << themeName;
  if (!proc.start(KProcess::Block))
    KMessageBox::error(this,i18n("Unable to start ksplash."));
}

//-----------------------------------------------------------------------------
#include "installer.moc"
