// Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

#include <qdir.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qlistbox.h>

#include <dcopclient.h>

#include <kapp.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include "kwintheme.h"

extern "C"
{
  KCModule * create_kwintheme(QWidget * parent, const char * name)
  {
    KGlobal::locale()->insertCatalogue("kcmkwintheme");
    return new KWinThemeModule(parent, name);
  }
}

KWinThemeModule::KWinThemeModule(QWidget * parent, const char * name)
  : KCModule(parent, name)
{
  // Find all theme desktop files in all 'data' dirs owned by kwin.
  // Create a ThemeInfo object for each of these.

  QStringList dirList = KGlobal::dirs()->findDirs("data", "kwin");
  QStringList::ConstIterator it;

  for (it = dirList.begin(); it != dirList.end(); ++it)
  {
    QDir d(*it);

    if (d.exists())
    {
      const QFileInfoList * entryList = d.entryInfoList();

      for (QFileInfoListIterator it(*entryList); it.current(); ++it)
      {
        QString filename(it.current()->absFilePath());

        if (KDesktopFile::isDesktopFile(filename))
        {
          KDesktopFile desktopFile(filename);
          QString libName = desktopFile.readEntry("X-KDE-Library");

          if (!libName.isEmpty())
            themeInfoList_.append(ThemeInfo(desktopFile.readName(), libName));
        }
      }
    }
  }

  lb_themes_ = new QListBox(this);

  connect
    (
     lb_themes_,
     SIGNAL(selectionChanged()),
     SLOT(slotSelectionChanged())
    );

  QVBoxLayout * layout = new QVBoxLayout(this);

  layout->addWidget(lb_themes_);

  load();
}

KWinThemeModule::~KWinThemeModule()
{
  // Empty.
}

void KWinThemeModule::load()
{
  QString currentLibraryName = savedLibraryName();

  int i = 0;

  QValueList<ThemeInfo>::ConstIterator it;

  for (it = themeInfoList_.begin(); it != themeInfoList_.end(); ++it)
  {
    ThemeInfo info(*it);

    lb_themes_->insertItem(info.name(), i);

    if (info.libraryName() == currentLibraryName)
      lb_themes_->setCurrentItem(i);

    ++i;
  }

  emit changed(false);
}

void KWinThemeModule::defaults()
{
  int i = 0;

  QValueList<ThemeInfo>::ConstIterator it;

  for (it = themeInfoList_.begin(); it != themeInfoList_.end(); ++it)
  {
    ThemeInfo info(*it);

    if (info.libraryName() == "libkwindefault")
    {
      lb_themes_->setCurrentItem(i);
      break;
    }

    ++i;
  }

  emit changed(savedLibraryName() != selectedLibraryName());
}

void KWinThemeModule::save()
{
  if (savedLibraryName() != selectedLibraryName())
  {
    KConfig kwinConfig("kwinrc");

    kwinConfig.setGroup("Style");

    kwinConfig.writeEntry("PluginLib", selectedLibraryName());

    kwinConfig.sync();

    resetKWin();
  }

  emit changed(false);
}

void KWinThemeModule::slotEnable(bool e)
{
  lb_themes_->setEnabled(e);
}

  QString
KWinThemeModule::quickHelp() const
{
  return i18n
    (
     "<h1>Window manager theme</h1>"
     "This module allows you to choose the style of window borders."
    );
}

  void
KWinThemeModule::resetKWin() const
{
  bool ok = kapp->dcopClient()
    ->send("kwin", "KWinInterface", "reconfigure()", QByteArray());

  if (!ok)
    kdDebug() << "kcmkwintheme: Couldn't reconfigure kwin." << endl; 
}

  QString
KWinThemeModule::savedLibraryName() const
{
  KConfig kwinConfig("kwinrc");

  kwinConfig.setGroup("Style");

  QString currentLibraryName = kwinConfig.readEntry("PluginLib");

  if (currentLibraryName.isEmpty())
    currentLibraryName = "kwindefault";

  return currentLibraryName;
}

  void
KWinThemeModule::slotSelectionChanged()
{
  emit(changed(savedLibraryName() != selectedLibraryName()));
}

  QString
KWinThemeModule::selectedLibraryName() const
{
  int selectedItem = lb_themes_->currentItem();

  if (-1 == selectedItem)
    return QString::null;

  return themeInfoList_[selectedItem].libraryName();
}

#include "kwintheme.moc"
