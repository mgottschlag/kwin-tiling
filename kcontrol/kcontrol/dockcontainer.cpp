/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <qapp.h>
#include <qlabel.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmodule.h>

#include "dockcontainer.h"
#include "dockcontainer.moc"

#include "modules.h"
#include "proxywidget.h"

DockContainer::DockContainer(QWidget *parent, const char *name)
  : QWidget(parent, name)
  , _basew(0L)
  , _module(0L)
{
  _busy = new QLabel(i18n("Loading..."), this);
  _busy->setAlignment(AlignCenter);
  _busy->setGeometry(0,0, width(), height());
  _busy->hide();
}

void DockContainer::setBaseWidget(QWidget *widget)
{
  if (!widget) return;
  
  _basew = widget;
  _basew->reparent(this, 0 , QPoint(0,0), true);
  resize(_basew->sizeHint());
  emit newModule(widget->caption(), "");
}

void DockContainer::dockModule(ConfigModule *module)
{
  if (_module == module)
    return;

  if (_module && _module->isChanged())
    {	  	  
      int res = KMessageBox::warningYesNo(0,i18n("There are unsaved changes in the "
                                                 "active module.\n"
                                                 "Do you want to apply the changes "
                                                 "before running\n"
                                                 "the new module or forget the changes?"),
                                          i18n("Unsaved changes"),
                                          i18n("&Apply"),
                                          i18n("&Forget"));
      if (res == KMessageBox::Yes)
        _module->module()->applyClicked();
      
      _module->deleteClient();
    }
  else if (_module)
    _module->deleteClient();
  
  _busy->raise();
  _busy->show();
  _busy->repaint();
  QApplication::setOverrideCursor( waitCursor );
  ProxyWidget *widget = module->module();
  QApplication::restoreOverrideCursor();
  
  if (widget)
    {
      _module = module;
      connect(_module, SIGNAL(childClosed()),
              this, SLOT(removeModule()));
      
      widget->reparent(this, 0 , QPoint(0,0), false);
      widget->resize(width(), height());
      //updateGeometry();
      
      QString quickhelp = "";
      if (_module && _module->module())
        quickhelp = _module->module()->quickHelp();
      
      emit newModule(widget->caption(), quickhelp);
    }
  else
    KMessageBox::sorry(0, i18n("Sorry, the control module \"%1\" could not be loaded.\n"
                               "Perhaps it is not installed.").arg(module->name())
                       , i18n("Could not load control module."));
  _busy->raise();
  if (widget) widget->show();
  _busy->hide();
}

void DockContainer::removeModule()
{
  _module = 0L;
  resizeEvent(0L);
  
  if (_basew)
	emit newModule(_basew->caption(), "");
  else
	emit newModule("", "");
}

void DockContainer::resizeEvent(QResizeEvent *)
{
  _busy->resize(width(), height());
  if (_module)
	{
	  _basew->hide();
	  _module->module()->resize(size());
	}
  else
	{
	  _basew->show();
	  _basew->resize(size());
	}
}
