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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <unistd.h>
#include <sys/types.h>


#include <qlabel.h>
#include <qlayout.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3Frame>
#include <Q3PtrList>
#include <QX11EmbedWidget>
#include <kapplication.h>
#include <kdebug.h>
#include <kservicegroup.h>
#include <kprocess.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kauthorized.h>

#include "modules.h"
#include "modules.moc"
#include "global.h"
#include "proxywidget.h"
#include <kcmoduleloader.h>
#include "kcrootonly.h"

#include <X11/Xlib.h>
#include <QX11Info>
#include <kvbox.h>
#include <kauthorized.h>


template class Q3PtrList<ConfigModule>;


ConfigModule::ConfigModule(const KService::Ptr &s)
  : KCModuleInfo(s), _changed(false), _module(0), _embedWidget(0),
    _rootProcess(0), _embedLayout(0), _embedFrame(0)
{
}

ConfigModule::~ConfigModule()
{
  deleteClient();
}

ProxyWidget *ConfigModule::module()
{
  if (_module)
    return _module;

  bool run_as_root = needsRootPrivileges() && (getuid() != 0);

  KCModule *modWidget = 0;

  if (run_as_root && isHiddenByDefault())
     modWidget = new KCRootOnly(0, "root_only");
  else
      modWidget = KCModuleLoader::loadModule(*this);

  if (modWidget)
    {

      _module = new ProxyWidget(modWidget, moduleName(), "", run_as_root);
      connect(_module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
      connect(_module, SIGNAL(closed()), this, SLOT(clientClosed()));
      connect(_module, SIGNAL(helpRequest()), this, SIGNAL(helpRequest()));
      connect(_module, SIGNAL(runAsRoot()), this, SLOT(runAsRoot()));

      return _module;
    }

  return 0;
}

void ConfigModule::deleteClient()
{
  if (_embedWidget)
    XKillClient(QX11Info::display(), _embedWidget->containerWinId());

  delete _rootProcess;
  _rootProcess = 0;

  delete _embedWidget;
  _embedWidget = 0;
  delete _embedFrame;
  _embedFrame = 0;
  kapp->syncX();

  if(_module)
    _module->close(true);
  _module = 0;

  delete _embedLayout;
  _embedLayout = 0;

  KCModuleLoader::unloadModule(*this);
  _changed = false;
}

void ConfigModule::clientClosed()
{
  deleteClient();

  emit changed(this);
  emit childClosed();
}


void ConfigModule::clientChanged(bool state)
{
  setChanged(state);
  emit changed(this);
}


void ConfigModule::runAsRoot()
{
  if (!_module)
    return;

  delete _rootProcess;
  delete _embedWidget;
  delete _embedLayout;

  // create an embed widget that will embed the
  // kcmshell running as root
  _embedLayout = new QVBoxLayout(_module->parentWidget());
  _embedFrame = new KVBox( _module->parentWidget() );
#warning "KDE4 porting ";
  //_embedFrame->setFrameStyle( Q3Frame::Box | Q3Frame::Raised );
  QPalette pal( Qt::red );
  pal.setColor( QColorGroup::Background,
		_module->parentWidget()->colorGroup().background() );
#warning "KDE4 porting"
  //_embedFrame->setPalette( pal );
  //_embedFrame->setLineWidth( 2 );
  //_embedFrame->setMidLineWidth( 2 );
  _embedLayout->addWidget(_embedFrame,1);
  _embedWidget = new QX11EmbedWidget(_embedFrame );
  _module->hide();
  _embedFrame->show();
  QLabel *_busy = new QLabel(i18n("<big>Loading...</big>"), _embedWidget);
  _busy->setAlignment(Qt::AlignCenter);
  _busy->setTextFormat(Qt::RichText);
  _busy->setGeometry(0,0, _module->width(), _module->height());
  _busy->show();

  // prepare the process to run the kcmshell
  QString cmd = service()->exec().trimmed();
  bool kdeshell = false;
  if (cmd.left(5) == "kdesu")
    {
      cmd = cmd.remove(0,5).trimmed();
      // remove all kdesu switches
      while( cmd.length() > 1 && cmd[ 0 ] == '-' )
        {
          int pos = cmd.find( ' ' );
          cmd = cmd.remove( 0, pos ).trimmed();
        }
    }

  if (cmd.left(8) == "kcmshell")
    {
      cmd = cmd.remove(0,8).trimmed();
      kdeshell = true;
    }

  // run the process
  QString kdesu = KStandardDirs::findExe("kdesu");
  if (!kdesu.isEmpty())
    {
      _rootProcess = new KProcess;
      *_rootProcess << kdesu;
      *_rootProcess << "--nonewdcop";
      // We have to disable the keep-password feature because
      // in that case the modules is started through kdesud and kdesu
      // returns before the module is running and that doesn't work.
      // We also don't have a way to close the module in that case.
      *_rootProcess << "--n"; // Don't keep password.
      if (kdeshell) {
         *_rootProcess << QString("kcmshell %1 --embed %2 --lang %3").arg(cmd).arg(_embedWidget->winId()).arg(KGlobal::locale()->language());
      }
      else {
         *_rootProcess << QString("%1 --embed %2 --lang %3").arg(cmd).arg(_embedWidget->winId()).arg( KGlobal::locale()->language() );
      }

      connect(_rootProcess, SIGNAL(processExited(KProcess*)), this, SLOT(rootExited(KProcess*)));

      if ( !_rootProcess->start(KProcess::NotifyOnExit) )
      {
          delete _rootProcess;
          _rootProcess = 0L;
      }

      return;
    }

  // clean up in case of failure
  delete _embedFrame;
  _embedWidget = 0;
  delete _embedLayout;
  _embedLayout = 0;
  _module->show();
}


void ConfigModule::rootExited(KProcess *)
{
  if (_embedWidget->containerWinId())
    XDestroyWindow(QX11Info::display(), _embedWidget->containerWinId());

  delete _embedWidget;
  _embedWidget = 0;

  delete _rootProcess;
  _rootProcess = 0;

  delete _embedLayout;
  _embedLayout = 0;

  delete _module;
  _module=0;

  _changed = false;
  emit changed(this);
  emit childClosed();
}

const KAboutData *ConfigModule::aboutData() const
{
  if (!_module) return 0;
  return _module->aboutData();
}


ConfigModuleList::ConfigModuleList()
{
  setAutoDelete(true);
  subMenus.setAutoDelete(true);
}

void ConfigModuleList::readDesktopEntries()
{
  readDesktopEntriesRecursive( KCGlobal::baseGroup() );
}

bool ConfigModuleList::readDesktopEntriesRecursive(const QString &path)
{

  KServiceGroup::Ptr group = KServiceGroup::group(path);

  if (!group || !group->isValid()) return false;

  KServiceGroup::List list = group->entries(true, true);

  if( list.isEmpty() )
	  return false;

  Menu *menu = new Menu;
  subMenus.insert(path, menu);

  for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
  {
     KSycocaEntry *p = (*it);
     if (p->isType(KST_KService))
     {
        KService *s = static_cast<KService*>(p);
        if (!KAuthorized::authorizeKActionControlModule(s->menuId()))
           continue;

        ConfigModule *module = new ConfigModule(s);
        if (module->library().isEmpty())
        {
           delete module;
           continue;
        }

        append(module);
        menu->modules.append(module);
     }
     else if (p->isType(KST_KServiceGroup) &&
		     readDesktopEntriesRecursive(p->entryPath()) )
        	menu->submenus.append(p->entryPath());

  }
  return true;
}

Q3PtrList<ConfigModule> ConfigModuleList::modules(const QString &path)
{
  Menu *menu = subMenus.find(path);
  if (menu)
     return menu->modules;

  return Q3PtrList<ConfigModule>();
}

QStringList ConfigModuleList::submenus(const QString &path)
{
  Menu *menu = subMenus.find(path);
  if (menu)
     return menu->submenus;

  return QStringList();
}

QString ConfigModuleList::findModule(ConfigModule *module)
{
  Q3DictIterator<Menu> it(subMenus);
  Menu *menu;
  for(;(menu = it.current());++it)
  {
     if (menu->modules.containsRef(module))
        return it.currentKey();
  }
  return QString::null;
}
