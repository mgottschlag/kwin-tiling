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

  label = new QLabel("1", gbox, i18n("Decimal symbol"));
  edDecSym = new QLineEdit(gbox);
  connect( edDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edDecSym, 1, 2);

  label = new QLabel("1", gbox, i18n("Thousands separator"));
  edThoSep = new QLineEdit(gbox);
  connect( edThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edThoSep, 2, 2);

  label = new QLabel("1", gbox, i18n("Positive sign"));
  edMonPosSign = new QLineEdit(gbox);
  connect( edMonPosSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosSignChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonPosSign, 3, 2);

  label = new QLabel("1", gbox, i18n("Negative sign"));
  edMonNegSign = new QLineEdit(gbox);
  connect( edMonNegSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegSignChanged(const QString &) ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(edMonNegSign, 4, 2);

  tl1->activate();

  // Examples
  gbox = new QGroupBox("1", this, i18n("Examples"));
  tl->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  tl->addStretch(1);
  tl->activate();

  loadSettings();
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

void KLocaleConfigNumber::loadSettings()
{
  KLocale *locale = KGlobal::locale();

  edDecSym->setText(locale->_decimalSymbol);
  edThoSep->setText(locale->_thousandsSeparator);
  edMonPosSign->setText(locale->_positiveSign);
  edMonNegSign->setText(locale->_negativeSign);
}

void KLocaleConfigNumber::applySettings()
{
  KLocale *locale = KGlobal::locale();
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->number + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("DecimalSymbol", ".");
  str = str==locale->_decimalSymbol?QString::null:locale->_decimalSymbol;
  config->writeEntry("DecimalSymbol", str, true, true);

  str = ent.readEntry("ThousandsSeparator", ",");
  str = str==locale->_thousandsSeparator?QString::null:"$0"+locale->_thousandsSeparator+"$0";
  config->writeEntry("ThousandsSeparator", str, true, true);

  config->sync();
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

void KLocaleConfigNumber::slotMonPosSignChanged(const QString &t)
{
  KGlobal::locale()->_positiveSign = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigNumber::slotMonNegSignChanged(const QString &t)
{
  KGlobal::locale()->_negativeSign = t;
  ((KLocaleApplication*)kapp)->updateSample();
}



void KLocaleConfigNumber::reset()
{
  KLocale *locale = KGlobal::locale();

  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->number + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  locale->_decimalSymbol = ent.readEntry("DecimalSymbol", ".");
  locale->_thousandsSeparator = ent.readEntry("ThousandsSeparator", ",");
  locale->_positiveSign = ent.readEntry("PositiveSign");
  locale->_negativeSign = ent.readEntry("NegativeSign", "-");

  loadSettings();
}
