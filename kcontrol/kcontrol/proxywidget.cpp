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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            



#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>


#include <klocale.h>
#include <kapp.h>
#include <kcmodule.h>


#include "proxywidget.h"
#include "proxywidget.moc"


void setVisible(QPushButton *btn, bool vis)
{
  if (vis)
    btn->show();
  else
    btn->hide();
}


ProxyWidget::ProxyWidget(KCModule *client, QString title, QPixmap icon,
			 KDockContainer *parent, const char *name)
  : KDockWidget(title,icon,parent,name), _client(client)
{
  client->reparent(this,0,QPoint(0,0),true);
  connect(client, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
  
  _sep = new QFrame(this);
  _sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  _sep->show();
  
  _help = new QPushButton(i18n("Help"), this);
  _default = new QPushButton(i18n("Default"), this);
  _reset = new QPushButton(i18n("Reset"), this);
  _cancel = new QPushButton(i18n("Cancel"), this);
  _apply = new QPushButton(i18n("Apply"), this);
  _ok = new QPushButton(i18n("OK"), this);
  
  // only enable the requested buttons
  int b = _client->buttons();
  setVisible(_help, b & KCModule::Help);
  setVisible(_default, b & KCModule::Default);
  setVisible(_reset, b & KCModule::Reset);
  setVisible(_cancel, b & KCModule::Cancel);
  setVisible(_apply, b & KCModule::Apply);
  setVisible(_ok, b & KCModule::Ok);
  
  // disable initial buttons
  _reset->setEnabled(false);
  _apply->setEnabled(false);
  
  connect(_help, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), this, SLOT(defaultClicked()));
  connect(_reset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(okClicked()));
  
  QGridLayout *top = new QGridLayout(this, 4, 6, 5);
  top->addRowSpacing(0, DOCKBAR_HEIGHT);
  top->addMultiCellWidget(client, 1, 1, 0, 6);
  top->addMultiCellWidget(_sep, 2, 2, 0, 6);  
  top->addWidget(_help, 3, 0);
  top->addWidget(_default, 3, 1);
  top->addWidget(_reset, 3, 2);
  top->addWidget(_apply, 3, 4);
  top->addWidget(_ok, 3, 5);
  top->addWidget(_cancel, 3, 6);
  
  top->setRowStretch(1, 1);
  top->setColStretch(3, 1);
  
  top->activate();
  
  connect(this, SIGNAL(closeClicked()), this, SLOT(cancelClicked()));
  
  // Restrict minimum size to the optimal one  
  setMinimumSize(sizeHint());
  
  // Notify parent about minimumSize change
  parent->updateGeometry();
}


ProxyWidget::~ProxyWidget()
{
}


void ProxyWidget::helpClicked()
{
  emit helpRequest();
}


void ProxyWidget::defaultClicked()
{
  _client->defaults();
  clientChanged(true);
}


void ProxyWidget::resetClicked()
{
  _client->load();
  clientChanged(false);
}


void ProxyWidget::cancelClicked()
{
  emit closed();
}


void ProxyWidget::applyClicked()
{
  _client->save();
  clientChanged(false);
}


void ProxyWidget::okClicked()
{
  _client->save();
  emit closed();
}


void ProxyWidget::clientChanged(bool state)
{
  // enable/disable buttons
  int b = _client->buttons();
  _reset->setEnabled(state);
  _apply->setEnabled(state);
  _ok->setEnabled(state);

  // forward the signal
  emit changed(state);
}
