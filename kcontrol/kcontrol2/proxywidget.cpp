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


ProxyWidget::ProxyWidget(KCModule *client, QString title, QPixmap icon, KDockContainer *parent, const char *name)
  : KDockWidget(title,icon,parent,name), _client(client)
{
  client->reparent(this,0,QPoint(0,0),true);
  connect(client, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

  _buttons = new QWidget(this);

  _sep = new QFrame(this);
  _sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  _sep->show();

  QHBoxLayout *box = new QHBoxLayout(_buttons, 2, 2);

  _help = new QPushButton(i18n("Help"), _buttons);
  box->addWidget(_help);
  _default = new QPushButton(i18n("Default"), _buttons);
  box->addWidget(_default);
  _reset = new QPushButton(i18n("Reset"), _buttons);
  box->addWidget(_reset);
  box->addStretch();
  _cancel = new QPushButton(i18n("Cancel"), _buttons);
  box->addWidget(_cancel);
  _apply = new QPushButton(i18n("Apply"), _buttons);
  box->addWidget(_apply);
  _ok = new QPushButton(i18n("Ok"), _buttons);
  box->addWidget(_ok);

  // only enable the requested buttons
  int b = _client->buttons();
  _help->setEnabled(b & KCModule::Help);
  _default->setEnabled(b & KCModule::Default);
  _reset->setEnabled(b & KCModule::Reset);
  _cancel->setEnabled(b & KCModule::Cancel);
  _apply->setEnabled(b & KCModule::Apply);
  _ok->setEnabled(b & KCModule::Ok);

  _buttons->setFixedHeight(_help->sizeHint().height()+4);
  resizeEvent(0);
  _buttons->show();

  connect(_help, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), this, SLOT(defaultClicked()));
  connect(_reset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_ok, SIGNAL(clicked()), this, SLOT(okClicked()));


  connect(this, SIGNAL(closeClicked()), this, SLOT(cancelClicked()));
}


ProxyWidget::~ProxyWidget()
{
}


void ProxyWidget::resizeEvent(QResizeEvent *event)
{
  KDockWidget::resizeEvent(event);

  _sep->setGeometry(0,height()-_buttons->height()-2,width(),2);
  _buttons->setGeometry(0,height()-_buttons->height(),width(),_buttons->height());
  _client->setGeometry(0,DOCKBAR_HEIGHT,width(),height()-_buttons->height()-2-DOCKBAR_HEIGHT);
}


void ProxyWidget::helpClicked()
{
  // TODO: really show some help
  emit helpRequest("index.html");
}


void ProxyWidget::defaultClicked()
{
  _client->defaults();
  emit changed(true);
}


void ProxyWidget::resetClicked()
{
  _client->load();
  emit changed(false);
}


void ProxyWidget::cancelClicked()
{
  emit closed();
}


void ProxyWidget::applyClicked()
{
  _client->save();
  emit changed(false);
}


void ProxyWidget::okClicked()
{
  _client->save();
  emit closed();
}
