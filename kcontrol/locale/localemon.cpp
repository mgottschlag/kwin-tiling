/*
 * localemon.cpp
 *
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
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
#include <qobjectlist.h>

#include <kglobal.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klocaleadv.h"
#include "klocalesample.h"
#include "toplevel.h"
#include "localemon.h"
#include "localemon.moc"

extern KLocaleAdvanced *locale;

KLocaleConfigMoney::KLocaleConfigMoney(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;

  // Money
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  label = new QLabel(this, I18N_NOOP("Currency symbol"));
  edMonCurSym = new QLineEdit(this);
  connect( edMonCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edMonCurSym, 0, 2);

  label = new QLabel(this, I18N_NOOP("Decimal symbol"));
  edMonDecSym = new QLineEdit(this);
  connect( edMonDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edMonDecSym, 1, 2);

  label = new QLabel(this, I18N_NOOP("Thousands separator"));
  edMonThoSep = new QLineEdit(this);
  connect( edMonThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonThoSep, 2, 2);

  label = new QLabel(this, I18N_NOOP("Fract digits"));
  edMonFraDig = new QLineEdit(this);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonFraDigChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonFraDig, 3, 2);

  label = new QLabel(this, I18N_NOOP("Positive currency prefix"));
  chMonPosPreCurSym = new QCheckBox(this);
  connect( chMonPosPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonPosPreCurSymChanged() ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(chMonPosPreCurSym, 4, 2);

  label = new QLabel(this, I18N_NOOP("Negative currency prefix"));
  chMonNegPreCurSym = new QCheckBox(this);
  connect( chMonNegPreCurSym, SIGNAL( clicked() ), this, SLOT( slotMonNegPreCurSymChanged() ) );
  tl1->addWidget(label, 5, 1);
  tl1->addWidget(chMonNegPreCurSym, 5, 2);

  label = new QLabel(this, I18N_NOOP("Sign position, positive"));
  cmbMonPosMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonPosMonSignPosChanged(int) ) );
  tl1->addWidget(label, 6, 1);
  tl1->addWidget(cmbMonPosMonSignPos, 6, 2);

  label = new QLabel(this, I18N_NOOP("Sign position, negative"));
  cmbMonNegMonSignPos = new QComboBox(this, "signpos");
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonNegMonSignPosChanged(int) ) );
  tl1->addWidget(label, 7, 1);
  tl1->addWidget(cmbMonNegMonSignPos, 7, 2);

  // insert some items
  int i = 5;
  while (i--)
  {
    cmbMonPosMonSignPos->insertItem(QString::null);
    cmbMonNegMonSignPos->insertItem(QString::null);
  }

  tl1->setRowStretch(8, 1);

  load();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}

/**
 * Load stored configuration.
 */
void KLocaleConfigMoney::load()
{
  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->money() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  // different tmp variables
  QString str;
  int i;
  bool b;

  // Currency symbol
  str = config->readEntry(QString::fromLatin1("CurrencySymbol"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$"));
  locale->setCurrencySymbol(str);

  // Decimal symbol
  str = ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));
  locale->setMonetaryDecimalSymbol(str);

  // Thousends separator
  str = config->readEntry(QString::fromLatin1("MonetaryThousendSeparator"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));
  locale->setMonetaryThousandsSeparator(str);

  // Fract digits
  i = config->readNumEntry(QString::fromLatin1("FractDigits"), -1);
  if (i == -1)
    i = ent.readNumEntry(QString::fromLatin1("FractDigits"), 2);             
  locale->setFracDigits(i);

  // PositivePrefixCurrencySymbol
  b = ent.readBoolEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), b);
  locale->setPositivePrefixCurrencySymbol(b);

  // NegativePrefixCurrencySymbol
  b = ent.readBoolEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), b);
  locale->setNegativePrefixCurrencySymbol(b);

  // PositiveMonetarySignPosition
  i = config->readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), -1);
  if (i == -1)
    i = ent.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), KLocale::BeforeQuantityMoney);
  locale->setPositiveMonetarySignPosition((KLocale::SignPosition)i);

  // NegativeMonetarySignPosition
  i = config->readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), -1);
  if (i == -1)
    i = ent.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), KLocale::ParensAround);       
  locale->setNegativeMonetarySignPosition((KLocale::SignPosition)i);

  // update the widgets
  edMonCurSym->setText(locale->currencySymbol());
  edMonDecSym->setText(locale->monetaryDecimalSymbol());
  edMonThoSep->setText(locale->monetaryThousandsSeparator());
  edMonFraDig->setText(locale->formatNumber(locale->fracDigits(), 0));
  chMonPosPreCurSym->setChecked(locale->positivePrefixCurrencySymbol());
  chMonNegPreCurSym->setChecked(locale->negativePrefixCurrencySymbol());
  cmbMonPosMonSignPos->setCurrentItem(locale->positiveMonetarySignPosition());
  cmbMonNegMonSignPos->setCurrentItem(locale->negativeMonetarySignPosition());
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

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->money() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;
  int i;
  bool b;

  str = ent.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$"));
  str = config->readEntry(QString::fromLatin1("CurrencySymbol"), str);
  if (str != locale->currencySymbol())
    config->writeEntry(QString::fromLatin1("CurrencySymbol"), locale->currencySymbol(), true, true);

  str = ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1("."));
  str = config->readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), str);
  if (str != locale->monetaryDecimalSymbol())
    config->writeEntry(QString::fromLatin1("MonetaryDecimalSymbol"), locale->monetaryDecimalSymbol(), true, true);

  str = ent.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(","));
  str = config->readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), str);
  if (str != locale->monetaryThousandsSeparator())
    config->writeEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1("$0")+locale->monetaryThousandsSeparator()+QString::fromLatin1("$0"), true, true);

  str = ent.readEntry(QString::fromLatin1("PositiveSign"));
  str = config->readEntry(QString::fromLatin1("PositiveSign"), str);
  if (str != locale->positiveSign())
    config->writeEntry(QString::fromLatin1("PositiveSign"), locale->positiveSign(), true, true);

  str = ent.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-"));
  str = config->readEntry(QString::fromLatin1("NegativeSign"), str);
  if (str != locale->negativeSign())
    config->writeEntry(QString::fromLatin1("NegativeSign"), locale->negativeSign(), true, true);

  i = ent.readNumEntry(QString::fromLatin1("FractDigits"), 2);
  i = config->readNumEntry(QString::fromLatin1("FractDigits"), i);
  if (i != locale->fracDigits())
    config->writeEntry(QString::fromLatin1("FractDigits"), locale->fracDigits(), true, true);

  b = ent.readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), b);
  if (b != locale->positivePrefixCurrencySymbol())
    config->writeEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), locale->positivePrefixCurrencySymbol(), true, true);

  b = ent.readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true);
  b = config->readNumEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), b);
  if (b != locale->negativePrefixCurrencySymbol())
    config->writeEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), locale->negativePrefixCurrencySymbol(), true, true);

  i = ent.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), (int)KLocale::BeforeQuantityMoney);
  i = config->readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), i);
  if (i != locale->positiveMonetarySignPosition())
    config->writeEntry(QString::fromLatin1("PositiveMonetarySignPosition"), (int)locale->positiveMonetarySignPosition(), true, true);

  i = ent.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), (int)KLocale::ParensAround);
  i = config->readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), i);
  if (i != locale->negativeMonetarySignPosition())
    config->writeEntry(QString::fromLatin1("NegativeMonetarySignPosition"), (int)locale->negativeMonetarySignPosition(), true, true);

  delete config;
}

void KLocaleConfigMoney::defaults()
{
  reset();
}

void KLocaleConfigMoney::slotMonCurSymChanged(const QString &t)
{
  locale->setCurrencySymbol(t);
  emit resample();
}

void KLocaleConfigMoney::slotMonDecSymChanged(const QString &t)
{
  locale->setMonetaryDecimalSymbol(t);
  emit resample();
}

void KLocaleConfigMoney::slotMonThoSepChanged(const QString &t)
{
  locale->setMonetaryThousandsSeparator(t);
  emit resample();
}

void KLocaleConfigMoney::slotMonFraDigChanged(const QString &t)
{
  locale->setFracDigits((int)locale->readNumber(t));
  emit resample();
}

void KLocaleConfigMoney::slotMonPosPreCurSymChanged()
{
  locale->setPositivePrefixCurrencySymbol(chMonPosPreCurSym->isChecked());
  emit resample();
}

void KLocaleConfigMoney::slotMonNegPreCurSymChanged()
{
  locale->setNegativePrefixCurrencySymbol(chMonNegPreCurSym->isChecked());
  emit resample();
}

void KLocaleConfigMoney::slotMonPosMonSignPosChanged(int i)
{
  locale->setPositiveMonetarySignPosition((KLocale::SignPosition)i);
  emit resample();
}

void KLocaleConfigMoney::slotMonNegMonSignPosChanged(int i)
{
  locale->setNegativeMonetarySignPosition((KLocale::SignPosition)i);
  emit resample();
}

/**
 * Reset to defaults. This will be ran when user e.g. changes country.
 */
void KLocaleConfigMoney::reset()
{
  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->money() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  locale->setCurrencySymbol(ent.readEntry(QString::fromLatin1("CurrencySymbol"), QString::fromLatin1("$")));
  locale->setMonetaryDecimalSymbol(ent.readEntry(QString::fromLatin1("MonetaryDecimalSymbol"), QString::fromLatin1(".")));
  locale->setMonetaryThousandsSeparator(ent.readEntry(QString::fromLatin1("MonetaryThousandsSeparator"), QString::fromLatin1(",")));
  locale->setFracDigits(ent.readNumEntry(QString::fromLatin1("FractDigits"), 2));
  locale->setPositivePrefixCurrencySymbol(ent.readBoolEntry(QString::fromLatin1("PositivePrefixCurrencySymbol"), true));
  locale->setNegativePrefixCurrencySymbol(ent.readBoolEntry(QString::fromLatin1("NegativePrefixCurrencySymbol"), true));
  locale->setPositiveMonetarySignPosition((KLocale::SignPosition)ent.readNumEntry(QString::fromLatin1("PositiveMonetarySignPosition"), KLocale::BeforeQuantityMoney));
  locale->setNegativeMonetarySignPosition((KLocale::SignPosition)ent.readNumEntry(QString::fromLatin1("NegativeMonetarySignPosition"), KLocale::ParensAround));

  edMonCurSym->setText(locale->currencySymbol());
  edMonDecSym->setText(locale->monetaryDecimalSymbol());
  edMonThoSep->setText(locale->monetaryThousandsSeparator());
  edMonFraDig->setText(locale->formatNumber(locale->fracDigits(), 0));
  chMonPosPreCurSym->setChecked(locale->positivePrefixCurrencySymbol());
  chMonNegPreCurSym->setChecked(locale->negativePrefixCurrencySymbol());
  cmbMonPosMonSignPos->setCurrentItem(locale->positiveMonetarySignPosition());
  cmbMonNegMonSignPos->setCurrentItem(locale->negativeMonetarySignPosition());
}

void KLocaleConfigMoney::reTranslate()
{
  QObjectList list;
  list.append(cmbMonPosMonSignPos);
  list.append(cmbMonNegMonSignPos);

  QComboBox *wc;
  for(QObjectListIt li(list) ; (wc = (QComboBox *)li.current()) != 0; ++li)
  {
    wc->changeItem(locale->translate("Parens around"), 0);
    wc->changeItem(locale->translate("Before quantity money"), 1); 
    wc->changeItem(locale->translate("After quantity money"), 2);  
    wc->changeItem(locale->translate("Before money"), 3);
    wc->changeItem(locale->translate("After money"), 4);
  }
}
