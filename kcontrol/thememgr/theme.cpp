/*
 * theme.h
 *
 * Copyright (c) 1998 Stefan Taferner <taferner@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstrlist.h>
#include <kapp.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qwindowdefs.h>

#include <dcopclient.h>

#include <kapp.h>
#include <kconfigbackend.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdesktopfile.h>
#include <kicontheme.h>
#include <kipc.h>
#include <kdebug.h>
#include "theme.h"

#include <kio/netaccess.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

extern int dropError(Display *, XErrorEvent *);

//-----------------------------------------------------------------------------
Theme::Theme(): ThemeInherited(QString::null), mInstFiles()
{
  int len;

  setLocale();

  instOverwrite = false;

  mConfigDir = KGlobal::dirs()->saveLocation("config");
  len = mConfigDir.length();
  if (len > 0 && mConfigDir[len-1] != '/') mConfigDir += '/';

  mMappings = NULL;
  loadMappings();

  loadSettings();
}


//-----------------------------------------------------------------------------
Theme::~Theme()
{
  saveSettings();
  if (mMappings) delete mMappings;
}


//-----------------------------------------------------------------------------
void Theme::loadSettings(void)
{
  KConfig* cfg = kapp->config();

  cfg->setGroup("Install");
  mRestartCmd = cfg->readEntry("restart-cmd",
			       "kill `pidof %s`; %s >/dev/null 2>&1 &");
}


//-----------------------------------------------------------------------------
void Theme::saveSettings(void)
{
}


//-----------------------------------------------------------------------------
void Theme::setDescription(const QString aDescription)
{
  mDescription = aDescription;
}


//-----------------------------------------------------------------------------
void Theme::setName(const char *_name)
{
    QString aName = _name;
    QString mName;
    int i = aName.findRev('/');
    if (i>0) 
	QObject::setName(aName.mid(i+1, 1024).ascii());
    else 
	QObject::setName((mName = aName).ascii());
    
    if (aName[0]=='/') 
	mFileName = aName;
    else 
	mFileName = workDir() + aName;
}

//-----------------------------------------------------------------------------
const QString Theme::workDir(void)
{
  static QString str;
  if (str.isEmpty())
    str = locateLocal("data", "kthememgr/Work/");
  return str;
}


//-----------------------------------------------------------------------------
void Theme::loadMappings()
{
  QFile file;

  file.setName(locate("data", "kthememgr/theme.mappings"));
  if (!file.exists())
  {
     fatal(i18n("Mappings file theme.mappings not found.").ascii());
  }

  if (mMappings) delete mMappings;
  mMappings = new KSimpleConfig(file.name(), true);
}


//-----------------------------------------------------------------------------
void Theme::cleanupWorkDir(void)
{
  QString cmd;
  int rc;

  // cleanup work directory
  cmd.sprintf("rm -rf %s*", workDir().ascii());
  rc = system(cmd.ascii());
  if (rc) warning(i18n("Error during cleanup of work directory: rc=%d\n%s").ascii(),
		  rc, cmd.ascii());
}


//-----------------------------------------------------------------------------
bool Theme::load(const QString aPath, QString &error)
{
  QString cmd, str;
  QFileInfo finfo(aPath);
  int rc, num, i;

  assert(!aPath.isEmpty());
  kdDebug() << "Theme::load()" << endl;

  clear();
  cleanupWorkDir();
  setName(aPath.ascii());

  if (finfo.isDir())
  {
    // The theme given is a directory. Copy files over into work dir.

    i = aPath.findRev('/');
    if (i >= 0) str = workDir() + aPath.mid(i, 1024);
    else str = workDir();

    cmd = QString("cp -r \"%1\" \"%2\"").arg(aPath).arg(str);
    kdDebug() << cmd << endl;
    rc = system(QFile::encodeName(cmd).data());
    if (rc)
    {
      error = i18n("Theme contents could not be copied from\n%1\ninto\n%2")
		.arg(aPath).arg(str);      
      return false;
    }
  }
  else
  {
    // The theme given is a tar package. Unpack theme package.
    cmd = QString("cd \"%1\"; gzip -c -d \"%2\" | tar xf -")
             .arg(workDir()).arg(aPath);
    kdDebug() << cmd << endl;
    rc = system(QFile::encodeName(cmd).data());
    if (rc)
    {
      error = i18n("Theme contents could not be extracted from\n%1\ninto\n%1")
                .arg(aPath).arg(workDir());
      return false;
    }
  }

  // Let's see if the theme is stored in a subdirectory.
  QDir dir(workDir(), QString::null, QDir::Name, QDir::Files|QDir::Dirs);
  for (i=0, mThemePath=QString::null, num=0; dir[i]!=0; i++)
  {
    if (dir[i][0]=='.') continue;
    finfo.setFile(workDir() + dir[i]);
    if (!finfo.isDir()) break;
    mThemePath = dir[i];
    num++;
  }
  if (num==1) mThemePath = workDir() + mThemePath + '/';
  else mThemePath = workDir();

  // Search for the themerc file
  dir.setNameFilter("*.themerc");
  dir.setPath(mThemePath);
  mThemercFile = dir[0];
  if (mThemercFile.isEmpty())
  {
    error = i18n("Theme does not contain a .themerc file.");
    return false;
  }
  mThemercFile = mThemePath+mThemercFile;

  // Search for the preview image
  dir.setNameFilter("*.preview.*");
  mPreviewFile = dir[0];
  mPreviewFile = mThemePath+mPreviewFile;

  // read theme config file
  setReadOnly(TRUE);
  backEnd->changeFileName(mThemercFile, "", false);
  reparseConfiguration();

  readConfig();

  emit changed();
  return true;
}


//-----------------------------------------------------------------------------
bool Theme::save(const QString aPath)
{
  QString cmd;
  int rc;

  emit apply();
  writeConfig();

  backEnd->changeFileName(mThemercFile, "", false);
  backEnd->sync(true); // true so that disk entries are merged.  Is this right?

  if (stricmp(aPath.right(4).ascii(), ".tgz") == 0 ||
      stricmp(aPath.right(7).ascii(), ".tar.gz") == 0)
  {
    cmd.sprintf("cd \"%s\";tar cf - *|gzip -c >\"%s\"",
		workDir().ascii(), aPath.ascii());
  }
  else
  {
    cmd.sprintf("cd \"%s\"; rm -rf \"%s\"; cp -r * \"%s\"",
		workDir().ascii(), aPath.ascii(),
		aPath.ascii());
  }

  kdDebug() << cmd << endl;
  rc = system(cmd.ascii());
  if (rc) kdDebug() << "Failed to save theme to " << aPath << " with command " << cmd << endl;

  return (rc==0);
}


//-----------------------------------------------------------------------------
void Theme::removeFile(const QString& aName, const QString aDirName)
{
  if (aName.isEmpty()) return;

  if (aName[0] == '/' || aDirName.isEmpty())
    unlink(aName.ascii());
  else if (aDirName[aDirName.length()-1] == '/')
    unlink((aDirName + aName).ascii());
  else unlink((aDirName + '/' + aName).ascii());
}


//-----------------------------------------------------------------------------
bool Theme::installFile(const QString& aSrc, const QString& aDest)
{
  QString cmd, dest, src;
  QFileInfo finfo;
  QFile srcFile, destFile;
  int len, i;
  char buf[32768];
  bool backupMade = false;

  if (aSrc.isEmpty()) return true;

  assert(aDest[0] == '/');
  dest = aDest;

  src = mThemePath + aSrc;

  finfo.setFile(src);
  if (!finfo.exists())
  {
    kdDebug() << "File " << aSrc << " is not in theme package." << endl;
    return false;
  }

  if (finfo.isDir())
  {
    kdDebug() << aSrc << " is a direcotry instead of a file." << endl;
    return false;
  }

  finfo.setFile(dest);
  if (finfo.isDir())  // destination is a directory
  {
    len = dest.length();
    if (dest[len-1]=='/') dest[len-1] = '\0';
    i = src.findRev('/');
    dest = dest + '/' + src.mid(i+1,1024);
    finfo.setFile(dest);
  }

  if (!instOverwrite && finfo.exists()) // make backup copy
  {
    unlink((dest+'~').ascii());
    rename(dest.ascii(), (dest+'~').ascii());
    backupMade = true;
  }

  srcFile.setName(src);
  if (!srcFile.open(IO_ReadOnly))
  {
    warning(i18n("Cannot open file %s for reading").ascii(), src.ascii());
    return false;
  }

  destFile.setName(dest);
  if (!destFile.open(IO_WriteOnly))
  {
    warning(i18n("Cannot open file %s for writing").ascii(), dest.ascii());
    return false;
  }

  while (!srcFile.atEnd())
  {
    len = srcFile.readBlock(buf, 32768);
    if (len <= 0) break;
    if (destFile.writeBlock(buf, len) != len)
    {
      warning(i18n("Write error to %s:\n%s").ascii(), dest.ascii(),
	      strerror(errno));
      return false;
    }
  }

  srcFile.close();
  destFile.close();

  addInstFile(dest.ascii());
  kdDebug() << "Installed " << src << " to " << dest << ". Backup: backupMade" << endl;

  return true;
}

//-----------------------------------------------------------------------------
bool Theme::installDirectory(const QString& aSrc, const QString& aDest)
{
  bool backupMade = false; // Not used ??

  if (aSrc.isEmpty()) return true;

  assert(aDest[0] == '/');
  QString dest = aDest;

  QString src = mThemePath + aSrc;

  QFileInfo finfo(src);
  if (!finfo.exists())
  {
    kdDebug() << "Directory " << aSrc << " is not in theme package." << endl;
    return false;
  }
  if (!finfo.isDir())
  {
    kdDebug() << aSrc << " is not a directory." << endl;
    return false;
  }

  if (finfo.exists()) // delete and/or make backup copy
  {
    if (instOverwrite)
    {
       KURL url;
       url.setPath(dest);
       KIO::NetAccess::del(url);
    }
    else
    {
       KURL url;
       url.setPath(dest+'~');
       KIO::NetAccess::del(url);
       rename(QFile::encodeName(dest).data(), QFile::encodeName(dest+'~').data());
       backupMade = true;
    }
  }

  KURL url1;
  KURL url2;
  url1.setPath(src);
  url2.setPath(dest);

  KIO::NetAccess::dircopy(url1, url2);

  addInstFile(dest.ascii());
  kdDebug() << "Installed " << src << " to " << dest << ". Backup: backupMade" << endl;

  return true;
}


//-----------------------------------------------------------------------------
int Theme::installGroup(const char* aGroupName)
{
  QString value, oldValue, cfgFile, cfgGroup, appDir, group, emptyValue;
  QString oldCfgFile, key, cfgKey, cfgValue, themeValue, instCmd, baseDir;
  QString preInstCmd;
  bool absPath = false;
  KSimpleConfig* cfg = NULL;
  int len, i, installed = 0;
  const char* missing = 0;

  baseDir = KGlobal::dirs()->saveLocation("config");
  baseDir.truncate(baseDir.length()-7);
kdDebug() << ":: baseDir = " << baseDir << endl;

  kdDebug() << "*** beginning with " << aGroupName << endl;
  group = aGroupName;
  setGroup(group);
  
  if (!instOverwrite) uninstallFiles(aGroupName);
  else readInstFileList(aGroupName);

  while (!group.isEmpty())
  {
    mMappings->setGroup(group);
    kdDebug() << "Mappings group: " << group << endl;

    // Read config settings
    value = mMappings->readEntry("ConfigFile");
    if (!value.isEmpty())
    {
      cfgFile = value;
      if (cfgFile == "KDERC") cfgFile = QDir::homeDirPath() + "/.kderc";
      else if (cfgFile[0] != '/') cfgFile = mConfigDir + cfgFile;
    }
    value = mMappings->readEntry("ConfigGroup");
    if (!value.isEmpty()) cfgGroup = value;
    value = mMappings->readEntry("ConfigAppDir");
    if (!value.isEmpty() && (value[0] != '/'))
    {
      appDir = value;
      appDir = baseDir + appDir;

      len = appDir.length();
      if (len > 0 && appDir[len-1]!='/') appDir += '/';
    }
    absPath = mMappings->readBoolEntry("ConfigAbsolutePaths", absPath);
    value = mMappings->readEntry("ConfigEmpty");
    if (!value.isEmpty()) emptyValue = value;
    value = mMappings->readEntry("ConfigActivateCmd");
    if (!value.isEmpty() && (mCmdList.findIndex(value) < 0))
      mCmdList.append(value);
    instCmd = mMappings->readEntry("ConfigInstallCmd").stripWhiteSpace();
    preInstCmd = mMappings->readEntry("ConfigPreInstallCmd").stripWhiteSpace();

    // Some checks
    if (cfgFile.isEmpty()) missing = "ConfigFile";
    if (cfgGroup.isEmpty()) missing = "ConfigGroup";
    if (missing)
    {
      warning(i18n("Internal error in theme mappings\n"
		   "(file theme.mappings) in group %s:\n\n"
		   "Entry `%s' is missing or has no value.").ascii(),
	      group.ascii(), missing);
      break;
    }

    // Open config file and sync/close old one
    if (oldCfgFile != cfgFile)
    {
      if (cfg)
      {
	kdDebug() << "closing config file" << endl;
	cfg->sync();
	delete cfg;
      }
      kdDebug() << "opening config file " << cfgFile << endl;
      cfg = new KSimpleConfig(cfgFile);
      oldCfgFile = cfgFile;
    }

    // Set group in config file
    cfg->setGroup(cfgGroup);
    kdDebug() << cfgFile << ": " << cfgGroup << endl;

    // Execute pre-install command (if given)
    if (!preInstCmd.isEmpty()) preInstallCmd(cfg, preInstCmd);

    // Process all mapping entries for the group
    QMap<QString, QString> aMap = mMappings->entryMap(group);
    QMap<QString, QString>::Iterator aIt(aMap.begin());

    for (; aIt != aMap.end(); ++aIt) {
      key = aIt.key();
      if (stricmp(key.left(6).ascii(),"Config")==0) continue;
      value = (*aIt).stripWhiteSpace();
      len = value.length();
      bool bInstallFile = false;
      bool bInstallDir = false;
      if (len>0 && value[len-1]=='!')
      {
	value.truncate(len - 1);
      }
      else if (len>0 && value[len-1]=='*')
      {
	value.truncate(len - 1);
        bInstallDir = true;
      }
      else 
      {
        bInstallFile = true;
      }

      // parse mapping
      i = value.find(':');
      if (i >= 0)
      {
	cfgKey = value.left(i);
	cfgValue = value.mid(i+1, 1024);
      }
      else 
      {
	cfgKey = value;
	cfgValue = QString::null;
      }
      if (cfgKey.isEmpty()) cfgKey = key;

      if (bInstallFile || bInstallDir)
      {
	oldValue = cfg->readEntry(cfgKey);
	if (!oldValue.isEmpty() && oldValue==emptyValue)
	  oldValue = QString::null;
      }
      else 
      {
         oldValue = QString::null;
      }

      themeValue = readEntry(key);
      if (cfgValue.isEmpty()) cfgValue = themeValue;

      // Install file
      if (bInstallFile)
      {
	if (!themeValue.isEmpty())
	{
          KStandardDirs::makeDir(appDir);
	  if (installFile(themeValue, appDir + cfgValue))
	    installed++;
	  else bInstallFile = false;
	}
      }
      // Install dir
      if (bInstallDir)
      {
	if (!themeValue.isEmpty())
	{
          KStandardDirs::makeDir(appDir);
	  if (installDirectory(themeValue, appDir + cfgValue))
	    installed++;
	  else bInstallDir = false;
	}
      }

      // Determine config value
      if (cfgValue.isEmpty()) 
         cfgValue = emptyValue;
      else if ((bInstallFile || bInstallDir) && absPath) 
         cfgValue = appDir + cfgValue;
 
      // Set config entry
      kdDebug() << cfgKey << "=" << cfgValue << endl;
      if (cfgKey == "-") 
         cfg->deleteEntry(key, false);
      else 
         cfg->writeEntry(cfgKey, cfgValue);
    }

    if (!instCmd.isEmpty()) 
       installCmd(cfg, instCmd, installed);
    group = mMappings->readEntry("ConfigNextGroup");
  }

  if (cfg)
  {
    kdDebug() << "closing config file" << endl;
    cfg->sync();
    delete cfg;
  }

  writeInstFileList(aGroupName);
  kdDebug() << "*** done with " << aGroupName << endl;
  return installed;
}


//-----------------------------------------------------------------------------
void Theme::preInstallCmd(KSimpleConfig* aCfg, const QString& aCmd)
{
  QString grp = aCfg->group();
  QString value, cmd;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "stretchBorders")
  {
    value = readEntry("ShapePixmapBottom");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, false);
    value = readEntry("ShapePixmapTop");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, false);
    value = readEntry("ShapePixmapLeft");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, true);
    value = readEntry("ShapePixmapRight");
    if (!value.isEmpty()) stretchPixmap(mThemePath + value, true);
  }
  else
  {
    warning(i18n("Unknown pre-install command `%s' in\n"
		 "theme.mappings file in group %s.").ascii(), 
	    aCmd.ascii(), mMappings->group().ascii());
  }
}


//-----------------------------------------------------------------------------
void Theme::installCmd(KSimpleConfig* aCfg, const QString& aCmd,
		       int aInstalled)
{
  QString grp = aCfg->group();
  QString value, cmd;
  bool flag;

  cmd = aCmd.stripWhiteSpace();

  if (cmd == "winShapeMode")
  {
    aCfg->writeEntry("ShapeMode", aInstalled ? "on" : "off");
  }
  else if (cmd == "winTitlebar")
  {
    if (hasKey("TitlebarPixmapActive") || hasKey("TitlebarPixmapInactive"))
      value = "pixmap";
    else if (aCfg->readEntry("TitlebarLook") == "pixmap")
      value = "shadedHorizontal";
    else value = QString::null;
    if (!value.isEmpty()) aCfg->writeEntry("TitlebarLook", value);
  }
  else if (cmd == "winGimmickMode")
  {
    if (hasKey("Pixmap")) value = "on";
    else value = "off";
    aCfg->writeEntry("GimmickMode", value);
  }
  /*CT 17Jan1999 no more needed - handled by the kpanel now 
  else if (cmd == "panelBack")
  {
    value = aCfg->readEntry("Position");
    if (stricmp(value,"right")==0 || stricmp(value,"left")==0)
    {
      value = aCfg->readEntry("BackgroundTexture");
      rotateImage(kapp->localkdedir()+"/share/apps/kpanel/pics/"+value, -90);
    }
  }
  */
  else if (cmd == "setWallpaperMode")
  {
    value = aCfg->readEntry("wallpaper",QString::null);
    aCfg->writeEntry("UseWallpaper", !value.isEmpty());
  }
  else if (cmd == "oneDesktopMode")
  {
    flag = (aInstalled==1);
    aCfg->writeEntry("CommonDesktop",  flag);
    if (flag) aCfg->writeEntry("DeskNum", 0);
  }
  else
  {
    warning(i18n("Unknown command `%s' in theme.mappings\n"
		 "file in group %s.").ascii(), aCmd.ascii(),
	    mMappings->group().ascii());
  }

  if (stricmp(aCfg->group().ascii(), grp.ascii())!=0) 
    aCfg->setGroup(grp);
}

void Theme::applyIcons()
{
  KSimpleConfig *config = new KSimpleConfig("kdeglobals", false);

  config->setGroup("Icons");

  QString theme = config->readEntry("Theme");

  KIconTheme icontheme(theme);

  const char *groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", 0L };

  for (int i=0; i<KIcon::LastGroup; i++)
  {
    if (groups[i] == 0L)
      break;
    config->setGroup(QString::fromLatin1(groups[i]) + "Icons");
    config->writeEntry("Size", icontheme.defaultSize(i));
  } 
  delete config;
  
  for (int i=0; i<KIcon::LastGroup; i++)
  {
    KIPC::sendMessageAll(KIPC::IconChanged, i);
  } 

}


//-----------------------------------------------------------------------------
void Theme::doCmdList(void)
{
  QString cmd, str, appName;
  //  int rc;
  for (QStringList::ConstIterator it = mCmdList.begin();
       it != mCmdList.end();
       ++it)
  {
    cmd = *it;
    kdDebug() << "do command: " << cmd << endl;
    if (cmd.startsWith("kfmclient"))
    {
      system(cmd.ascii());
    }
    else if (cmd == "applyColors")
    {
      colorSchemeApply();
      runKrdb();
    }
    else if (cmd == "applyWallpaper")
    {
       // reconfigure kdesktop. kdesktop will notify all clients
       DCOPClient *client = kapp->dcopClient();
       if (!client->isAttached())
          client->attach();
       client->send("kdesktop", "KBackgroundIface", "configure()", "");
    }
    else if (cmd == "applyIcons")
    {
       applyIcons();
    }
    else if (cmd.startsWith("restart"))
    {
      appName = cmd.mid(7).stripWhiteSpace();
      str.sprintf(i18n("Restart %s to activate the new settings?").ascii(),
		  appName.ascii());
      if (KMessageBox::questionYesNo(0, str) == KMessageBox::Yes) {
          str.sprintf(mRestartCmd.ascii(), appName.ascii(), 
                      appName.ascii());
          system(str.ascii());
      }
    }
  }

  mCmdList.clear();
}


//-----------------------------------------------------------------------------
bool Theme::backupFile(const QString fname) const
{
  QFileInfo fi(fname);
  QString cmd;
  int rc;

  if (!fi.exists()) return false;

  unlink((fname + '~').ascii());
  cmd.sprintf("mv \"%s\" \"%s~\"", fname.ascii(),
	      fname.ascii());
  rc = system(cmd.ascii());
  if (rc) warning(i18n("Cannot make backup copy of %s: mv returned %d").ascii(),
		  fname.ascii(), rc);
  return (rc==0);
}

//-----------------------------------------------------------------------------
void Theme::addInstFile(const QString &aFileName)
{
  if (!aFileName.isEmpty() && (mInstFiles.findIndex(aFileName) < 0))
    mInstFiles.append(aFileName);
}


//-----------------------------------------------------------------------------
void Theme::readInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->config();

  assert(aGroupName!=0);
  cfg->setGroup("Installed Files");
  mInstFiles = cfg->readListEntry(aGroupName, ':');
}


//-----------------------------------------------------------------------------
void Theme::writeInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->config();

  assert(aGroupName!=0);
  cfg->setGroup("Installed Files");
  cfg->writeEntry(aGroupName, mInstFiles, ':');
}


//-----------------------------------------------------------------------------
void Theme::uninstallFiles(const char* aGroupName)
{
  QString cmd, fname, value;
  QFileInfo finfo;
  bool reverted = false;
  int processed = 0;

  kdDebug() << "*** beginning uninstall of " << aGroupName << endl;

  readInstFileList(aGroupName);
  for (QStringList::ConstIterator it=mInstFiles.begin(); 
       it != mInstFiles.end();
       ++it)
  {
    fname = *it;
    reverted = false;
    if (unlink(QFile::encodeName(fname).data())==0) 
       reverted = true;
    finfo.setFile(fname+'~');
    if (finfo.exists())
    {
      if (rename(QFile::encodeName(fname+'~').data(), QFile::encodeName(fname).data()))
	warning(i18n("Failed to rename %s to %s~:\n%s").ascii(),
		fname.ascii(), fname.ascii(),
		strerror(errno));
      else reverted = true;
    }

    if (reverted) 
    {
      kdDebug() << "uninstalled " << fname << endl;
      processed++;
    }
  }
  mInstFiles.clear();
  writeInstFileList(aGroupName);

  kdDebug() << "*** done uninstall of " << aGroupName << endl;
}


//-----------------------------------------------------------------------------
void Theme::install(void)
{
  kdDebug() << "Theme::install() started" << endl;

  loadMappings();
  mCmdList.clear();

  if (instWallpapers) installGroup("Display");
  if (instColors) installGroup("Colors");
  if (instIcons) installGroup("Icons");

  kdDebug() << "*** executing command list" << endl;

  doCmdList();

  kdDebug() << "Theme::install() done" << endl;
  saveSettings();
}


//-----------------------------------------------------------------------------
void Theme::readCurrent(void)
{
  emit changed();
}


//-----------------------------------------------------------------------------
KConfig* Theme::openConfig(const QString aAppName) const
{
  return new KConfig(aAppName + "rc");
}


//-----------------------------------------------------------------------------
void Theme::readConfig(void)
{
  QString fname;
  QColor col;
  col.setRgb(192,192,192);

  setGroup("General");
  mDescription = readEntry("description", QString(name()) + " Theme");

  setGroup("Colors");
  foregroundColor = readColorEntry(this, "foreground", &col);
  backgroundColor = readColorEntry(this, "background", &col);
  selectForegroundColor = readColorEntry(this, "selectForeground", &col);
  selectBackgroundColor = readColorEntry(this, "selectBackground", &col);
  activeForegroundColor = readColorEntry(this, "activeForeground", &col);
  activeBackgroundColor = readColorEntry(this, "activeBackground", &col);
  activeBlendColor = readColorEntry(this, "activeBlend", &col);
  inactiveForegroundColor = readColorEntry(this, "inactiveForeground", &col);
  inactiveBackgroundColor = readColorEntry(this, "inactiveBackground", &col);
  inactiveBlendColor = readColorEntry(this, "inactiveBlend", &col);
  windowForegroundColor = readColorEntry(this, "windowForeground", &col);
  windowBackgroundColor = readColorEntry(this, "windowBackground", &col);
  contrast = readNumEntry("Contrast", 7);

  if (!mPreviewFile.isEmpty())
  {
    if (!mPreview.load(mPreviewFile))
    {
      warning(i18n("Failed to load preview image %s").ascii(), mPreviewFile.ascii());
      mPreview.resize(0,0);
    }
  }
  else mPreview.resize(0,0);
}


//-----------------------------------------------------------------------------
void Theme::writeConfig(void)
{
  kdDebug() << "Theme::writeConfig() is broken" << endl;
  return;

  setGroup("General");
  writeEntry("description", mDescription);

#ifdef BROKEN
  setGroup("Colors");
  writeColorEntry(this, "BackgroundColor", backgroundColor);
  writeColorEntry(this, "SelectColor", selectColor);
  writeColorEntry(this, "TextColor", textColor);
  writeColorEntry(this, "ActiveTitleTextColor", activeTextColor);
  writeColorEntry(this, "InactiveTitleBarColor", inactiveTitleColor);
  writeColorEntry(this, "ActiveTitleBarColor", activeTitleColor);
  writeColorEntry(this, "InactiveTitleTextColor", inactiveTextColor);
  writeColorEntry(this, "WindowTextColor", windowTextColor);
  writeColorEntry(this, "WindowColor", windowColor);
  writeColorEntry(this, "SelectTextColor", selectTextColor);
  writeEntry("Contrast", contrast);
#endif
}


//-----------------------------------------------------------------------------
void Theme::writeColorEntry(KConfigBase* cfg, const char* aKey, 
			    const QColor& aColor)
{
  QString str;   // !!! Changed from QString(32). It should be safe with Qt2.0 (ce) 
  str.sprintf("#%02x%02x%02x", aColor.red(), aColor.green(), aColor.blue());
  cfg->writeEntry(aKey, str);
}


//-----------------------------------------------------------------------------
const QColor& Theme::readColorEntry(KConfigBase* cfg, const char* aKey,
				    const QColor* aDefault) const
{
  static QColor col;
  QString str = cfg->readEntry(aKey, QString::null);

  if (!str.isEmpty()) col.setNamedColor(str);
  else if (aDefault) col = *aDefault;

  return col;
}


//-----------------------------------------------------------------------------
void Theme::clear(void)
{
  QStringList gList(groupList());

  for (QStringList::Iterator gIt(gList.begin()); gIt != gList.end(); ++gIt) {
    deleteGroup(*gIt);
  }
}


//-----------------------------------------------------------------------------
bool Theme::hasGroup(const QString& aName, bool aNotEmpty)
{
  bool found = KSimpleConfig::hasGroup(aName);

  if (!aNotEmpty)
    return found;

  QMap<QString, QString> aMap = entryMap(aName);
  if (found && aNotEmpty) 
    found = !aMap.isEmpty();

  return found;
}

//-----------------------------------------------------------------------------
void Theme::stretchPixmap(const QString aFname, bool aStretchVert)
{
  QPixmap src, dest;
  QBitmap *srcMask, *destMask;
  int w, h, w2, h2;
  QPainter p;

  src.load(aFname);
  if (src.isNull()) return;

  w = src.width();
  h = src.height();

  if (aStretchVert)
  {
    w2 = w;
    for (h2=h; h2<64; h2=h2<<1)
      ;
  }
  else
  {
    h2 = h;
    for (w2=w; w2<64; w2=w2<<1)
      ;
  }

  dest = src;
  dest.resize(w2, h2);

  kdDebug() << "stretching " << aFname << " from " << w << "x" << h <<
    " to " << w2 << "x" << h2 << endl;

  p.begin(&dest);
  p.drawTiledPixmap(0, 0, w2, h2, src);
  p.end();

  srcMask = (QBitmap*)src.mask();
  if (srcMask)
  {
    destMask = (QBitmap*)dest.mask();
    p.begin(destMask);
    p.drawTiledPixmap(0, 0, w2, h2, *srcMask);
    p.end();
  }

  dest.save(aFname, QPixmap::imageFormat(aFname));
}


//-----------------------------------------------------------------------------
void Theme::rotateImage(const QString aFname, int aAngle)
{
  QPixmap src, dest;
  QWMatrix mx;

  src.load(aFname);
  if (src.isNull()) return;

  mx.rotate(aAngle);
  dest = src.xForm(mx);

  dest.save(aFname, QPixmap::imageFormat(aFname));
}


//-----------------------------------------------------------------------------
void Theme::iconToMiniIcon(const QString aIcon, const QString aMiniIcon)
{
  QPixmap src, dest;
  QWMatrix mx;

  src.load(aIcon);
  if (src.isNull()) return;

  mx.scale(.5, .5);
  dest = src.xForm(mx);

  dest.save(aMiniIcon, QPixmap::imageFormat(aIcon));
}


//-----------------------------------------------------------------------------
void Theme::runKrdb(void) const
{
  KSimpleConfig cfg("kcmdisplayrc", true);

  cfg.setGroup("X11");
  if (cfg.readBoolEntry("useResourceManager", true))
    system("krdb");
}


//-----------------------------------------------------------------------------
void Theme::colorSchemeApply(void)
{
  KIPC::sendMessageAll(KIPC::PaletteChanged);
}


//-----------------------------------------------------------------------------
const QString Theme::fileOf(const QString& aName) const
{
  int i = aName.findRev('/');
  if (i >= 0) return aName.mid(i+1, 1024);
  return aName;
}


//-----------------------------------------------------------------------------
const QString Theme::pathOf(const QString& aName) const
{
  int i = aName.findRev('/');
  if (i >= 0) return aName.left(i);
  return aName;
}


//-----------------------------------------------------------------------------
#include "theme.moc"
