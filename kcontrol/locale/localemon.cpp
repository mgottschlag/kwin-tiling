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

extern KLocale *locale;

KLocaleConfigMoney::KLocaleConfigMoney(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;

  // Money
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  label = new QLabel("1", this, I18N_NOOP("Currency symbol"));
  edMonCurSym = new QLineEdit(this);
  connect( edMonCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edMonCurSym, 0, 2);

  label = new QLabel("1", this, I18N_NOOP("Decimal symbol"));
  edMonDecSym = new QLineEdit(this);
  connect( edMonDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edMonDecSym, 1, 2);

  label = new QLabel("1", this, I18N_NOOP("Thousands separator"));
  edMonThoSep = new QLineEdit(this);
  connect( edMonThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonThoSep, 2, 2);

  label = new QLabel("1", this, I18N_NOOP("Fract digits"));
  edMonFraDig = new QLineEdit(this);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonFraDigChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonFraDig, 3, 2);

  label = new QLabel("1", this, I18N_NOOP("Positive currency prefix"));
  chMonPosPreCurSym = new QCheckBox(this);
  connect( chMonPosPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonPosPreCurSymChanged() ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(chMonPosPreCurSym, 4, 2);

  label = new QLabel("1", this, I18N_NOOP("Negative currency prefix"));
  chMonNegPreCurSym = new QCheckBox(this);
  connect( chMonNegPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonNegPreCurSymChanged() ) );
  tl1->addWidget(label, 5, 1);
  tl1->addWidget(chMonNegPreCurSym, 5, 2);

  label = new QLabel("1", this, I18N_NOOP("Sign position, positive"));
  cmbMonPosMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonPosMonSignPosChanged(int) ) );
  tl1->addWidget(label, 6, 1);
  tl1->addWidget(cmbMonPosMonSignPos, 6, 2);
  cmbMonPosMonSignPos->insertItem("0");
  cmbMonPosMonSignPos->insertItem("1");
  cmbMonPosMonSignPos->insertItem("2");
  cmbMonPosMonSignPos->insertItem("3");
  cmbMonPosMonSignPos->insertItem("4");

  label = new QLabel("1", this, I18N_NOOP("Sign position, negative"));
  cmbMonNegMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonNegMonSignPosChanged(int) ) );
  tl1->addWidget(label, 7, 1);
  tl1->addWidget(cmbMonNegMonSignPos, 7, 2);
  cmbMonNegMonSignPos->insertItem("0");
  cmbMonNegMonSignPos->insertItem("1");
  cmbMonNegMonSignPos->insertItem("2");
  cmbMonNegMonSignPos->insertItem("3");
  cmbMonNegMonSignPos->insertItem("4");

  tl1->setRowStretch(8, 1);

  load();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}


void KLocaleConfigMoney::load()
{
  edMonCurSym->setText(locale->_currencySymbol);
  edMonDecSym->setText(locale->_monetaryDecimalSymbol);
  edMonThoSep->setText(locale->_monetaryThousandsSeparator);
  edMonFraDig->setText(locale->formatNumber(locale->_fracDigits, 0));
  chMonPosPreCurSym->setChecked(locale->_positivePrefixCurrencySymbol);
  chMonNegPreCurSym->setChecked(locale->_negativePrefixCurrencySymbol);
  cmbMonPosMonSignPos->setCurrentItem(locale->_positiveMonetarySignPosition);
  cmbMonNegMonSignPos->setCurrentItem(locale->_negativeMonetarySignPosition);
}

void KLocaleConfigMoney::save()
{
  KSimpleConfig *c = new KSimpleConfig("kdeglobals", false);
  c->setGroup("Locale");
  // Write something to the file to make it dirty
  c->writeEntry("CurrencySymbol", QString::null);

  c->deleteEntry("CurrencySymbol", false);
  c->deleteEntry("MonetaryDecimalSymbol", false);
  c->deleteEntry("MonetaryThousandsSeparator", false);
  c->deleteEntry("PositiveSign", false);
  c->deleteEntry("NegativeSign", false);
  c->deleteEntry("FractDigits", false);
  c->deleteEntry("PositivePrefixCurrencySymbol", false);
  c->deleteEntry("NegativePrefixCurrencySymbol", false);
  c->deleteEntry("PositiveMonetarySignPosition", false);
  c->deleteEntry("NegativeMonetarySignPosition", false);
  delete c;

  KConfigBase *config = new KConfig;
  config->setGroup("Locale");

  KSimpleConfig ent(locate("locale", "l10n/" + locale->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;
  int i;
  bool b;

  str = ent.readEntry("CurrencySymbol", "$");
  str = config->readEntry("CurrencySymbol", str);
  if (str != locale->_currencySymbol)
    config->writeEntry("CurrencySymbol", locale->_currencySymbol, true, true);

  str = ent.readEntry("MonetaryDecimalSymbol", ".");
  str = config->readEntry("MonetaryDecimalSymbol", str);
  if (str != locale->_monetaryDecimalSymbol)
    config->writeEntry("MonetaryDecimalSymbol", locale->_monetaryDecimalSymbol, true, true);

  str = ent.readEntry("MonetaryThousandsSeparator", ",");
  str = config->readEntry("MonetaryThousandsSeparator", str);
  if (str != locale->_monetaryThousandsSeparator)
    config->writeEntry("MonetaryThousandsSeparator", "$0"+locale->_monetaryThousandsSeparator+"$0", true, true);

  str = ent.readEntry("PositiveSign");
  str = config->readEntry("PositiveSign", str);
  if (str != locale->_positiveSign)
    config->writeEntry("PositiveSign", locale->_positiveSign, true, true);

  str = ent.readEntry("NegativeSign", "-");
  str = config->readEntry("NegativeSign", str);
  if (str != locale->_negativeSign)
    config->writeEntry("NegativeSign", locale->_negativeSign, true, true);

  i = ent.readNumEntry("FractDigits", 2);
  i = config->readNumEntry("FractDigits", i);
  if (i != locale->_fracDigits)
    config->writeEntry("FractDigits", locale->_fracDigits, true, true);

  b = ent.readNumEntry("PositivePrefixCurrencySymbol", true);
  b = config->readNumEntry("PositivePrefixCurrencySymbol", b);
  if (b != locale->_positivePrefixCurrencySymbol)
    config->writeEntry("PositivePrefixCurrencySymbol", locale->_positivePrefixCurrencySymbol, true, true);

  b = ent.readNumEntry("NegativePrefixCurrencySymbol", true);
  b = config->readNumEntry("NegativePrefixCurrencySymbol", b);
  if (b != locale->_negativePrefixCurrencySymbol)
    config->writeEntry("NegativePrefixCurrencySymbol", locale->_negativePrefixCurrencySymbol, true, true);

  i = ent.readNumEntry("PositiveMonetarySignPosition", (int)KLocale::BeforeQuantityMoney);
  i = config->readNumEntry("PositiveMonetarySignPosition", i);
  if (i != locale->_positiveMonetarySignPosition)
    config->writeEntry("PositiveMonetarySignPosition", (int)locale->_positiveMonetarySignPosition, true, true);

  i = ent.readNumEntry("NegativeMonetarySignPosition", (int)KLocale::ParensAround);
  i = config->readNumEntry("NegativeMonetarySignPosition", i);
  if (i != locale->_negativeMonetarySignPosition)
    config->writeEntry("NegativeMonetarySignPosition", (int)locale->_negativeMonetarySignPosition, true, true);

  delete config;
}

void KLocaleConfigMoney::defaults()
{
  reset();
}

void KLocaleConfigMoney::slotMonCurSymChanged(const QString &t)
{
  locale->_currencySymbol = t;
  emit resample();
}

void KLocaleConfigMoney::slotMonDecSymChanged(const QString &t)
{
  locale->_monetaryDecimalSymbol = t;
  emit resample();
}

void KLocaleConfigMoney::slotMonThoSepChanged(const QString &t)
{
  locale->_monetaryThousandsSeparator = t;
  emit resample();
}

void KLocaleConfigMoney::slotMonFraDigChanged(const QString &t)
{
  locale->_fracDigits = (int)locale->readNumber(t);
  emit resample();
}

void KLocaleConfigMoney::slotMonPosPreCurSymChanged()
{
  locale->_positivePrefixCurrencySymbol = chMonPosPreCurSym->isChecked();
  emit resample();
}

void KLocaleConfigMoney::slotMonNegPreCurSymChanged()
{
  locale->_negativePrefixCurrencySymbol = chMonNegPreCurSym->isChecked();
  emit resample();
}

void KLocaleConfigMoney::slotMonPosMonSignPosChanged(int i)
{
  locale->_positiveMonetarySignPosition = (KLocale::SignPosition)i;
  emit resample();
}

void KLocaleConfigMoney::slotMonNegMonSignPosChanged(int i)
{
  locale->_negativeMonetarySignPosition = (KLocale::SignPosition)i;
  emit resample();
}

void KLocaleConfigMoney::reset()
{
  KSimpleConfig ent(locate("locale", "l10n/" + locale->money + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  locale->_currencySymbol = ent.readEntry("CurrencySymbol", "$");
  locale->_monetaryDecimalSymbol = ent.readEntry("MonetaryDecimalSymbol", ".");
  locale->_monetaryThousandsSeparator = ent.readEntry("MonetaryThousandsSeparator", ",");
  locale->_fracDigits = ent.readNumEntry("FractDigits", 2);
  locale->_positivePrefixCurrencySymbol = ent.readBoolEntry("PositivePrefixCurrencySymbol", true);
  locale->_negativePrefixCurrencySymbol = ent.readBoolEntry("NegativePrefixCurrencySymbol", true);
  locale->_positiveMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry("PositiveMonetarySignPosition", KLocale::BeforeQuantityMoney);
  locale->_negativeMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry("NegativeMonetarySignPosition", KLocale::ParensAround);

  load();
}
