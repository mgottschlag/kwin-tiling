/**
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qslider.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <netwm.h>

#include "desktop.h"
#include "desktop.moc"

extern "C"
{
  KCModule *create_numberandnames(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmnumberandnames");
    return new KDesktopConfig(parent, name);
  };
}

// I'm using 16 line inputs by intention as it makes sence to be able
// to see all desktop names at the same time. It also makes sence to
// be able to TAB through those line edits fast. So don't send me mails
// asking why I did not implement a more intelligent/smaler GUI.

KDesktopConfig::KDesktopConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *layout = new QVBoxLayout(this,
					KDialog::marginHint(),
					KDialog::spacingHint());
  
  // number group
  QGroupBox *number_group = new QGroupBox("", this);

  QHBoxLayout *lay = new QHBoxLayout(number_group,
				     KDialog::marginHint(),
				     KDialog::spacingHint());   

  QLabel *label = new QLabel(i18n("Number of Desktops: "), number_group);
  _numLabel = new QLabel("4", number_group);
  _numSlider = new QSlider(1, 16, 1, 4, Horizontal, number_group);
  connect(_numSlider, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));

  lay->addWidget(label);
  lay->addWidget(_numLabel);
  lay->addWidget(_numSlider);

  lay->setStretchFactor(_numLabel, 2);
  lay->setStretchFactor(_numSlider, 3);

  layout->addWidget(number_group);

  // name group
  QGroupBox *name_group = new QGroupBox("", this);

  QGridLayout *grid = new QGridLayout(name_group, 8, 4,
				      KDialog::marginHint(),
				      KDialog::spacingHint());     

  for(int i = 0; i < 8; i++)
    {
      _nameLabel[i] = new QLabel(i18n("Desktop%1:").arg(i+1), name_group);
      _nameLabel[i+8] = new QLabel(i18n("Desktop%1:").arg(i+8+1), name_group);
      _nameInput[i] = new KLineEdit(name_group);
      _nameInput[i+8] = new KLineEdit(name_group);

      connect(_nameInput[i], SIGNAL(textChanged(const QString&)),
	      SLOT(slotTextChanged(const QString&)));
      connect(_nameInput[i+8], SIGNAL(textChanged(const QString&)),
	      SLOT(slotTextChanged(const QString&)));

      grid->addWidget(_nameLabel[i], i, 0);
      grid->addWidget(_nameInput[i], i, 1);
      grid->addWidget(_nameLabel[i+8], i, 2);
      grid->addWidget(_nameInput[i+8], i, 3);
    }

  grid->setColStretch(1, 2);
  grid->setColStretch(3, 2);

  layout->addWidget(name_group);
  layout->setStretchFactor(name_group, 2);

  load();
}

void KDesktopConfig::load()
{
  KConfig c("kdeglobals");
  c.setGroup("KDE");
  int n = c.readNumEntry("NumberOfDesktops", 4);

  _numSlider->setValue(n);

  for(int i = 0; i < 16; i++)
    _nameInput[i]->setText(c.readEntry(QString("DesktopName_%1").arg(i),
				       i18n("Desktop %1").arg(i+1)));

  for(int i = 0; i < 16; i++)
    _nameInput[i]->setEnabled(i < n);
  emit changed(false);
}

void KDesktopConfig::save()
{
  KConfig c("kdeglobals");
  c.setGroup("KDE");
  c.writeEntry("NumberOfDesktops", _numSlider->value());

  for(int i = 0; i < 16; i++)
    c.writeEntry(QString("DesktopName_%1").arg(i),_nameInput[i]->text());

  c.sync();

  // set number of desktops
  NETRootInfo info1( qt_xdisplay(), NET::NumberOfDesktops );
  info1.setNumberOfDesktops(_numSlider->value());

  // set desktop names
  NETRootInfo info2( qt_xdisplay(), NET::DesktopNames );
  for(int i = 0; i < 16; i++)
    info2.setDesktopName(i, (_nameInput[i]->text()).utf8());

  emit changed(false);
}

void KDesktopConfig::defaults()
{
  int n = 4;
  _numSlider->setValue(n);
  _numLabel->setText(QString("%1").arg(n));
  
  for(int i = 0; i < 16; i++)
    _nameInput[i]->setText(i18n("Desktop %1").arg(i+1));

  for(int i = 0; i < 16; i++)
    _nameInput[i]->setEnabled(i < n);
  emit changed(false);
}

QString KDesktopConfig::quickHelp()
{
  return i18n("<h1>Desktop Number&Names</h1>");
}

void KDesktopConfig::slotValueChanged(int n)
{
  for(int i = 0; i < 16; i++)
    _nameInput[i]->setEnabled(i < n);
  _numLabel->setText(QString("%1").arg(n));
  emit changed(true);
}

void KDesktopConfig::slotTextChanged(const QString&)
{
  emit changed(true);
}
