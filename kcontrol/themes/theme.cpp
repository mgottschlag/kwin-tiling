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

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qstrlist.h>
#include <kapp.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qwindowdefs.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <kconfigbackend.h>
#include <kwm.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdesktopfile.h>

#include <sys/stat.h>
#include <assert.h>

#include "theme.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

extern int dropError(Display *, XErrorEvent *);

//-----------------------------------------------------------------------------
Theme::Theme(): ThemeInherited(QString::null), mInstFiles(true)
{
  int len;

  setLocale();

  instOverwrite = false;

  mConfigDir = KGlobal::dirs()->getSaveLocation("config");
  len = mConfigDir.length();
  if (len > 0 && mConfigDir[len-1] != '/') mConfigDir += '/';

  mMappings = NULL;
  loadMappings();

  // ensure that work directory exists
  mkdirhier(workDir().ascii());

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
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Install");
  mRestartCmd = cfg->readEntry("restart-cmd",
			       "kill `pidof %s`; %s >/dev/null 2>&1 &");
  mInstIcons = cfg->readNumEntry("icons");
}


//-----------------------------------------------------------------------------
void Theme::saveSettings(void)
{
  KConfig* cfg = kapp->getConfig();

  cfg->setGroup("Install");
  cfg->writeEntry("icons", mInstIcons);
  cfg->sync();
}


//-----------------------------------------------------------------------------
void Theme::setDescription(const QString aDescription)
{
  mDescription = aDescription.copy();
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
    str = locateLocal("appdata", "Work/");
  return str;
}


//-----------------------------------------------------------------------------
void Theme::loadMappings()
{
  QFile file;

  file.setName(locateLocal("appdata", "theme.mappings"));
  if (!file.exists())
  {
    file.setName("theme.mappings");
    if (!file.exists())
    {
      file.setName(locate("appdata", "theme.mappings"));
      if (!file.exists())
	fatal(i18n("Mappings file theme.mappings not found.").ascii());
    }
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
bool Theme::load(const QString aPath)
{
  QString cmd, str;
  QFileInfo finfo(aPath);
  int rc, num, i;

  assert(!aPath.isEmpty());
  debug("Theme::load()");

  clear();
  cleanupWorkDir();
  setName(aPath.ascii());

  if (finfo.isDir())
  {
    // The theme given is a directory. Copy files over into work dir.

    i = aPath.findRev('/');
    if (i >= 0) str = workDir() + aPath.mid(i, 1024);
    else str = workDir();

    cmd.sprintf("cp -r \"%s\" \"%s\"", aPath.ascii(),
		str.ascii());
    debug(cmd.ascii());
    rc = system(cmd.ascii());
    if (rc)
    {
      warning(i18n("Failed to copy theme contents\nfrom %s\ninto %s").ascii(),
	      aPath.ascii(), str.ascii());
      return false;
    }
  }
  else
  {
    // The theme given is a tar package. Unpack theme package.
    cmd.sprintf("cd \"%s\"; gzip -c -d \"%s\" | tar xf -", 
		workDir().ascii(), aPath.ascii());
    debug(cmd.ascii());
    rc = system(cmd.ascii());
    if (rc)
    {
      warning(i18n("Failed to unpack %s with\n%s").ascii(),
	      aPath.ascii(),
	      cmd.ascii());
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
    warning(i18n("Theme contains no .themerc file").ascii());
    return false;
  }

  // Search for the preview image
  dir.setNameFilter("*.preview.*");
  mPreviewFile = dir[0];

  // read theme config file
  setReadOnly(TRUE);
  backEnd->changeFileName(mThemercFile, false);
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

  backEnd->changeFileName(mThemercFile, false);
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

  debug(cmd.ascii());
  rc = system(cmd.ascii());
  if (rc) debug(i18n("Failed to save theme to\n%s\nwith command\n%s").ascii(),
		aPath.ascii(), cmd.ascii());

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

  if (aDest[0] == '/') dest = aDest;
  dest = locateLocal("appdata", aDest);

  src = mThemePath + aSrc;

  finfo.setFile(src);
  if (!finfo.exists())
  {
    debug("File %s is not in theme package.", aSrc.ascii());
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
  debug("Installed %s to %s %s", src.ascii(), dest.ascii(),
	backupMade ? "(backup of existing file)" : "");

  return true;
}


//-----------------------------------------------------------------------------
int Theme::installGroup(const char* aGroupName)
{
  QString value, oldValue, cfgFile, cfgGroup, appDir, group, emptyValue;
  QString oldCfgFile, key, cfgKey, cfgValue, themeValue, instCmd, baseDir;
  QString preInstCmd;
  bool absPath = false, doInstall;
  KSimpleConfig* cfg = NULL;
  int len, i, installed = 0;
  const char* missing = 0;

  debug("*** beginning with %s", aGroupName);
  group = aGroupName;
  setGroup(group);
  
  if (!instOverwrite) uninstallFiles(aGroupName);
  else readInstFileList(aGroupName);

  while (!group.isEmpty())
  {
    mMappings->setGroup(group);
    debug("Mappings group [%s]", group.ascii());

    // Read config settings
    value = mMappings->readEntry("ConfigFile");
    if (!value.isEmpty())
    {
      cfgFile = value.copy();
      if (cfgFile == "KDERC") cfgFile = QDir::homeDirPath() + "/.kderc";
      else if (cfgFile[0] != '/') cfgFile = mConfigDir + cfgFile;
    }
    value = mMappings->readEntry("ConfigGroup");
    if (!value.isEmpty()) cfgGroup = value.copy();
    value = mMappings->readEntry("ConfigAppDir");
    if (!value.isEmpty())
    {
      appDir = value.copy();
      if (appDir[0] != '/') baseDir = kapp->localkdedir() + "/share/";
      else baseDir = QString::null;

      mkdirhier(appDir.ascii(), baseDir.ascii());
      appDir = baseDir + appDir;

      len = appDir.length();
      if (len > 0 && appDir[len-1]!='/') appDir += '/';
    }
    absPath = mMappings->readBoolEntry("ConfigAbsolutePaths", absPath);
    value = mMappings->readEntry("ConfigEmpty");
    if (!value.isEmpty()) emptyValue = value.copy();
    value = mMappings->readEntry("ConfigActivateCmd");
    if (!value.isEmpty() && mCmdList.find(value.ascii()) < 0)
      mCmdList.append(value.ascii());
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
	debug("closing config file");
	cfg->sync();
	delete cfg;
      }
      debug("opening config file %s", cfgFile.ascii());
      cfg = new KSimpleConfig(cfgFile);
      oldCfgFile = cfgFile;
    }

    // Set group in config file
    cfg->setGroup(cfgGroup);
    debug("%s: [%s]", cfgFile.ascii(), cfgGroup.ascii());

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
      if (len>0 && value[len-1]=='!')
      {
	doInstall = false;
	value.truncate(len - 1);
      }
      else doInstall = true;

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

      if (doInstall)
      {
	oldValue = cfg->readEntry(cfgKey);
	if (!oldValue.isEmpty() && oldValue==emptyValue)
	  oldValue = QString::null;
      }
      else oldValue = QString::null;

      themeValue = readEntry(key);
      if (cfgValue.isEmpty()) cfgValue = themeValue;

      // Install file
      if (doInstall)
      {
	if (!themeValue.isEmpty())
	{
	  if (installFile(themeValue, appDir + cfgValue))
	    installed++;
	  else doInstall = false;
	}
      }

      // Determine config value
      if (cfgValue.isEmpty()) cfgValue = emptyValue;
      else if (doInstall && absPath) cfgValue = appDir + cfgValue;
 
      // Set config entry
      debug("%s=%s", cfgKey.ascii(), cfgValue.ascii());
      if (cfgKey == "-") cfg->deleteEntry(key, false);
      else cfg->writeEntry(cfgKey, cfgValue);
    }

    if (!instCmd.isEmpty()) installCmd(cfg, instCmd, installed);
    group = mMappings->readEntry("ConfigNextGroup");
  }

  if (cfg)
  {
    debug("closing config file");
    cfg->sync();
    delete cfg;
  }

  writeInstFileList(aGroupName);
  debug("*** done with %s", aGroupName);
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
  else if (cmd == "enableSounds")
  {
    aCfg->setGroup("GlobalConfiguration");
    aCfg->writeEntry("EnableSounds", aInstalled>0 ? "Yes" : "No");
  }
  else if (cmd == "setWallpaperMode")
  {
    value = aCfg->readEntry("wallpaper",QString::null);
    aCfg->writeEntry("UseWallpaper", !value.isEmpty());
  }
  else if (cmd == "oneDesktopMode")
  {
    flag = readBoolEntry("CommonDesktop", true);
    flag |= (aInstalled==1);
    aCfg->writeEntry("OneDesktopMode",  flag);
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


//-----------------------------------------------------------------------------
void Theme::doCmdList(void)
{
  QString cmd, str, appName;
  bool kwmRestart = false;
  //  int rc;

  for (cmd=mCmdList.first(); !cmd.isNull(); cmd=mCmdList.next())
  {
    debug("do command: %s", cmd.ascii());
    if (cmd.left(6) == "kwmcom")
    {
      cmd = cmd.mid(6,256).stripWhiteSpace();
      if (stricmp(cmd.ascii(),"restart") == 0) kwmRestart = true;
      else KWM::sendKWMCommand(cmd);
    }
    else if (cmd.left(9) == "kfmclient")
    {
      system(cmd.ascii());
    }
    else if (cmd == "applyColors")
    {
      colorSchemeApply();
      runKrdb();
    }
    else if (strncmp(cmd.ascii(), "restart", 7) == 0)
    {
      appName = cmd.mid(7,256).stripWhiteSpace();
      str.sprintf(i18n("Restart %s to activate the new settings?").ascii(),
		  appName.ascii());
      if (!QMessageBox::information(0,i18n("Theme Setup - Restart Application"), str,
				   i18n("&Yes"), i18n("&No")))
	{
	  str.sprintf(mRestartCmd.ascii(), appName.ascii(), 
		      appName.ascii());
	  system(str.ascii());
	}
    }
  }

  if (kwmRestart) KWM::sendKWMCommand("restart");
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
int Theme::installIcons(void)
{
  QString key, value, mapval, fname, fpath, destName, icon, miniIcon;
  QString iconDir, miniIconDir, cmd, destNameMini, localShareDir;
  QStrList extraIcons;
  QFileInfo finfo;
  int i, installed = 0;
  const char* groupName = "Icons";
  const char* groupNameExtra = "Extra Icons";
  bool wantRestart = false;

  debug("*** beginning with %s", groupName);

  if (!instOverwrite)
  {
    uninstallFiles(groupName);
    if (mInstIcons > 0) wantRestart = true;
    mInstIcons = 0;
  }
  else readInstFileList(groupName);

  setGroup(groupName);
  mMappings->setGroup(groupName);

  mkdirhier("share/icons/mini");
  iconDir = kapp->localkdedir() + "/share/icons/";
  miniIconDir = kapp->localkdedir() + "/share/icons/mini/";
  localShareDir = kapp->localkdedir() + "/share/";

  // Process all mapping entries for the group
  QMap<QString, QString> aMap = entryMap(groupName);
  QMap<QString, QString>::Iterator aIt(aMap.begin());

  for (; aIt != aMap.end(); ++aIt) {
    key = aIt.key();
    if (stricmp(key.left(6).ascii(),"Config")==0)
    {
      warning(i18n("Icon key name %s can not be used").ascii(), key.ascii());
      continue;
    }
    value = *aIt;
    i = value.find(':');
    if (i > 0)
    {
      icon = value.left(i);
      miniIcon = value.mid(i+1, 1024);
    }
    else
    {
      icon = value;
      miniIcon = "mini-" + icon;
      iconToMiniIcon(mThemePath + icon, mThemePath + miniIcon);
    }

    // test if there is a 1:1 mapping in the mappings file
    destName = mMappings->readEntry(key,QString::null);
    destNameMini = QString::null;

    // if not we have to search for the proper kdelnk file
    // and extract the icon name from there.
    if (destName.isEmpty())
    {
      fname = "/" + key + ".kdelnk";
      fpath = locate("apps", fname);
      if (fpath.isNull())
	fpath = locate("mime", fname);
      
      if (!fpath.isEmpty()) {
	KDesktopFile cfg(fpath, true);
	destName = cfg.readIcon();
      }
    }
    else
    {
      // test if the 1:1 mapping contains different names for icon
      // and mini icon
      i = destName.find(':');
      if (i >= 0)
      {
	debug("mapping %s to...", destName.ascii());
	destNameMini = destName.mid(i+1, 1024);
	destName.truncate(i);
	debug("   %s  %s", destName.ascii(), destNameMini.ascii());
      }
    }

    if (destNameMini.isEmpty()) destNameMini = destName;

    // If we have still not found a destination icon name we will install
    // the icons with their current name
    if (destName.isEmpty())
    {
      if (icon.isEmpty()) continue;
      warning(i18n("No proper kdelnk file found for %s.\n"
		   "Installing icon(s) as %s").ascii(),
	      key.ascii(), icon.ascii());
      destName = icon;
    }

    // install icons
    if (destName.find('/')>=0) 
    {
      mkdirhier(pathOf(destName).ascii(),localShareDir.ascii());
      value = localShareDir + destName;
    }
    else value = iconDir + destName;
    if (installFile(icon, value)) installed++;

    if (destNameMini != "-")
    {
      if (destNameMini.find('/')>=0) 
      {
	mkdirhier(pathOf(destNameMini).ascii(),localShareDir.ascii());
	value = localShareDir + destNameMini;
      }
      else value = miniIconDir + destNameMini;
      if (installFile(miniIcon, value)) installed++;
    }
  }

  // Look if there is anything to do after installation
  value = mMappings->readEntry("ConfigActivateCmd");
  if (!value.isEmpty() && (installed>0 || wantRestart) && 
      mCmdList.find(value.ascii()) < 0) mCmdList.append(value.ascii());

  writeInstFileList(groupName);

  // Handle extra icons
  setGroup(groupNameExtra);
  mMappings->setGroup(groupNameExtra);

  aMap = entryMap(groupNameExtra);
  for (aIt = aMap.begin(); aIt != aMap.end(); ++aIt) {
    key = aIt.key();
    value = *aIt;
    i = value.find(':');
    if (i > 0)
    {
      icon = value.left(i);
      miniIcon = value.mid(i+1, 1024);
    }
    else
    {
      icon = value;
      miniIcon = "mini-" + icon;
      iconToMiniIcon(mThemePath + icon, mThemePath + miniIcon);
    }

    // Install icons
    value = iconDir + icon;
    if (installFile(icon, value)) installed++;

    value = miniIconDir + icon;
    if (installFile(miniIcon, value)) installed++;
  }

  writeInstFileList(groupName);
  debug("*** done with %s (%d icons installed)", groupName, installed);

  mInstIcons += installed;
  return installed;
}


//-----------------------------------------------------------------------------
void Theme::addInstFile(const char* aFileName)
{
  if (aFileName && *aFileName && mInstFiles.find(aFileName) < 0)
    mInstFiles.append(strdup(aFileName));
}


//-----------------------------------------------------------------------------
void Theme::readInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->getConfig();

  assert(aGroupName!=0);
  cfg->setGroup("Installed Files");
  mInstFiles.clear();
  cfg->readListEntry(aGroupName, mInstFiles, ':');
}


//-----------------------------------------------------------------------------
void Theme::writeInstFileList(const char* aGroupName)
{
  KConfig* cfg = kapp->getConfig();

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
  QStrList fileList;

  debug("*** beginning uninstall of %s", aGroupName);

  readInstFileList(aGroupName);
  for (fname=mInstFiles.first(); !fname.isEmpty(); fname=mInstFiles.next())
  {
    reverted = false;
    if (unlink(fname.ascii())==0) reverted = true;
    finfo.setFile(fname+'~');
    if (finfo.exists())
    {
      if (rename((fname+'~').ascii(), fname.ascii()))
	warning(i18n("Failed to rename %s to %s~:\n%s").ascii(),
		fname.ascii(), fname.ascii(),
		strerror(errno));
      else reverted = true;
    }

    if (reverted) 
    {
      debug("uninstalled %s", fname.ascii());
      processed++;
    }
  }
  mInstFiles.clear();
  writeInstFileList(aGroupName);

  debug("*** done uninstall of %s", aGroupName);
}


//-----------------------------------------------------------------------------
void Theme::install(void)
{
  debug("Theme::install() started");

  loadMappings();
  mCmdList.clear();

  if (instPanel) installGroup("Panel");
  if (instSounds) installGroup("Sounds");
  if (instWindowBorder) installGroup("Window Border");
  if (instWindowTitlebar) installGroup("Window Titlebar");
  if (instWindowButtonLayout) installGroup("Window Button Layout");
  if (instWindowGimmick) installGroup("Window Gimmick");
  if (instWallpapers) installGroup("Display");
  if (instKfm) installGroup("File Manager");
  if (instColors) installGroup("Colors");
  if (instIcons) installIcons();

  debug("*** executing command list");

  doCmdList();

  debug("Theme::install() done");
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
    fname = mThemePath + mPreviewFile;
    if (!mPreview.load(fname))
    {
      warning(i18n("Failed to load preview image %s").ascii(), fname.ascii());
      mPreview.resize(0,0);
    }
  }
  else mPreview.resize(0,0);
}


//-----------------------------------------------------------------------------
void Theme::writeConfig(void)
{
  debug("Theme::writeConfig() is broken");
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
bool Theme::mkdirhier(const char* aDir, const char* aBaseDir)
{
  QDir dir;
  const char* dirName;
  int oldMask = umask(077);

  if (aBaseDir) dir.cd(aBaseDir,true);
  else if (aDir[0]!='/') dir.cd(kapp->localkdedir());
  else dir.cd("/");

  char *buffer = qstrdup(aDir);

  for (dirName=strtok(buffer,"/"); dirName; dirName=strtok(0, "/"))
  {
    if (dirName[0]=='\0') continue;
    if (!dir.exists(dirName))
    {
      if (!dir.mkdir(dirName))
      {
	warning(i18n("Cannot create directory %s").ascii(),
		(dir.dirName() + dirName).ascii());
	umask(oldMask);
	return false;
      }
    }
    if (!dir.cd(dirName))
    {
      warning(i18n("Cannot enter directory %s").ascii(),
	      (dir.dirName() + dirName).ascii());
      umask(oldMask);
      return false;
    }
  }

  delete [] buffer;
  umask(oldMask);
  return true;
}


//-----------------------------------------------------------------------------
bool Theme::hasGroup(const QString& aName, bool aNotEmpty)
{
  bool found(hasGroup(aName));

  if (!aNotEmpty)
    return found;

  QMap<QString, QString> aMap = entryMap(aName);
  if (found && aNotEmpty) 
    found = aMap.isEmpty();

  return found;
}


//-----------------------------------------------------------------------------
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, 
			      &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0) return -1;
  if (n == 0) XFree((char*) *p);
  return n;
}


//-----------------------------------------------------------------------------
static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;

  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }

  result = p[0];
  XFree((char *) p);
  return TRUE;
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

  debug("stretching %s from %dx%d to %dx%d", aFname.ascii(),
	w, h, w2, h2);

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
  XEvent ev;
  unsigned int i, nrootwins;
  Window dw1, dw2, *rootwins;
  int (*defaultHandler)(Display *, XErrorEvent *);
  Display* dpy = qt_xdisplay();
  Window root = RootWindow(dpy, DefaultScreen(dpy));
  Atom KDEChangePalette = XInternAtom(dpy, "KDEChangePalette", false);
	
  defaultHandler = XSetErrorHandler(dropError);
  XQueryTree(dpy, root, &dw1, &dw2, &rootwins, &nrootwins);
	
  // Matthias
  Atom a = XInternAtom(dpy, "KDE_DESKTOP_WINDOW", False);
  for (i = 0; i < nrootwins; i++) {
    long result = 0;
    getSimpleProperty(rootwins[i],a, result);
    if (result){
      ev.xclient.type = ClientMessage;
      ev.xclient.display = dpy;
      ev.xclient.window = rootwins[i];
      ev.xclient.message_type = KDEChangePalette;
      ev.xclient.format = 32;
      
      XSendEvent(dpy, rootwins[i] , False, 0L, &ev);
    }
  }
  
  XFlush(dpy);
  XSetErrorHandler(defaultHandler);
  
  XFree((char*)rootwins);
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
