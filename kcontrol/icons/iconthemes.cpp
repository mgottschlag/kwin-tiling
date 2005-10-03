/**
 *  Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <config.h>

#include <stdlib.h>
#include <unistd.h>

#include <qfileinfo.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <Q3Frame>
#include <QHBoxLayout>

#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#undef Unsorted
#include <kipc.h>

#include <klistview.h>
#include <kurlrequesterdlg.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kiconloader.h>

#include <kio/job.h>
#include <kio/netaccess.h>
#include <ktar.h>

#ifdef HAVE_LIBART
#include <ksvgiconengine.h>
#endif

#include "iconthemes.h"

IconThemesConfig::IconThemesConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *topLayout = new QVBoxLayout(this, KDialog::marginHint(),
                                           KDialog::spacingHint());

  Q3Frame *m_preview=new Q3Frame(this);
  m_preview->setMinimumHeight(50);

  QHBoxLayout *lh2=new QHBoxLayout( m_preview );
  m_previewExec=new QLabel(m_preview);
  m_previewExec->setPixmap(DesktopIcon("exec"));
  m_previewFolder=new QLabel(m_preview);
  m_previewFolder->setPixmap(DesktopIcon("folder"));
  m_previewDocument=new QLabel(m_preview);
  m_previewDocument->setPixmap(DesktopIcon("document"));

  lh2->addStretch(10);
  lh2->addWidget(m_previewExec);
  lh2->addStretch(1);
  lh2->addWidget(m_previewFolder);
  lh2->addStretch(1);
  lh2->addWidget(m_previewDocument);
  lh2->addStretch(10);


  m_iconThemes=new KListView(this/*"IconThemeList"*/);
  m_iconThemes->addColumn(i18n("Name"));
  m_iconThemes->addColumn(i18n("Description"));
  m_iconThemes->setAllColumnsShowFocus( true );
  m_iconThemes->setFullWidth(true);
  connect(m_iconThemes,SIGNAL(selectionChanged(Q3ListViewItem *)),
		SLOT(themeSelected(Q3ListViewItem *)));

  QPushButton *installButton=new QPushButton( i18n("Install New Theme..."),
	this, "InstallNewTheme");
  connect(installButton,SIGNAL(clicked()),SLOT(installNewTheme()));
  m_removeButton=new QPushButton( i18n("Remove Theme"),
	this, "RemoveTheme");
  connect(m_removeButton,SIGNAL(clicked()),SLOT(removeSelectedTheme()));

  topLayout->addWidget(
	new QLabel(i18n("Select the icon theme you want to use:"), this));
  topLayout->addWidget(m_preview);
  topLayout->addWidget(m_iconThemes);
  QHBoxLayout *lg = new QHBoxLayout(topLayout, KDialog::spacingHint());
  lg->addWidget(installButton);
  lg->addWidget(m_removeButton);

  loadThemes();

  m_defaultTheme=iconThemeItem(KIconTheme::current());
  m_iconThemes->setSelected(m_defaultTheme, true);
  updateRemoveButton();

  load();

  m_iconThemes->setFocus();
}

IconThemesConfig::~IconThemesConfig()
{
}

Q3ListViewItem *IconThemesConfig::iconThemeItem(const QString &name)
{
  Q3ListViewItem *item;
  for ( item=m_iconThemes->firstChild(); item ; item=item->nextSibling() )
    if (m_themeNames[item->text(0)]==name) return item;

  return 0L;
}

void IconThemesConfig::loadThemes()
{
  m_iconThemes->clear();
  m_themeNames.clear();
  QStringList themelist(KIconTheme::list());
  QString name;
  QString tname;
  QStringList::Iterator it;
  for (it=themelist.begin(); it != themelist.end(); ++it)
  {
    KIconTheme icontheme(*it);
    if (!icontheme.isValid()) kdDebug() << "notvalid\n";
    if (icontheme.isHidden()) continue;

    name=icontheme.name();
    tname=name;

 //  Just in case we have duplicated icon theme names on separate directories
    for (int i=2; m_themeNames.find(tname)!=m_themeNames.end() ; i++)
        tname=QString("%1-%2").arg(name).arg(i);

    m_iconThemes->insertItem(new Q3ListViewItem(m_iconThemes,name,
		icontheme.description()));

    m_themeNames.insert(name,*it);

  }
}

void IconThemesConfig::installNewTheme()
{
  KURL themeURL = KURLRequesterDlg::getURL(QString::null, this,
                                           i18n("Drag or Type Theme URL"));
  kdDebug() << themeURL.prettyURL() << endl;

  if (themeURL.url().isEmpty()) return;

  QString themeTmpFile;
  // themeTmpFile contains the name of the downloaded file

  if (!KIO::NetAccess::download(themeURL, themeTmpFile, this)) {
    QString sorryText;
    if (themeURL.isLocalFile())
       sorryText = i18n("Unable to find the icon theme archive %1.");
    else
       sorryText = i18n("Unable to download the icon theme archive;\n"
                        "please check that address %1 is correct.");
    KMessageBox::sorry(this, sorryText.arg(themeURL.prettyURL()));
    return;
  }

  QStringList themesNames = findThemeDirs(themeTmpFile);
  if (themesNames.isEmpty()) {
    QString invalidArch(i18n("The file is not a valid icon theme archive."));
    KMessageBox::error(this, invalidArch);

    KIO::NetAccess::removeTempFile(themeTmpFile);
    return;
  }

  if (!installThemes(themesNames, themeTmpFile)) {
    //FIXME: make me able to know what is wrong....
    // QStringList instead of bool?
    QString somethingWrong =
        i18n("A problem occurred during the installation process; "
             "however, most of the themes in the archive have been installed");
    KMessageBox::error(this, somethingWrong);
  }

  KIO::NetAccess::removeTempFile(themeTmpFile);

  KGlobal::instance()->newIconLoader();
  loadThemes();

  Q3ListViewItem *item=iconThemeItem(KIconTheme::current());
  m_iconThemes->setSelected(item, true);
  updateRemoveButton();
}

bool IconThemesConfig::installThemes(const QStringList &themes, const QString &archiveName)
{
  bool everythingOk = true;
  QString localThemesDir(locateLocal("icon", "./"));

  KProgressDialog progressDiag(this, "themeinstallprogress",
                               i18n("Installing icon themes"),
                               QString::null,
                               true);
  progressDiag.setAutoClose(true);
  progressDiag.progressBar()->setTotalSteps(themes.count());
  progressDiag.show();

  KTar archive(archiveName);
  archive.open(QIODevice::ReadOnly);
  kapp->processEvents();

  const KArchiveDirectory* rootDir = archive.directory();

  KArchiveDirectory* currentTheme;
  for (QStringList::ConstIterator it = themes.begin();
       it != themes.end();
       ++it) {
    progressDiag.setLabel(
        i18n("<qt>Installing <strong>%1</strong> theme</qt>")
        .arg(*it));
    kapp->processEvents();

    if (progressDiag.wasCancelled())
      break;

    currentTheme = dynamic_cast<KArchiveDirectory*>(
                     const_cast<KArchiveEntry*>(
                       rootDir->entry(*it)));
    if (currentTheme == NULL) {
      // we tell back that something went wrong, but try to install as much
      // as possible
      everythingOk = false;
      continue;
    }

    currentTheme->copyTo(localThemesDir + *it);
    progressDiag.progressBar()->advance(1);
  }

  archive.close();
  return everythingOk;
}

QStringList IconThemesConfig::findThemeDirs(const QString &archiveName)
{
  QStringList foundThemes;

  KTar archive(archiveName);
  archive.open(QIODevice::ReadOnly);
  const KArchiveDirectory* themeDir = archive.directory();

  KArchiveEntry* possibleDir = 0L;
  KArchiveDirectory* subDir = 0L;

  // iterate all the dirs looking for an index.theme or index.desktop file
  QStringList entries = themeDir->entries();
  for (QStringList::Iterator it = entries.begin();
       it != entries.end();
       ++it) {
    possibleDir = const_cast<KArchiveEntry*>(themeDir->entry(*it));
    if (possibleDir->isDirectory()) {
      subDir = dynamic_cast<KArchiveDirectory*>( possibleDir );
      if (subDir && (subDir->entry("index.theme") != NULL ||
                     subDir->entry("index.desktop") != NULL))
        foundThemes.append(subDir->name());
    }
  }

  archive.close();
  return foundThemes;
}

void IconThemesConfig::removeSelectedTheme()
{
  Q3ListViewItem *selected = m_iconThemes->selectedItem();
  if (!selected)
     return;

  QString question=i18n("<qt>Are you sure you want to remove the "
        "<strong>%1</strong> icon theme?<br>"
        "<br>"
        "This will delete the files installed by this theme.</qt>").
	arg(selected->text(0));

  bool deletingCurrentTheme=(selected==iconThemeItem(KIconTheme::current()));

  int r=KMessageBox::warningContinueCancel(this,question,i18n("Confirmation"),KStdGuiItem::del());
  if (r!=KMessageBox::Continue) return;

  KIconTheme icontheme(m_themeNames[selected->text(0)]);

  // delete the index file before the async KIO::del so loadThemes() will
  // ignore that dir.
  unlink(QFile::encodeName(icontheme.dir()+"/index.theme").data());
  unlink(QFile::encodeName(icontheme.dir()+"/index.desktop").data());
  KIO::del(KURL( icontheme.dir() ));

  KGlobal::instance()->newIconLoader();

  loadThemes();

  Q3ListViewItem *item=0L;
  //Fallback to the default if we've deleted the current theme
  if (!deletingCurrentTheme)
     item=iconThemeItem(KIconTheme::current());
  if (!item)
     item=iconThemeItem(KIconTheme::defaultThemeName());

  m_iconThemes->setSelected(item, true);
  updateRemoveButton();

  if (deletingCurrentTheme) // Change the configuration
    save();
}

void IconThemesConfig::updateRemoveButton()
{
  Q3ListViewItem *selected = m_iconThemes->selectedItem();
  bool enabled = false;
  if (selected)
  {
    KIconTheme icontheme(m_themeNames[selected->text(0)]);
    QFileInfo fi(icontheme.dir());
    enabled = fi.isWritable();
    // Don't let users remove the current theme.
    if(m_themeNames[selected->text(0)] == KIconTheme::current() || 
			 m_themeNames[selected->text(0)] == KIconTheme::defaultThemeName())
      enabled = false;
  }
  m_removeButton->setEnabled(enabled);
}

void IconThemesConfig::themeSelected(Q3ListViewItem *item)
{
#ifdef HAVE_LIBART
  KSVGIconEngine engine;
#endif 
  QString dirName(m_themeNames[item->text(0)]);
  KIconTheme icontheme(dirName);
  if (!icontheme.isValid()) kdDebug() << "notvalid\n";

  updateRemoveButton();
  const int size = icontheme.defaultSize(KIcon::Desktop);

  KIcon icon=icontheme.iconPath("exec.png", size, KIcon::MatchBest);
  if (!icon.isValid()) {
#ifdef HAVE_LIBART
	  icon=icontheme.iconPath("exec.svg", size, KIcon::MatchBest);
	  if(engine.load(size, size, icon.path))
              m_previewExec->setPixmap(QPixmap(*engine.image()));
          else {
              icon=icontheme.iconPath("exec.svgz", size, KIcon::MatchBest);
              if(engine.load(size, size, icon.path))
                  m_previewExec->setPixmap(QPixmap(*engine.image()));
          }
#endif
  }
  else
          m_previewExec->setPixmap(QPixmap(icon.path));

  icon=icontheme.iconPath("folder.png",size,KIcon::MatchBest);
  if (!icon.isValid()) {
#ifdef HAVE_LIBART
	  icon=icontheme.iconPath("folder.svg", size, KIcon::MatchBest);
	  if(engine.load(size, size, icon.path))
              m_previewFolder->setPixmap(QPixmap(*engine.image()));
          else {
              icon=icontheme.iconPath("folder.svgz", size, KIcon::MatchBest);
              if(engine.load(size, size, icon.path))
                  m_previewFolder->setPixmap(QPixmap(*engine.image()));
          }
#endif
  }
  else
  	  m_previewFolder->setPixmap(QPixmap(icon.path));

  icon=icontheme.iconPath("txt.png",size,KIcon::MatchBest);
  if (!icon.isValid()) {
#ifdef HAVE_LIBART
	  icon=icontheme.iconPath("txt.svg", size, KIcon::MatchBest);
	  if(engine.load(size, size, icon.path))
              m_previewDocument->setPixmap(QPixmap(*engine.image()));
          else {
              icon=icontheme.iconPath("txt.svgz", size, KIcon::MatchBest);
              if(engine.load(size, size, icon.path))
                  m_previewDocument->setPixmap(QPixmap(*engine.image()));
          }
#endif
  }
  else  
	  m_previewDocument->setPixmap(QPixmap(icon.path));
  
  emit changed(true);
  m_bChanged = true;
}

void IconThemesConfig::load()
{
  emit changed(false);
  m_bChanged = false;
}

void IconThemesConfig::save()
{
  if (!m_bChanged)
     return;
  Q3ListViewItem *selected = m_iconThemes->selectedItem();
  if (!selected)
     return;

  KSimpleConfig *config = new KSimpleConfig("kdeglobals", false);
  config->setGroup("Icons");

  config->writeEntry("Theme", m_themeNames[selected->text(0)]);
  KIconTheme::reconfigure();
  emit changed(false);

  delete config;

  for (int i=0; i<KIcon::LastGroup; i++)
  {
    // TODO BUG  KAPP::KIPC::IconChanged is _NEVER_ getting called WHY?
    KIPC::sendMessageAll(KIPC::IconChanged, i);
  }

  KService::rebuildKSycoca(this);

  m_bChanged = false;
  m_removeButton->setEnabled(false);
}

void IconThemesConfig::defaults()
{
  if (m_iconThemes->currentItem()==m_defaultTheme) return;

  m_iconThemes->setSelected(m_defaultTheme, true);
  updateRemoveButton();

  emit changed(true);
  m_bChanged = true;
}

#include "iconthemes.moc"
