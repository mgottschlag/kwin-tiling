/*
 * localenum.cpp
 *
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qregexp.h>

#include <kdialog.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "toplevel.h"
#include "localenum.h"
#include "localenum.moc"

KLocaleConfigNumber::KLocaleConfigNumber(KLocale *locale,
					 QWidget *parent, const char*name)
  : QWidget(parent, name),
    m_locale(locale)
{
  QGridLayout *lay = new QGridLayout(this, 5, 2,
				     KDialog::marginHint(),
				     KDialog::spacingHint());
  lay->setAutoAdd(TRUE);

  m_labDecSym = new QLabel(this, I18N_NOOP("&Decimal symbol:"));
  m_edDecSym = new QLineEdit(this);
  connect( m_edDecSym, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotDecSymChanged(const QString &) ) );
  m_labDecSym->setBuddy(m_edDecSym);

  m_labThoSep = new QLabel(this, I18N_NOOP("Tho&usands separator:"));
  m_edThoSep = new QLineEdit(this);
  connect( m_edThoSep, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotThoSepChanged(const QString &) ) );
  m_labThoSep->setBuddy(m_edThoSep);

  m_labMonPosSign = new QLabel(this, I18N_NOOP("Positive si&gn:"));
  m_edMonPosSign = new QLineEdit(this);
  connect( m_edMonPosSign, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotMonPosSignChanged(const QString &) ) );
  m_labMonPosSign->setBuddy(m_edMonPosSign);

  m_labMonNegSign = new QLabel(this, I18N_NOOP("&Negative sign:"));
  m_edMonNegSign = new QLineEdit(this);
  connect( m_edMonNegSign, SIGNAL( textChanged(const QString &) ),
	   this, SLOT( slotMonNegSignChanged(const QString &) ) );
  m_labMonNegSign->setBuddy(m_edMonNegSign);

  lay->setColStretch(1, 1);

  connect(this, SIGNAL(localeChanged()),
	  SLOT(slotLocaleChanged()));
}

KLocaleConfigNumber::~KLocaleConfigNumber()
{
}

void KLocaleConfigNumber::save()
{
  // temperary use of our locale as the global locale
  KLocale *lsave = KGlobal::_locale;
  KGlobal::_locale = m_locale;

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, "Locale");

  KSimpleConfig ent(locate("locale",
			   QString::fromLatin1("l10n/%1/entry.desktop")
			   .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("DecimalSymbol",
		      QString::fromLatin1("."));
  config->deleteEntry("DecimalSymbol", false, true);
  if (str != m_locale->decimalSymbol())
    config->writeEntry("DecimalSymbol",
		       m_locale->decimalSymbol(), true, true);

  str = ent.readEntry("ThousandsSeparator",
		      QString::fromLatin1(","));
  config->deleteEntry("ThousandsSeparator", false, true);
  str.replace(QString::fromLatin1("$0"), QString::null);
  if (str != m_locale->thousandsSeparator())
    config->writeEntry("ThousandsSeparator",
		       QString::fromLatin1("$0%1$0")
		       .arg(m_locale->thousandsSeparator()), true, true);

  str = ent.readEntry("PositiveSign");
  config->deleteEntry("PositiveSign", false, true);
  if (str != m_locale->positiveSign())
    config->writeEntry("PositiveSign", m_locale->positiveSign(), true, true);

  str = ent.readEntry("NegativeSign", QString::fromLatin1("-"));
  config->deleteEntry("NegativeSign", false, true);
  if (str != m_locale->negativeSign())
    config->writeEntry("NegativeSign", m_locale->negativeSign(), true, true);

  // restore the old global locale
  KGlobal::_locale = lsave;
}

void KLocaleConfigNumber::slotLocaleChanged()
{
  // #### load all settings here
  m_edDecSym->setText( m_locale->decimalSymbol() );
  m_edThoSep->setText( m_locale->thousandsSeparator() );
  m_edMonPosSign->setText( m_locale->positiveSign() );
  m_edMonNegSign->setText( m_locale->negativeSign() );
}

void KLocaleConfigNumber::slotDecSymChanged(const QString &t)
{
  m_locale->setDecimalSymbol(t);
  emit localeChanged();
}

void KLocaleConfigNumber::slotThoSepChanged(const QString &t)
{
  m_locale->setThousandsSeparator(t);
  emit localeChanged();
}

void KLocaleConfigNumber::slotMonPosSignChanged(const QString &t)
{
  m_locale->setPositiveSign(t);
  emit localeChanged();
}

void KLocaleConfigNumber::slotMonNegSignChanged(const QString &t)
{
  m_locale->setNegativeSign(t);
  emit localeChanged();
}

void KLocaleConfigNumber::slotTranslate()
{
  QString str;

  str = m_locale->translate( "Here you can define the decimal separator used "
			     "to display numbers (i.e. a dot or a comma in "
			     "most countries).<p>"
			     "Note that the decimal separator used to "
			     "display monetary values has to be set "
			     "separately (see the 'Money' tab)." );
  QWhatsThis::add( m_labDecSym, str );
  QWhatsThis::add( m_edDecSym,  str );

  str = m_locale->translate( "Here you can define the thousands separator "
			     "used to display numbers.<p>"
			     "Note that the thousands separator used to "
			     "display monetary values has to be set "
			     "separately (see the 'Money' tab)." );
  QWhatsThis::add( m_labThoSep, str );
  QWhatsThis::add( m_edThoSep,  str );

  str = m_locale->translate( "Here you can specify text used to prefix "
			     "positive numbers. Most people leave this "
			     "blank." );
  QWhatsThis::add( m_labMonPosSign, str );
  QWhatsThis::add( m_edMonPosSign,  str );

  str = m_locale->translate( "Here you can specify text used to prefix "
			     "negative numbers. This should not be empty, so "
			     "you can distinguish positive and negative "
			     "numbers. It is normally set to minus (-)." );
  QWhatsThis::add( m_labMonNegSign, str );
  QWhatsThis::add( m_edMonNegSign,  str );
}
