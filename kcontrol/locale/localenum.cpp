/*
 * localenum.cpp
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
#include <qlayout.h>
#include <qwhatsthis.h>

#include <kglobal.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klocaleadv.h"
#include "toplevel.h"
#include "localenum.h"
#include "localenum.moc"

extern KLocaleAdvanced *locale;

KLocaleConfigNumber::KLocaleConfigNumber(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QGridLayout *tl1 = new QGridLayout(this, 1, 1, 10, 5);
  tl1->setColStretch(2, 1);

  labDecSym = new QLabel(this, I18N_NOOP("Decimal symbol"));
  edDecSym = new QLineEdit(this);
  connect( edDecSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDecSymChanged(const QString &) ) );
  tl1->addWidget(labDecSym, 0, 1);
  tl1->addWidget(edDecSym, 0, 2);

  labThoSep = new QLabel(this, I18N_NOOP("Thousands separator"));
  edThoSep = new QLineEdit(this);
  connect( edThoSep, SIGNAL( textChanged(const QString &) ), this, SLOT( slotThoSepChanged(const QString &) ) );
  tl1->addWidget(labThoSep, 1, 1);
  tl1->addWidget(edThoSep, 1, 2);

  labMonPosSign = new QLabel(this, I18N_NOOP("Positive sign"));
  edMonPosSign = new QLineEdit(this);
  connect( edMonPosSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosSignChanged(const QString &) ) );
  tl1->addWidget(labMonPosSign, 2, 1);
  tl1->addWidget(edMonPosSign, 2, 2);

  labMonNegSign = new QLabel(this, I18N_NOOP("Negative sign"));
  edMonNegSign = new QLineEdit(this);
  connect( edMonNegSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegSignChanged(const QString &) ) );
  tl1->addWidget(labMonNegSign, 3, 1);
  tl1->addWidget(edMonNegSign, 3, 2);

  tl1->setRowStretch(4, 1);
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

/**
 * Load stored configuration.
 */
void KLocaleConfigNumber::load()
{
  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, QString::fromLatin1("Locale"));

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->number() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  // different tmp variables
  QString str;

  // DecimalSymbol
  str = config->readEntry(QString::fromLatin1("DecimalSymbol"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1("."));
  locale->setDecimalSymbol(str);

  // ThousandsSeparator
  str = config->readEntry(QString::fromLatin1("ThousandsSeparator"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));
  locale->setThousandsSeparator(str);
 
  // PositiveSign
  str = config->readEntry(QString::fromLatin1("PositiveSign"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("PositiveSign"));
  locale->setPositiveSign(str);

  // NegativeSign
  str = config->readEntry(QString::fromLatin1("NegativeSign"));
  if (str.isNull())
    str = ent.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-"));
  locale->setNegativeSign(str);

  // update the widgets
  edDecSym->setText(locale->decimalSymbol());
  edThoSep->setText(locale->thousandsSeparator());
  edMonPosSign->setText(locale->positiveSign());
  edMonNegSign->setText(locale->negativeSign());
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

  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->number() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;

  str = ent.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1("."));
  str = config->readEntry(QString::fromLatin1("DecimalSymbol"), str);
  if (str != locale->decimalSymbol())
    config->writeEntry(QString::fromLatin1("DecimalSymbol"), locale->decimalSymbol(), true, true);

  str = ent.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(","));
  str = config->readEntry(QString::fromLatin1("ThousandsSeparator"), str);
  if (str != locale->thousandsSeparator())
    config->writeEntry(QString::fromLatin1("ThousandsSeparator"), locale->thousandsSeparator(), true, true);

  delete config;
}

void KLocaleConfigNumber::defaults()
{
  reset();
}

void KLocaleConfigNumber::slotDecSymChanged(const QString &t)
{
  locale->setDecimalSymbol(t);
  emit resample();
}

void KLocaleConfigNumber::slotThoSepChanged(const QString &t)
{
  locale->setThousandsSeparator(t);
  emit resample();
}

void KLocaleConfigNumber::slotMonPosSignChanged(const QString &t)
{
  locale->setPositiveSign(t);
  emit resample();
}

void KLocaleConfigNumber::slotMonNegSignChanged(const QString &t)
{
  locale->setNegativeSign(t);
  emit resample();
}

void KLocaleConfigNumber::reset()
{
  KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/") + locale->number() + QString::fromLatin1("/entry.desktop")), true);
  ent.setGroup(QString::fromLatin1("KCM Locale"));

  QString str;

  locale->setDecimalSymbol(ent.readEntry(QString::fromLatin1("DecimalSymbol"), QString::fromLatin1(".")));
  locale->setThousandsSeparator(ent.readEntry(QString::fromLatin1("ThousandsSeparator"), QString::fromLatin1(",")));
  locale->setPositiveSign(ent.readEntry(QString::fromLatin1("PositiveSign")));
  locale->setNegativeSign(ent.readEntry(QString::fromLatin1("NegativeSign"), QString::fromLatin1("-")));

  edDecSym->setText(locale->decimalSymbol());
  edThoSep->setText(locale->thousandsSeparator());
  edMonPosSign->setText(locale->positiveSign());
  edMonNegSign->setText(locale->negativeSign());
}

void KLocaleConfigNumber::reTranslate()
{
  QString str;

  str = locale->translate( "Here you can define the decimal separator used "
			   "to display numbers (i.e. a dot or a comma in "
			   "most countries).<p>"
			   "Note that the decimal separator used to "
			   "display monetary values has to be set "
			   "separately (see the 'Money' tab)." );
  QWhatsThis::add( labDecSym, str );
  QWhatsThis::add( edDecSym,  str );

  str = locale->translate( "Here you can define the thousands separator "
			   "used to display numbers.<p>"
			   "Note that the thousands separator used to "
			   "display monetary values has to be set "
			   "separately (see the 'Money' tab)." );
  QWhatsThis::add( labThoSep, str );
  QWhatsThis::add( edThoSep,  str );

  str = locale->translate( "Here you can specify text used to prefix "
			   "positive numbers. Most people leave this "
			   "blank." );
  QWhatsThis::add( labMonPosSign, str );
  QWhatsThis::add( edMonPosSign,  str );

  str = locale->translate( "Here you can specify text used to prefix "
			   "negative numbers. This shouldn't be empty, so "
			   "you can distinguis positive and negative "
			   "numbers. It's normally set to minus (-)." );
  QWhatsThis::add( labMonNegSign, str );
  QWhatsThis::add( edMonNegSign,  str );
}
