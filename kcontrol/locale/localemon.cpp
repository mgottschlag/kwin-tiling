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

#ifndef LAT
#define LAT QString::fromLatin1("1")
#endif

KLocaleConfigMoney::KLocaleConfigMoney(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;

  // Money
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  label = new QLabel(LAT, this, I18N_NOOP("Currency symbol"));
  edMonCurSym = new QLineEdit(this);
  connect( edMonCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edMonCurSym, 0, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Decimal symbol"));
  edMonDecSym = new QLineEdit(this);
  connect( edMonDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edMonDecSym, 1, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Thousands separator"));
  edMonThoSep = new QLineEdit(this);
  connect( edMonThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonThoSep, 2, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Fract digits"));
  edMonFraDig = new QLineEdit(this);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonFraDigChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonFraDig, 3, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Positive currency prefix"));
  chMonPosPreCurSym = new QCheckBox(this);
  connect( chMonPosPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonPosPreCurSymChanged() ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(chMonPosPreCurSym, 4, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Negative currency prefix"));
  chMonNegPreCurSym = new QCheckBox(this);
  connect( chMonNegPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonNegPreCurSymChanged() ) );
  tl1->addWidget(label, 5, 1);
  tl1->addWidget(chMonNegPreCurSym, 5, 2);

  label = new QLabel(LAT, this, I18N_NOOP("Sign position, positive"));
  cmbMonPosMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonPosMonSignPosChanged(int) ) );
  tl1->addWidget(label, 6, 1);
  tl1->addWidget(cmbMonPosMonSignPos, 6, 2);
  cmbMonPosMonSignPos->insertItem(LAT);
  cmbMonPosMonSignPos->insertItem(LAT);
  cmbMonPosMonSignPos->insertItem(LAT);
  cmbMonPosMonSignPos->insertItem(LAT);
  cmbMonPosMonSignPos->insertItem(LAT);

  label = new QLabel(LAT, this, I18N_NOOP("Sign position, negative"));
  cmbMonNegMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonNegMonSignPosChanged(int) ) );
  tl1->addWidget(label, 7, 1);
  tl1->addWidget(cmbMonNegMonSignPos, 7, 2);
  cmbMonNegMonSignPos->insertItem(LAT);
  cmbMonNegMonSignPos->insertItem(LAT);
  cmbMonNegMonSignPos->insertItem(LAT);
  cmbMonNegMonSignPos->insertItem(LAT);
  cmbMonNegMonSignPos->insertItem(LAT);

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
  KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"), false);
  c->setGroup(QString::fromLatin1("Locale"));
  // Write something to the file to make it dirty
  c->writeEntry(QString::fromLatin1("CurrencySymbol"), QString::null);

  c->deleteEntry(QString::fromLatin1("CurrencySymbol"), false);
  c->deleteEntry(QString::fromLatin1("MonetaryDecimalSymbol"), false);
  c->deleteEntry(QString::fromLatin1("MonetaryThousandsSeparator"), false);
  c->deleteEntry(QString::fromLatin1("PositiveSign"), false);
  c->deleteEntry(QString::fromLatin1("NegativeSign"), false);
  c->deleteEntry(QString::fromLatin1("FractDigits"), false);
  c->deleteEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), false);
  c->deleteEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), false);
  c->deleteEntry(QString::fromLatin1("PositiveMonetarySignPosition"), false);
  c->deleteEntry(QString::fromLatin1("NegativeMonetarySignPosition"), false);
  delete c;

  KConfigBase *config = new KConfig;
  config->setGroup(QString::fromLatin1("Locale"));

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->time + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;
  int i;
  bool b;

  str = ent.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$"));
  str = config->readEntry(QString::fromLatin1("CurrencySymbol"), str);
  if (str != locale->_currencySymbol)
    config->writeEntry(QString::fromLatin1("CurrencySymbol"), locale->_currencySymbol, true, true);

  str = ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));
  str = config->readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), str);
  if (str != locale->_monetaryDecimalSymbol)
    config->writeEntry(QString::fromLatin1("MonetaryDecimalSymbol"), locale->_monetaryDecimalSymbol, true, true);

  str = ent.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));
  str = config->readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), str);
  if (str != locale->_monetaryThousandsSeparator)
    config->writeEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1("$0")+locale->_monetaryThousandsSeparator+QString::fromLatin1("$0"), true, true);

  str = ent.readEntry(QString::fromLatin1("PositiveSign"));
  str = config->readEntry(QString::fromLatin1("PositiveSign"), str);
  if (str != locale->_positiveSign)
    config->writeEntry(QString::fromLatin1("PositiveSign"), locale->_positiveSign, true, true);

  str = ent.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-"));
  str = config->readEntry(QString::fromLatin1("NegativeSign"), str);
  if (str != locale->_negativeSign)
    config->writeEntry(QString::fromLatin1("NegativeSign"), locale->_negativeSign, true, true);

  i = ent.readNumEntry(QString::fromLatin1("FractDigits"), 2);
  i = config->readNumEntry(QString::fromLatin1("FractDigits"), i);
  if (i != locale->_fracDigits)
    config->writeEntry(QString::fromLatin1("FractDigits"), locale->_fracDigits, true, true);

  b = ent.readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), b);
  if (b != locale->_positivePrefixCurrencySymbol)
    config->writeEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), locale->_positivePrefixCurrencySymbol, true, true);

  b = ent.readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), b);
  if (b != locale->_negativePrefixCurrencySymbol)
    config->writeEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), locale->_negativePrefixCurrencySymbol, true, true);

  i = ent.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), (int)KLocale::BeforeQuantityMoney);
  i = config->readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), i);
  if (i != locale->_positiveMonetarySignPosition)
    config->writeEntry(QString::fromLatin1("PositiveMonetarySignPosition"), (int)locale->_positiveMonetarySignPosition, true, true);

  i = ent.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), (int)KLocale::ParensAround);
  i = config->readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), i);
  if (i != locale->_negativeMonetarySignPosition)
    config->writeEntry(QString::fromLatin1("NegativeMonetarySignPosition"), (int)locale->_negativeMonetarySignPosition, true, true);

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
  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->money + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  locale->_currencySymbol = ent.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$"));
  locale->_monetaryDecimalSymbol = ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));
  locale->_monetaryThousandsSeparator = ent.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));
  locale->_fracDigits = ent.readNumEntry(QString::fromLatin1("FractDigits"), 2);
  locale->_positivePrefixCurrencySymbol = ent.readBoolEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  locale->_negativePrefixCurrencySymbol = ent.readBoolEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  locale->_positiveMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), KLocale::BeforeQuantityMoney);
  locale->_negativeMonetarySignPosition = (KLocale::SignPosition)ent.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), KLocale::ParensAround);

  load();
}
