/*
 * localemon.cpp
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

#define KLocaleConfigAdvanced KLocaleConfigMoney
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klocalesample.h"
#include "main.h"
#include "localemon.h"
#include "localemon.moc"

#define i18n(a) (a)

KLocaleConfigMoney::KLocaleConfigMoney(QWidget *parent, const char*name)
 : KConfigWidget(parent, name)
{
  QLabel *label;
  QGroupBox *gbox;

  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);

  // Money
  gbox = new QGroupBox(this, i18n("Money"));
  tl->addWidget(gbox);

  QGridLayout *tl1 = new QGridLayout(gbox, 6, 4, 5);
  tl1->addRowSpacing(0, 15);
  tl1->addRowSpacing(4, 10);
  tl1->addColSpacing(0, 10);
  tl1->addColSpacing(3, 10);
  tl1->setColStretch(2, 1); 

  label = new QLabel("1", gbox, i18n("Currency symbol"));
  edMonCurSym = new QLineEdit(gbox);
  connect( edMonCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edMonCurSym, 1, 2);

  label = new QLabel("1", gbox, i18n("Decimal symbol"));
  edMonDecSym = new QLineEdit(gbox);
  connect( edMonDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonDecSym, 2, 2);

  label = new QLabel("1", gbox, i18n("Thousands separator"));
  edMonThoSep = new QLineEdit(gbox);
  connect( edMonThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonThoSep, 3, 2);

  label = new QLabel("1", gbox, i18n("Fract digits"));
  edMonFraDig = new QLineEdit(gbox);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonFraDigChanged(const QString &) ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(edMonFraDig, 4, 2);

  label = new QLabel("1", gbox, i18n("Positive currency prefix"));
  chMonPosPreCurSym = new QCheckBox(gbox);
  connect( chMonPosPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonPosPreCurSymChanged() ) );
  tl1->addWidget(label, 5, 1);
  tl1->addWidget(chMonPosPreCurSym, 5, 2);

  label = new QLabel("1", gbox, i18n("Negative currency prefix"));
  chMonNegPreCurSym = new QCheckBox(gbox);
  connect( chMonNegPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonNegPreCurSymChanged() ) );
  tl1->addWidget(label, 6, 1);
  tl1->addWidget(chMonNegPreCurSym, 6, 2);

  label = new QLabel("1", gbox, i18n("Sign position, positive"));
  cmbMonPosMonSignPos = new QComboBox(gbox, "signpos");
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonPosMonSignPosChanged(int) ) );
  tl1->addWidget(label, 7, 1);
  tl1->addWidget(cmbMonPosMonSignPos, 7, 2);
  cmbMonPosMonSignPos->insertItem("0");
  cmbMonPosMonSignPos->insertItem("1");
  cmbMonPosMonSignPos->insertItem("2");
  cmbMonPosMonSignPos->insertItem("3");
  cmbMonPosMonSignPos->insertItem("4");

  label = new QLabel("1", gbox, i18n("Sign position, negative"));
  cmbMonNegMonSignPos = new QComboBox(gbox, "signpos");
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonNegMonSignPosChanged(int) ) );
  tl1->addWidget(label, 8, 1);
  tl1->addWidget(cmbMonNegMonSignPos, 8, 2);
  cmbMonNegMonSignPos->insertItem("0");
  cmbMonNegMonSignPos->insertItem("1");
  cmbMonNegMonSignPos->insertItem("2");
  cmbMonNegMonSignPos->insertItem("3");
  cmbMonNegMonSignPos->insertItem("4");

  tl1->activate();

  // Examples
  gbox = new QGroupBox("1", this, i18n("Examples"));
  tl->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  tl->addStretch(1);
  tl->activate();

  loadSettings();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}


void KLocaleConfigMoney::loadSettings()
{
  KLocale *locale = KGlobal::locale();

  edMonCurSym->setText(locale->_currencySymbol);
  edMonDecSym->setText(locale->_monetaryDecimalSymbol);
  edMonThoSep->setText(locale->_monetaryThousandsSeparator);
  edMonFraDig->setText(locale->formatNumber(locale->_fracDigits, 0));
  chMonPosPreCurSym->setChecked(locale->_positivePrefixCurrencySymbol);
  chMonNegPreCurSym->setChecked(locale->_negativePrefixCurrencySymbol);
  cmbMonPosMonSignPos->setCurrentItem(locale->_positiveMonetarySignPosition);
  cmbMonNegMonSignPos->setCurrentItem(locale->_negativeMonetarySignPosition);
}

void KLocaleConfigMoney::applySettings()
{
  KLocale *locale = KGlobal::locale();
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->money + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;
  int i;

  str = ent.readEntry("CurrencySymbol", "$");
  str = str==locale->_currencySymbol?QString::null:locale->_currencySymbol;
  config->writeEntry("CurrencySymbol", str, true, true);

  str = ent.readEntry("MonetaryDecimalSymbol", ".");
  str = str==locale->_monetaryDecimalSymbol?QString::null:locale->_monetaryDecimalSymbol;
  config->writeEntry("MonetaryDecimalSymbol", str, true, true);

  str = ent.readEntry("MonetaryThousandsSeparator", ",");
  str = str==locale->_monetaryThousandsSeparator?QString::null:"$0"+locale->_monetaryThousandsSeparator+"$0";
  config->writeEntry("MonetaryThousandsSeparator", str, true, true);

  str = ent.readEntry("PositiveSign");
  str = str==locale->_positiveSign?QString::null:locale->_positiveSign;
  config->writeEntry("PositiveSign", str, true, true);

  str = ent.readEntry("NegativeSign", "-");
  str = str==locale->_negativeSign?QString::null:locale->_negativeSign;
  config->writeEntry("NegativeSign", str, true, true);

  i = ent.readNumEntry("FractDigits", 2);
  str = i==locale->_fracDigits?QString::null:QString::number(locale->_fracDigits);
  config->writeEntry("FractDigits", str, true, true);

  i = ent.readNumEntry("PositivePrefixCurrencySymbol", true);
  str = i==locale->_positivePrefixCurrencySymbol?QString::null:QString::number(locale->_positivePrefixCurrencySymbol);
  config->writeEntry("PositivePrefixCurrencySymbol", str, true, true);

  i = ent.readNumEntry("NegativePrefixCurrencySymbol", true);
  str = i==locale->_negativePrefixCurrencySymbol?QString::null:QString::number(locale->_negativePrefixCurrencySymbol);
  config->writeEntry("negativePrefixCurrencySymbol", str, true, true);

  i = ent.readNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney);
  str = i==locale->_positiveMonetarySignPosition?QString::null:QString::number(locale->_positiveMonetarySignPosition);
  config->writeEntry("NegativeMonetarySignPosition", str, true, true);
  
  i = ent.readNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround);
  str = i==locale->_negativeMonetarySignPosition?QString::null:QString::number(locale->_negativeMonetarySignPosition);
  config->writeEntry("NegativeMonetarySignPosition", str, true, true);

  config->sync();
}

void KLocaleConfigMoney::defaultSettings()
{
}

void KLocaleConfigMoney::updateSample()
{
  sample->update();
}

void KLocaleConfigMoney::slotMonCurSymChanged(const QString &t)
{
  KGlobal::locale()->_currencySymbol = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonDecSymChanged(const QString &t)
{
  KGlobal::locale()->_monetaryDecimalSymbol = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonThoSepChanged(const QString &t)
{
  KGlobal::locale()->_monetaryThousandsSeparator = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonFraDigChanged(const QString &t)
{
  KGlobal::locale()->_fracDigits = (int)KGlobal::locale()->readNumber(t);
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonPosPreCurSymChanged()
{
  KGlobal::locale()->_positivePrefixCurrencySymbol = chMonPosPreCurSym->isChecked();
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonNegPreCurSymChanged()
{
  KGlobal::locale()->_negativePrefixCurrencySymbol = chMonNegPreCurSym->isChecked();
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonPosMonSignPosChanged(int i)
{
  KGlobal::locale()->_positiveMonetarySignPosition = (KLocale::SignPosition)i;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonNegMonSignPosChanged(int i)
{
  KGlobal::locale()->_negativeMonetarySignPosition = (KLocale::SignPosition)i;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::reset()
{
  KLocale *locale = KGlobal::locale();

  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->money + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  locale->_currencySymbol = ent.readEntry("CurrencySymbol", "$");
  locale->_monetaryDecimalSymbol = ent.readEntry("MonetaryDecimalSymbol", ".");
  locale->_monetaryThousandsSeparator = ent.readEntry("MonetaryThousandsSeparator", ",");
  locale->_fracDigits = ent.readNumEntry("FractDigits", 2);
  locale->_positivePrefixCurrencySymbol = ent.readBoolEntry("PositivePrefixCurrencySymbol", true);
  locale->_negativePrefixCurrencySymbol = ent.readBoolEntry("NegativePrefixCurrencySymbol", true);
  locale->_positiveMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney);
  locale->_negativeMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround);

  loadSettings();
}
