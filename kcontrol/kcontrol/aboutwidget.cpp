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

#include <qcolor.h>

#include <ktextbrowser.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "global.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"

AboutWidget::AboutWidget(QWidget *parent , const char *name)
  : QVBox(parent, name)
{
  KTextBrowser *browser = new KTextBrowser(this);
  browser->setFrameStyle(QFrame::NoFrame);
  
  QColorGroup clgrp = colorGroup();
  clgrp.setColor( QColorGroup::Base, QColor( 255, 255, 255 ) );
  browser->setPaperColorGroup( clgrp );
  browser->setFrameStyle(QFrame::NoFrame);
  browser->setFocusPolicy(NoFocus);
  //browser->setHScrollBarMode( QScrollView::AlwaysOff );
  browser->setNotifyClick(true);

  QString wizard = locate("data", "kcontrol/pics/wizard.png");
  QString kcontrol = locate("icon", "large/hicolor/apps/kcontrol.png");

  QString text = i18n("<p>"
                      "<table cellpadding=2 cellspacing=1 border=0  width=98% bgcolor=#EEEEFF>"
                      "<tr>"
                      "<td width=1%>"
                      "<img src=\""
                      + kcontrol +
                      "\" border=\"0\">"
                      "</td>"
                      "<td width=90%>"
                      "<b><big><big>KDE Control Center</big></big></b>"
                      "<br>"
                      "Configure your desktop environment."
                      "</td></tr>"
                      "</table>"
                      "</p>"
                      "<br>"
                      "<p>"
                      "Welcome to the \"KDE Control Center\", a central place to configure your "
                      "desktop environment. "
                      "Select a item from the index on the left to load a configuration module. "
                      "Click on \"Desktop\" -> \"Background\" to configure the desktop background "
                      "for example."
                      "</p>"
                      "<br>"
                      "<p>"
                      "<table cellpadding=2 cellspacing=1 border=0  width=100% bgcolor=#EEEEFF>"
                      "<tr>"
                      "<td>KDE version:</td>"
                      "<td><b>"
                      + KCGlobal::kdeVersion() +
                      "</b></td>"
                      "<td rowspan = 6 bgcolor=#FFFFFF>"
                      "<img src=\""
                      + wizard +
                      "\" align=\"left\" border=\"0\">"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td>User:</td>"
                      "<td><b>"
                      + KCGlobal::userName() +
                      "</td></b>"
                      "</tr>"
                      "<tr>"
                      "<td>Hostname:</td>"
                      "<td><b>"
                      + KCGlobal::hostName() +
                      "</td></b>"
                      "</tr>"
                      "<tr>"
                      "<td>System:</td>"
                      "<td><b>"
                      + KCGlobal::systemName() +
                      "</td></b>"
                      "</tr>"
                      "<tr>"
                      "<td>Release:</td>"
                      "<td><b>"
                      + KCGlobal::systemRelease() +
                      "</td></b>"
                      "</tr>"
                      "<tr>"
                      "<td>Machine:</td>"
                      "<td><b>"
                      + KCGlobal::systemMachine() +
                      "</td></b>"
                      "</tr>"
                      "<tr>"
                      "<td colspan=2  bgcolor=#FFFFFF>"
                      "<br>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td colspan=2  bgcolor=#FFFFFF>"
                      "Click on the \"Help\" tab on the left to browse a help text on the active "
                      "control module. Use the \"Search\" tab if you are unsure where to look for "
                      "a particular configuration option. "
                      "</td>"
                      "</tr>"
                      "</table>"
                      "</p>");

  browser->setText(text);
}
