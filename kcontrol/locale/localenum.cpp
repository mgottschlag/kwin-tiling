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
#include <qlayout.h>

#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfigNumber
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "toplevel.h"
#include "localenum.h"
#include "localenum.moc"

extern KLocale *locale;

KLocaleConfigNumber::KLocaleConfigNumber(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1); 

  QLabel *label = new QLabel(this, I18N_NOOP("Decimal symbol"));
  edDecSym = new QLineEdit(this);
  connect( edDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDecSymChanged(const QString &) ) );
  tl1->addWidget(label, 0, 1);
  tl1->addWidget(edDecSym, 0, 2);

  label = new QLabel(this, I18N_NOOP("Thousands separator"));
  edThoSep = new QLineEdit(this);
  connect( edThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotThoSepChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edThoSep, 1, 2);

  label = new QLabel(this, I18N_NOOP("Positive sign"));
  edMonPosSign = new QLineEdit(this);
  connect( edMonPosSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosSignChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edMonPosSign, 2, 2);

  label = new QLabel(this, I18N_NOOP("Negative sign"));
  edMonNegSign = new QLineEdit(this);
  connect( edMonNegSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegSignChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edMonNegSign, 3, 2);

  tl1->setRowStretch(4, 1);

  load();
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

void KLocaleConfigNumber::load()
{
  edDecSym->setText(locale->_decimalSymbol);
  edThoSep->setText(locale->_thousandsSeparator);
  edMonPosSign->setText(locale->_positiveSign);
  edMonNegSign->setText(locale->_negativeSign);
}

void KLocaleConfigNumber::save()
{
  KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"), false);
  c->setGroup(QString::fromLatin1("Locale"));
  // Write something to the file to make it dirty
  c->writeEntry(QString::fromLatin1("DecimalSymbol"), QString::null);

  c->deleteEntry(QString::fromLatin1("DecimalSymbol"), false);
  c->deleteEntry(QString::fromLatin1("ThousandsSeparator"), false);
  delete c;

  KConfigBase *config = new KConfig;
  config->setGroup(QString::fromLatin1("Locale"));

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->time + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;

  str = ent.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1("."));
  str = config->readEntry(QString::fromLatin1("DecimalSymbol"), str);
  if (str != locale->_decimalSymbol)
    config->writeEntry(QString::fromLatin1("DecimalSymbol"), locale->_decimalSymbol, true, true);

  str = ent.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));
  str = config->readEntry(QString::fromLatin1("ThousandsSeparator"), str);
  if (str != locale->_thousandsSeparator)
    config->writeEntry(QString::fromLatin1("ThousandsSeparator"), locale->_thousandsSeparator, true, true);

  delete config;
}

void KLocaleConfigNumber::defaults()
{
  reset();
}

void KLocaleConfigNumber::slotDecSymChanged(const QString &t)
{
  locale->_decimalSymbol = t;
  emit resample();
}

void KLocaleConfigNumber::slotThoSepChanged(const QString &t)
{
  locale->_thousandsSeparator = t;
  emit resample();
}

void KLocaleConfigNumber::slotMonPosSignChanged(const QString &t)
{
  locale->_positiveSign = t;
  emit resample();
}

void KLocaleConfigNumber::slotMonNegSignChanged(const QString &t)
{
  locale->_negativeSign = t;
  emit resample();
}

void KLocaleConfigNumber::reset()
{
  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->number + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;

  locale->_decimalSymbol = ent.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1("."));
  locale->_thousandsSeparator = ent.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));
  locale->_positiveSign = ent.readEntry(QString::fromLatin1("PositiveSign"));
  locale->_negativeSign = ent.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-"));

  load();
}

void KLocaleConfigNumber::reTranslate()
{
}
