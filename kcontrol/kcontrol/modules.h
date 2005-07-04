/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef MODULES_H
#define MODULES_H

#include <kcmoduleinfo.h>
#include <qobject.h>
#include <qdict.h>

template<class ConfigModule> class QPtrList;
class QStringList;
class KAboutData;
class KCModule;
class ProxyWidget;
class KProcess;
class QXEmbed;
class QVBoxLayout;
class QVBox;

class ConfigModule : public QObject, public KCModuleInfo
{
  Q_OBJECT

public:

  ConfigModule(const KService::Ptr &s);
  ~ConfigModule();

  bool isChanged() { return _changed; };
  void setChanged(bool changed) { _changed = changed; };

  bool isActive() { return _module != 0; };
  ProxyWidget *module();
  const KAboutData *aboutData() const;


public slots:

  void deleteClient();


private slots:

  void clientClosed();
  void clientChanged(bool state);
  void runAsRoot();
  void rootExited(KProcess *proc);


signals:

  void changed(ConfigModule *module);
  void childClosed();
  void helpRequest();


private:
  
  bool         _changed;
  ProxyWidget *_module;
  QXEmbed     *_embedWidget;
  KProcess    *_rootProcess;
  QVBoxLayout *_embedLayout;
  QVBox       *_embedFrame;

};

class ConfigModuleList : public QPtrList<ConfigModule>
{
public:

  ConfigModuleList();

  void readDesktopEntries();
  bool readDesktopEntriesRecursive(const QString &path);

  /**
   * Returns all submenus of the submenu identified by path
   */
  QPtrList<ConfigModule> modules(const QString &path);
  
  /**
   * Returns all modules of the submenu identified by path
   */
  QStringList submenus(const QString &path);

  /**
   * Returns the path of the submenu the module is in
   */
  QString findModule(ConfigModule *module);
 
protected:

  class Menu
  {
  public:
    QPtrList<ConfigModule> modules;
    QStringList submenus;
  };

  QDict<Menu> subMenus;
};

#endif
