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

#include <unistd.h>
#include <sys/types.h>


#include <qdir.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <ksimpleconfig.h>
#include <kdbtn.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kdialog.h>

#include "kdm-sess.moc"


extern KSimpleConfig *c;

KDMSessionsWidget::KDMSessionsWidget(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
      QString wtstr;


      QGroupBox *group0 = new QGroupBox( i18n("Allo&w to shutdown"), this );

      sdcombo = new QComboBox( FALSE, group0 );
      sdcombo->insertItem(i18n("All"), SdAll);
      sdcombo->insertItem(i18n("Console Only"), SdConsoleOnly);
      sdcombo->insertItem(i18n("Root Only"), SdRootOnly);
      sdcombo->insertItem(i18n("None"), SdNone);
      connect(sdcombo, SIGNAL(activated(int)), this, SLOT(changed()));
      QWhatsThis::add( group0, i18n("Here you can select who is allowed to shutdown"
        " the computer using KDM. Possible values are:<ul>"
        " <li><em>All:</em> everybody can shutdown the computer using KDM</li>"
        " <li><em>Console Only:</em> only users sitting in front of this computer can shut it down using KDM</li></ul>"
        " <li><em>Root Only:</em> KDM will only allow shutdown after the user has entered the root password</li>"
        " <li><em>None:</em> nobody can shutdown the computer using KDM</li>") );


      QGroupBox *group1 = new QGroupBox( i18n("Commands"), this );

      QLabel *shutdown_label = new QLabel(i18n("S&hutdown"), group1);
      shutdown_lined = new KLineEdit(group1);
      shutdown_label->setBuddy( shutdown_lined );
      connect(shutdown_lined, SIGNAL(textChanged(const QString&)),
          this, SLOT(changed()));
      wtstr = i18n("Command to initiate the shutdown sequence. Typical value: /sbin/halt");
      QWhatsThis::add( shutdown_label, wtstr );
      QWhatsThis::add( shutdown_lined, wtstr );

      QLabel *restart_label = new QLabel(i18n("&Restart"), group1);
      restart_lined = new KLineEdit(group1);
      restart_label->setBuddy( restart_lined );
      connect(restart_lined, SIGNAL(textChanged(const QString&)),
          this, SLOT(changed()));
      wtstr = i18n("Command to initiate the restart sequence. Typical value: /sbin/reboot");
      QWhatsThis::add( restart_label, wtstr );
      QWhatsThis::add( restart_lined, wtstr );

      console_check = new QCheckBox(i18n("A&llow console mode"), group1);
      connect(console_check, SIGNAL(toggled(bool)),
          this, SLOT(slotConsoleCheckToggled(bool)));
      connect(console_check, SIGNAL(toggled(bool)),
          this, SLOT(changed()));
      wtstr = i18n("When this is checked, KDM shows the option to switch to console mode.");
      QWhatsThis::add( console_check, wtstr );

      console_label = new QLabel(i18n("Console &mode"), group1);
      console_lined = new QLineEdit(group1);
      console_label->setBuddy( console_lined );
      connect(console_lined, SIGNAL(textChanged(const QString&)),
          this, SLOT(changed()));
      wtstr = i18n("Command to enter console mode. Typical value: /sbin/init 3");
      QWhatsThis::add( console_label, wtstr );
      QWhatsThis::add( console_lined, wtstr );

#ifdef __linux__
      QGroupBox *group4 = new QGroupBox( i18n("Lilo"), this );

      lilo_check = new QCheckBox(i18n("Show boot options"), group4);
      connect(lilo_check, SIGNAL(toggled(bool)),
          this, SLOT(slotLiloCheckToggled(bool)));
      connect(lilo_check, SIGNAL(toggled(bool)),
          this, SLOT(changed()));
      wtstr = i18n("");
      QWhatsThis::add( lilo_check, wtstr );

      lilocmd_label = new QLabel(i18n("Lilo command"), group4);
      lilocmd_lined = new KLineEdit(group4);
      lilocmd_label->setBuddy( lilocmd_lined );
      connect(lilocmd_lined, SIGNAL(textChanged(const QString&)),
          this, SLOT(changed()));
      wtstr = i18n("Command to run Lilo. Typical value: /sbin/lilo");
      QWhatsThis::add( lilocmd_label, wtstr );
      QWhatsThis::add( lilocmd_lined, wtstr );

      lilomap_label = new QLabel(i18n("Lilo map file"), group4);
      lilomap_lined = new KLineEdit(group4);
      lilomap_label->setBuddy( lilomap_lined );
      connect(lilomap_lined, SIGNAL(textChanged(const QString&)),
          this, SLOT(changed()));
      wtstr = i18n("Position of Lilo's map file. Typical value: /boot/map");
      QWhatsThis::add( lilomap_label, wtstr );
      QWhatsThis::add( lilomap_lined, wtstr );
#endif

      QGroupBox *group2 = new QGroupBox( i18n("Session types"), this );

      QLabel *type_label = new QLabel(i18n("New t&ype"), group2);
      session_lined = new QLineEdit(group2);
      type_label->setBuddy( session_lined );
      connect(session_lined, SIGNAL(textChanged(const QString&)),
              SLOT(slotCheckNewSession(const QString&)));
      connect(session_lined, SIGNAL(returnPressed()),
              SLOT(slotAddSessionType()));
      connect(session_lined, SIGNAL(returnPressed()),
          this, SLOT(changed()));
      wtstr = i18n( "To create a new session type, enter its name here and click on <em>Add</em>" );
      QWhatsThis::add( type_label, wtstr );
      QWhatsThis::add( session_lined, wtstr );

      btnadd = new QPushButton( i18n("Add &new"), group2 );
      btnadd->setEnabled(false);
      connect( btnadd, SIGNAL( clicked() ), SLOT( changed() ) );
      connect( btnadd, SIGNAL( clicked() ), SLOT( slotAddSessionType() ) );
      QWhatsThis::add( btnadd, i18n( "Click here to add the new session type entered in the <em>New type</em> field to the list of available sessions." ) );

      QLabel *types_label = new QLabel(i18n("Available &types"), group2);
      sessionslb = new MyListBox(group2);
      types_label->setBuddy( sessionslb );
      connect(sessionslb, SIGNAL(highlighted(int)),
              SLOT(slotSessionHighlighted(int)));
      wtstr = i18n( "This listbox lists the available session types that will be presented to the user." );
      QWhatsThis::add( types_label, wtstr );
      QWhatsThis::add( sessionslb, wtstr );

      btnrm = new QPushButton( i18n("R&emove"), group2 );
      btnrm->setEnabled(false);
      connect( btnrm, SIGNAL( clicked() ), SLOT( slotRemoveSessionType() ) );
      connect( btnrm, SIGNAL( clicked() ), SLOT( changed() ) );
      QWhatsThis::add( btnrm, i18n( "Click here to remove the currently selected session type" ) );

      btnup = new KDirectionButton(UpArrow, group2);
      btnup->setEnabled(false);
      connect(btnup, SIGNAL( clicked() ), SLOT( slotSessionUp() ));
      connect(btnup, SIGNAL( clicked() ), SLOT( changed() ));
      btndown = new KDirectionButton(DownArrow, group2);
      btndown->setEnabled(false);
      btndown->setFixedSize(20, 20);
      connect(btndown,SIGNAL( clicked() ), SLOT( slotSessionDown() ));
      connect(btndown,SIGNAL( clicked() ), SLOT( changed() ));
      wtstr = i18n( "With these two arrow buttons, you can change the order in which the available session types are presented to the user" );
      QWhatsThis::add( btnup, wtstr );
      QWhatsThis::add( btndown, wtstr );

      QBoxLayout *main = new QVBoxLayout( this, 10 );
      QBoxLayout *lgroup0 = new QVBoxLayout( group0, 10 );
      QGridLayout *lgroup1 = new QGridLayout( group1, 3, 5, 10);
#ifdef __linux__
      QGridLayout *lgroup4 = new QGridLayout( group4, 3, 4, 10);
#endif
      QGridLayout *lgroup2 = new QGridLayout( group2, 5, 5, 10);

      main->addWidget(group0);
      main->addWidget(group1);
#ifdef __linux__
      main->addWidget(group4);
#endif
      main->addWidget(group2);

      lgroup0->addSpacing(group0->fontMetrics().height()/2);
      lgroup0->addWidget(sdcombo);

      lgroup1->addRowSpacing(0, group1->fontMetrics().height()/2);
      lgroup1->addColSpacing(2, KDialog::spacingHint() * 2);
      lgroup1->setColStretch(1, 1);
      lgroup1->setColStretch(4, 1);
      lgroup1->addWidget(shutdown_label, 1, 0);
      lgroup1->addWidget(shutdown_lined, 1, 1);
      lgroup1->addWidget(restart_label, 2, 0);
      lgroup1->addWidget(restart_lined, 2, 1);
      lgroup1->addMultiCellWidget(console_check, 1,1, 3,4);
      lgroup1->addWidget(console_label, 2, 3);
      lgroup1->addWidget(console_lined, 2, 4);

#ifdef __linux__
      lgroup4->addRowSpacing(0, group4->fontMetrics().height()/2);
      lgroup4->addColSpacing(1, KDialog::spacingHint() * 2);
      lgroup4->setColStretch(3, 1);
      lgroup4->addWidget(lilo_check, 1, 0);
      lgroup4->addWidget(lilocmd_label, 1, 2);
      lgroup4->addWidget(lilocmd_lined, 1, 3);
      lgroup4->addWidget(lilomap_label, 2, 2);
      lgroup4->addWidget(lilomap_lined, 2, 3);
#endif

      lgroup2->addRowSpacing(0, group2->fontMetrics().height()/2);
      lgroup2->addWidget(type_label, 1, 0);
      lgroup2->addMultiCellWidget(session_lined, 2, 2, 0, 2);
      lgroup2->addWidget(types_label, 1, 3);
      lgroup2->addMultiCellWidget(sessionslb, 2, 5, 3, 3);
      lgroup2->addWidget(btnadd, 3, 0);
      lgroup2->addWidget(btnrm, 3, 2);
      lgroup2->addWidget(btnup, 2, 4);
      lgroup2->addWidget(btndown, 3, 4);
      lgroup2->setColStretch(1, 1);
      lgroup2->setColStretch(3, 2);
      lgroup2->setRowStretch(4, 1);

      main->activate();

      load();

      // read only mode
      if (getuid() != 0)
    {
/*      sdcombo->setEnabled(false);
      restart_lined->setReadOnly(true);
      shutdown_lined->setReadOnly(true);
      session_lined->setReadOnly(true);
      console_lined->setReadOnly(true);
      sessionslb->setEnabled(false);
      btnup->setEnabled(false);
      btndown->setEnabled(false);
      btnrm->setEnabled(false);
      btnadd->setEnabled(false);
*/
      group0->setEnabled(false);
      group1->setEnabled(false);
#ifdef __linux__
      group4->setEnabled(false);
#endif
      group2->setEnabled(false);
    }
}

void KDMSessionsWidget::slotConsoleCheckToggled(bool on)
{
    console_label->setEnabled(on);
    console_lined->setEnabled(on);
}

void KDMSessionsWidget::slotLiloCheckToggled(bool on)
{
#ifdef __linux__
    lilocmd_label->setEnabled(on);
    lilocmd_lined->setEnabled(on);
    lilomap_label->setEnabled(on);
    lilomap_lined->setEnabled(on);
#endif
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

void KDMSessionsWidget::save()
{
    //kdDebug() << "KDMSessionsWidget::applySettings()" << endl;

    c->setGroup("KDM");

    c->writeEntry("Shutdown", shutdown_lined->text(), true);
    c->writeEntry("Restart", restart_lined->text(), true);
    c->writeEntry("ConsoleMode", console_lined->text(), true);
    c->writeEntry("AllowConsoleMode", console_check->isChecked());

  // write shutdown auth
  switch ( sdcombo->currentItem() )
  {
    case SdNone:
    c->writeEntry( "ShutdownButton", "None" );
    break;
    case SdAll:
    c->writeEntry( "ShutdownButton", "All" );
    break;
    case SdRootOnly:
    c->writeEntry( "ShutdownButton", "RootOnly" );
    break;
    case SdConsoleOnly:
    c->writeEntry( "ShutdownButton", "ConsoleOnly" );
    break;
    default:
    break;
  }

    QString sesstr;
    for(uint i = 0; i < sessionslb->count(); i++)
    {
      sesstr.append(sessionslb->text(i));
      sesstr.append(",");
    }
    c->writeEntry( "SessionTypes", sesstr );

#ifdef __linux__
    c->setGroup("Lilo");

    c->writeEntry("Lilo", lilo_check->isChecked());
    c->writeEntry("LiloCommand", lilocmd_lined->text());
    c->writeEntry("LiloMap", lilomap_lined->text());
#endif
}


void KDMSessionsWidget::load()
{
  QString str;

  c->setGroup("KDM");

  // read restart and shutdown cmds
  restart_lined->setText(c->readEntry("Restart", "/sbin/reboot"));
  shutdown_lined->setText(c->readEntry("Shutdown", "/sbin/halt"));
  console_lined->setText(c->readEntry("ConsoleMode", "/sbin/init 3"));
  bool cmon = c->readBoolEntry("AllowConsoleMode", true);
  console_check->setChecked(cmon);
  slotConsoleCheckToggled(cmon);

  str = c->readEntry("ShutdownButton", "All");
  SdModes sdMode;
  if(str == "All")
    sdMode = SdAll;
  else if(str == "ConsoleOnly")
    sdMode = SdConsoleOnly;
  else if(str == "RootOnly")
    sdMode = SdRootOnly;
  else
    sdMode = SdNone;
  sdcombo->setCurrentItem(sdMode);

  QStringList sessions = c->readListEntry( "SessionTypes");
  sessionslb->clear();
  sessionslb->insertStringList(sessions);

#ifdef __linux__
    c->setGroup("Lilo");

    bool lien = c->readBoolEntry("Lilo", false);
    lilo_check->setChecked(lien);
    slotLiloCheckToggled(lien);

    lilocmd_lined->setText(c->readEntry("LiloCommand", "/sbin/lilo"));
    lilomap_lined->setText(c->readEntry("LiloMap", "/boot/map"));
#endif
}



void KDMSessionsWidget::defaults()
{
  restart_lined->setText("/sbin/reboot");
  shutdown_lined->setText("/sbin/halt");
  console_check->setChecked(true);
  slotConsoleCheckToggled(true);
  console_lined->setText("/sbin/init 3");

  sdcombo->setCurrentItem(SdAll);

  sessionslb->clear();
  sessionslb->insertItem("kde");
  sessionslb->insertItem("failsafe");

#ifdef __linux__
    lilo_check->setChecked(false);
    slotLiloCheckToggled(false);

    lilocmd_lined->setText("/sbin/lilo");
    lilomap_lined->setText("/boot/map");
#endif
}


void KDMSessionsWidget::changed()
{
  emit KCModule::changed(true);
}
