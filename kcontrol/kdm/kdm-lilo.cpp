/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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


#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>

#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "kdm-lilo.moc"


extern KSimpleConfig *c;

KDMLiloWidget::KDMLiloWidget(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QGridLayout *grid = new QGridLayout(this,4,2,16,8);

  useLilo = new QCheckBox(i18n("Use Lilo for reboot options"), this);
  grid->addMultiCellWidget(useLilo, 0,0, 0,1);
  connect(useLilo, SIGNAL(clicked()), this, SLOT(changed()));
  connect(useLilo, SIGNAL(clicked()), this, SLOT(liloClicked()));

  liloCmd = new QLineEdit(this);
  grid->addWidget(liloCmd, 1, 1);
  QLabel *label = new QLabel(liloCmd, i18n("Lilo command"), this);
  grid->addWidget(label, 1, 0);
  connect(liloCmd, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));

  liloMap = new QLineEdit(this);
  grid->addWidget(liloMap, 2, 1);
  label = new QLabel(liloMap, i18n("Lilo map file"), this);
  grid->addWidget(label, 2, 0);
  connect(liloMap, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));

  grid->setRowStretch(3,1);
  grid->setColStretch(1,1);

  load();

  // read only mode
  if (getuid() != 0)
    {
      useLilo->setEnabled(false);
      liloCmd->setReadOnly(true);
      liloMap->setReadOnly(true);
    }
}


void KDMLiloWidget::save()
{
  c->setGroup("Lilo");

  c->writeEntry("Lilo", useLilo->isChecked());
  c->writeEntry("LiloCommand", liloCmd->text());
  c->writeEntry("LiloMap", liloMap->text());
}


void KDMLiloWidget::load()
{
  c->setGroup("Lilo");

  bool use = c->readBoolEntry("Lilo", false);
  useLilo->setChecked(use);

  liloCmd->setText(c->readEntry("LiloCommand", "/sbin/lilo"));
  liloMap->setText(c->readEntry("LiloMap", "/boot/map"));

  liloClicked();
}


void KDMLiloWidget::defaults()
{
  useLilo->setChecked(false);

  liloCmd->setText("/sbin/lilo");
  liloMap->setText("/boot/map");

  liloClicked();
}


void KDMLiloWidget::liloClicked()
{
  liloCmd->setEnabled(useLilo->isChecked());
  liloMap->setEnabled(useLilo->isChecked());
}


void KDMLiloWidget::changed()
{
  emit KCModule::changed(true);
}
