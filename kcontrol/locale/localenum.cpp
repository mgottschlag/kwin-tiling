/*
 * localenum.cpp
 *
 * Copyright (c) 1999 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfigNumber
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klocalesample.h"
#include "main.h"
#include "localenum.h"
#include "localenum.moc"

#define i18n(a) (a)

KLocaleConfigNumber::KLocaleConfigNumber(QWidget *parent, const char*name)
 : KConfigWidget(parent, name)
{
  QLabel *label;
  QGroupBox *gbox;

  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);

  // Numbers
  gbox = new QGroupBox(this, i18n("Numbers"));
  tl->addWidget(gbox);

  QGridLayout *tl1 = new QGridLayout(gbox, 2, 4, 5);
  tl1->addRowSpacing(0, 15);
  tl1->addRowSpacing(4, 10);
  tl1->addColSpacing(0, 10);
  tl1->addColSpacing(3, 10);
  tl1->setColStretch(2, 1); 

  label = new QLabel(i18n("Decimal symbol"), gbox);
  edDecSym = new QLineEdit(gbox);
  connect( edDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edDecSym, 1, 2);

  label = new QLabel(i18n("Thousands separator"), gbox);
  edThoSep = new QLineEdit(gbox);
  connect( edThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edThoSep, 2, 2);

  tl1->activate();

  // Examples
  gbox = new QGroupBox("1", this, i18n("Examples"));
  tl->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  syncWithKLocaleNum();
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

void KLocaleConfigNumber::loadSettings()
{
}

void KLocaleConfigNumber::applySettings()
{
  KLocale *locale = KGlobal::locale();
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->number + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("DecimalSymbol");
  if (str != locale->_decimalSymbol)
    config->writeEntry("DecimalSymbol", locale->_decimalSymbol, true, true);

  str = ent.readEntry("ThousandsSeparator");
  if (str != locale->_thousandsSeparator)
    config->writeEntry("ThousandsSeparator", locale->_thousandsSeparator, true, true);
}

void KLocaleConfigNumber::defaultSettings()
{
}

void KLocaleConfigNumber::updateSample()
{
  if (sample)
    sample->update();
}



void KLocaleConfigNumber::slotDecSymChanged(const QString &t)
{
  KGlobal::locale()->_decimalSymbol = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigNumber::slotThoSepChanged(const QString &t)
{
  KGlobal::locale()->_thousandsSeparator = t;
  sample->update();
}

void KLocaleConfigNumber::syncWithKLocaleNum()
{
  edDecSym->setText(KGlobal::locale()->_decimalSymbol);
  edThoSep->setText(KGlobal::locale()->_thousandsSeparator);
}
