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

#include <qwhatsthis.h>

#include "proxywidget.h"
#include "proxywidget.moc"


class WhatsThis : public QWhatsThis
{
public:
    WhatsThis( ProxyWidget* parent )
    : QWhatsThis( parent ), proxy( parent ) {}
    ~WhatsThis(){};


    QString text( const QPoint &  ) {
    if ( !proxy->quickHelp().isEmpty() )
        return proxy->quickHelp();
    else
        return i18n("The currently loaded configuration module.");
    }

private:
    ProxyWidget* proxy;
};

static void setVisible(QPushButton *btn, bool vis)
{
  if (vis)
    btn->show();
  else
    btn->hide();
}

ProxyWidget::ProxyWidget(KCModule *client, QString title, const char *name,
             bool run_as_root)
  : QWidget(0, name)
  , _client(client)
{
  setCaption(title);

  (void) new WhatsThis( this );

  _client->resize(_client->minimumSize());
  _client->reparent(this,0,QPoint(0,0),true);
  connect(_client, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

  _sep = new QFrame(this);
  _sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  _sep->show();

  _help = new QPushButton(i18n("&Help"), this);
  _default = new QPushButton(i18n("&Default"), this);
  _reset = new QPushButton(i18n("&Reset"), this);
  _cancel = new QPushButton(i18n("&Cancel"), this);
  _ok = new QPushButton(i18n("&OK"), this);
  _apply = new QPushButton(i18n("&Apply"), this);
  _root = new QPushButton(i18n("&Modify"), this);

  // only enable the requested buttons
  int b = _client->buttons();
  setVisible(_help, b & KCModule::Help);
  setVisible(_default, !run_as_root && b & KCModule::Default);
  setVisible(_reset, !run_as_root && b & KCModule::Reset);
  setVisible(_cancel, b & KCModule::Cancel);
  setVisible(_apply, !run_as_root && b & KCModule::Apply);
  setVisible(_ok, b & KCModule::Ok);
  setVisible(_root, run_as_root);

  // disable initial buttons
  _reset->setEnabled(false);
  _apply->setEnabled(false);

  connect(_help, SIGNAL(clicked()), this, SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), this, SLOT(defaultClicked()));
  connect(_reset, SIGNAL(clicked()), this, SLOT(resetClicked()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(applyClicked()));
  connect(_root, SIGNAL(clicked()), this, SLOT(rootClicked()));

  if (run_as_root)
    connect(_ok, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  else
    connect(_ok, SIGNAL(clicked()), this, SLOT(okClicked()));

  QVBoxLayout *top = new QVBoxLayout(this, 2, 4);
  top->addWidget(_client);
  top->addWidget(_sep);

  QHBoxLayout *buttons = new QHBoxLayout(top, 4);
  buttons->addWidget(_help);
  buttons->addWidget(_default);
  buttons->addWidget(_reset);
  buttons->addStretch(1);
  buttons->addWidget(_ok);
  if (run_as_root)
    buttons->addWidget(_root);
  else
    buttons->addWidget(_apply);
  buttons->addWidget(_cancel);

  top->activate();

  // Restrict minimum size to the optimal one
  //setMinimumSize(sizeHint());
}

ProxyWidget::~ProxyWidget()
{
  delete _client;
}

QString ProxyWidget::quickHelp() const
{
  if (_client)
    return _client->quickHelp();
  else
    return "";
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


void ProxyWidget::rootClicked()
{
  emit runAsRoot();
}


void ProxyWidget::clientChanged(bool state)
{
  // enable/disable buttons
    // int b = _client->buttons();
  _reset->setEnabled(state);
  _apply->setEnabled(state);
  //_ok->setEnabled(state);

  // forward the signal
  emit changed(state);
}

const KAboutData *ProxyWidget::aboutData() const
{
  return _client->aboutData();
}
