/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  

#include <qdir.h>
#include <qcombobox.h>
#include <kdbtn.h>
#include <klined.h>
#include <klocale.h>
#include <kstddirs.h>

#include "utils.h"

#include "kdm-sess.moc"

// Destructor
KDMSessionsWidget::~KDMSessionsWidget()
{
  // All widgets are destroyed by their layouts
}

KDMSessionsWidget::KDMSessionsWidget(QWidget *parent, const char *name, bool init)
  : KConfigWidget(parent, name)
{
      gui = !init;
      loadSettings();      
      if(gui)
        setupPage(parent);
}

void KDMSessionsWidget::setupPage(QWidget *)
{
      QGroupBox *group0 = new QGroupBox( i18n("Allow to shutdown"), this );

      sdcombo = new QComboBox( FALSE, group0 );
      connect(sdcombo, SIGNAL(highlighted(int)), SLOT(slotSetAllowShutdown(int)));
      sdcombo->insertItem(i18n("None"), 0);
      sdcombo->insertItem(i18n("All"), 1);
      sdcombo->insertItem(i18n("Root Only"), 2);
      sdcombo->insertItem(i18n("Console Only"), 3);
      sdcombo->setCurrentItem(sdMode);

      QGroupBox *group1 = new QGroupBox( i18n("Commands"), this );

      QLabel *shutdown_label = new QLabel(i18n("Shutdown"), group1);
      shutdown_lined = new KLineEdit(group1);
      shutdown_lined->setText(shutdownstr);

      QLabel *restart_label = new QLabel(i18n("Restart"), group1);
      restart_lined = new KLineEdit(group1);
      restart_lined->setText(restartstr);

      QLabel *console_label = new QLabel(i18n("Console mode"), group1);
      console_lined = new QLineEdit(group1);
      console_lined->setText(consolestr);

      QGroupBox *group2 = new QGroupBox( i18n("Session types"), this );
      
      QLabel *type_label = new QLabel(i18n("New type"), group2);
      session_lined = new QLineEdit(group2);
      connect(session_lined, SIGNAL(textChanged(const QString&)),
              SLOT(slotCheckNewSession(const QString&)));
      connect(session_lined, SIGNAL(returnPressed()),
              SLOT(slotAddSessionType()));

      QLabel *types_label = new QLabel(i18n("Available types"), group2);

      sessionslb = new MyListBox(group2);
      connect(sessionslb, SIGNAL(highlighted(int)), 
              SLOT(slotSessionHighlighted(int)));
      sessionslb->insertStrList(&sessions);

      btnrm = new QPushButton( i18n("Remove"), group2 );
      btnrm->setEnabled(false);
      connect( btnrm, SIGNAL( clicked() ), SLOT( slotRemoveSessionType() ) );

      btnadd = new QPushButton( i18n("Add"), group2 );
      btnadd->setEnabled(false);
      connect( btnadd, SIGNAL( clicked() ), SLOT( slotAddSessionType() ) );

      btnup = new KDirectionButton(UpArrow, group2);
      btnup->setEnabled(false);
      connect(btnup, SIGNAL( clicked() ), SLOT( slotSessionUp() ));
      btndown = new KDirectionButton(DownArrow, group2);
      btndown->setEnabled(false);
      btndown->setFixedSize(20, 20);
      connect(btndown,SIGNAL( clicked() ), SLOT( slotSessionDown() ));

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QBoxLayout *lgroup0 = new QVBoxLayout( group0, 10 );

      QGridLayout *lgroup1 = new QGridLayout( group1, 4, 2, 10);
      QGridLayout *lgroup2 = new QGridLayout( group2, 6, 4, 10);
                                              
      main->addWidget(group0);
      main->addWidget(group1);
      main->addWidget(group2);
      
      lgroup0->addSpacing(10);
      lgroup0->addWidget(sdcombo);
      
      lgroup1->addRowSpacing(0, group1->fontMetrics().height()/2);
      lgroup1->addWidget(shutdown_label, 1, 0);
      lgroup1->addWidget(shutdown_lined, 1, 1);
      lgroup1->addWidget(restart_label, 2, 0);
      lgroup1->addWidget(restart_lined, 2, 1);
      lgroup1->addWidget(console_label, 3, 0);
      lgroup1->addWidget(console_lined, 3, 1);
      lgroup1->setColStretch(1, 1);
      
      lgroup2->addRowSpacing(0, group2->fontMetrics().height()/2);
      lgroup2->addWidget(type_label, 1, 0);
      lgroup2->addMultiCellWidget(session_lined, 2, 2, 0, 1);
      lgroup2->addWidget(types_label, 1, 2);
      lgroup2->addMultiCellWidget(sessionslb, 2, 5, 2, 2);
      lgroup2->addWidget(btnrm, 3, 1);
      lgroup2->addWidget(btnadd, 4, 1);
      lgroup2->addWidget(btnup, 2, 3);
      lgroup2->addWidget(btndown, 3, 3);
      lgroup2->setColStretch(0, 1);
      lgroup2->setColStretch(2, 1);
      lgroup2->setRowStretch(5, 1);
        
      main->activate();
}

void KDMSessionsWidget::slotSessionHighlighted(int s)
{
  session_lined->setText(sessionslb->text(s));
  btnup->setEnabled(s > 0);
  btndown->setEnabled(s < (int)sessionslb->count()-1);
  btnrm->setEnabled(sessionslb->currentItem() > -1);
  if(!sessionslb->isItemVisible(s))
    sessionslb->centerCurrentItem();
}

void KDMSessionsWidget::slotCheckNewSession(const QString& str)
{
  btnadd->setEnabled(!str.isEmpty());
}

void KDMSessionsWidget::slotSessionUp()
{
  moveSession(-1);
}

void KDMSessionsWidget::slotSessionDown()
{
  moveSession(1);
}

void KDMSessionsWidget::moveSession(int d)
{
  int id = sessionslb->currentItem();
  QString str = sessionslb->text(id);
  sessionslb->removeItem(id);
  sessionslb->insertItem(str, id+d);
  sessionslb->setCurrentItem(id+d);
}

void KDMSessionsWidget::slotAddSessionType()
{
  if(!session_lined->text().isEmpty())
  {
    sessionslb->insertItem(session_lined->text());
    session_lined->setText("");
  }
}

void KDMSessionsWidget::slotRemoveSessionType()
{
  int i = sessionslb->currentItem();
  if(i > -1)
    sessionslb->removeItem(i);
}

void KDMSessionsWidget::slotSetAllowShutdown(int s)
{
  sdMode = s;
}

void KDMSessionsWidget::applySettings()
{
  //debug("KDMSessionsWidget::applySettings()");
  KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));

  c->setGroup("KDM");

  if(!shutdown_lined->text().isEmpty())
    c->writeEntry("ShutDown", shutdown_lined->text(), true);
  if(!restart_lined->text().isEmpty())
    c->writeEntry("Restart", restart_lined->text(), true);
  if(!console_lined->text().isEmpty())
    c->writeEntry("ConsoleMode", console_lined->text(), true);

  // write shutdown auth
  switch ( sdMode )
  {
    case Non:
	c->writeEntry( "ShutDownButton", "None" );
	break;
    case All:
	c->writeEntry( "ShutDownButton", "All" );
	break;
    case RootOnly:
	c->writeEntry( "ShutDownButton", "RootOnly" );
	break;
    case ConsoleOnly:
	c->writeEntry( "ShutDownButton", "ConsoleOnly" );
	break;
    default:
	break;
  }

  if(sessionslb->count() > 0)
  {
    QString sesstr;
    for(uint i = 0; i < sessionslb->count(); i++)
    {
      sesstr.append(sessionslb->text(i));
      sesstr.append(";");
    }
    c->writeEntry( "SessionTypes", sesstr );
  }

  delete c;
}

void KDMSessionsWidget::loadSettings()
{
  QString str;
  
  // Get config object
  KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmrc"));
  c->setGroup("KDM");

  // read restart and shutdown cmds
  restartstr = c->readEntry("Restart", "/sbin/reboot");
  shutdownstr = c->readEntry("Shutdown", "/sbin/halt");
  consolestr = c->readEntry("ConsoleMode", "/sbin/init 3");

  str = c->readEntry("ShutDownButton", "All");
  if(str == "All")
    sdMode = All;
  else if(str == "None")
    sdMode = Non;
  else if(str == "RootOnly")
    sdMode = RootOnly;
  else
    sdMode = ConsoleOnly;

  str = c->readEntry( "SessionTypes");
  if(!str.isEmpty())
    semsplit( str, sessions);	  
  //for(uint i = 0; i < sessions.count(); i++)
    //debug("session type: %s", sessions.at(i));

  delete c;
}



