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

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qwhatsthis.h>

#include "global.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"

AboutWidget::AboutWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  // main layout
  QVBoxLayout *mainlayout = new QVBoxLayout(this, 10, 15);

  // title hbox
  QHBox *title = new QHBox(this);
  title->setSpacing(10);

  // title image
  QLabel *title_image = new QLabel(title);
  title_image->setPixmap(KGlobal::iconLoader()->loadIcon("kcontrol",
                                                         KIcon::Desktop, KIcon::SizeLarge));
  // title text
  QLabel *title_text = new QLabel(title);
  title_text->setTextFormat(RichText);
  title_text->setText("<big><big>"
                      + i18n("KDE Control Center")
                      + "</big></big>"
                      + "<br>"
                      + i18n("Configure your desktop environment."));

  // give title text as much space as possible
  title->setStretchFactor(title_text, 10);

  // add title to mainlayout
  mainlayout->addWidget(title);

  // intro text widget
  QLabel *intro_text = new QLabel(this);
  intro_text->setTextFormat(RichText);
  intro_text->setText(i18n("Welcome to the \"KDE Control Center\", "
                           "a central place to configure your "
                           "desktop environment. "
                           "Select a item from the index on the left "
                           "to load a configuration module."));

  QWhatsThis::add( this, intro_text->text() );

  // add intro text to mainlayout
  mainlayout->addWidget(intro_text);
  mainlayout->setStretchFactor(intro_text, 1);

  // body hbox
  QHBox *body = new QHBox(this);
  body->setSpacing(10);

  // body text
  QLabel *body_text = new QLabel(body);
  body_text->setTextFormat(RichText);
  body_text->setText("<table border=0 width=100%>"
                     "<tr>"
                     "<td>"
                     "<table celpadding=2 cellspacing=1 border=0>"
                     "<tr>"
                     "<td>"
                     + i18n("KDE version:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::kdeVersion() +
                     "</b></td>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     + i18n("User:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::userName() +
                     "</td></b>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     + i18n("Hostname:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::hostName() +
                     "</td></b>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     + i18n("System:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::systemName() +
                     "</td></b>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     + i18n("Release:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::systemRelease() +
                     "</td></b>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     + i18n("Machine:") +
                     "</td>"
                     "<td><b>"
                     + KCGlobal::systemMachine() +
                     "</td></b>"
                     "</tr>"
                     "<tr>"
                     "<td>"
                     "<br>"
                     "</td>"
                     "</tr>"
                     "</table>"
                     "</td>"
                     "</tr>"
                     "<tr>"
                     "<td colspan=2>"
                     + i18n("Click on the \"<b>Help</b>\" tab on the left to view help "
                            "for the active "
                            "control module. Use the \"<b>Search</b>\" tab if you are unsure "
                            "where to look for "
                            "a particular configuration option.") +
                     "</td>"
                     "</tr>"
                     "</table>");

  // body image
  QLabel *body_image = new QLabel(body);
  body_image->setPixmap(QPixmap(locate("data", "kcontrol/pics/wizard.png")));

  // add body to mainlayout
  mainlayout->addWidget(body);
  mainlayout->setStretchFactor(body, 10);

  // start the party
  mainlayout->activate();
}
