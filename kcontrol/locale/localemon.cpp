/*
 * localemon.cpp
 *
 * Copyright (c) 1999-2001 Hans Petter Bieker <bieker@kde.org>
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
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qvbox.h>

#include <kglobal.h>
#include <kdialog.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>

#include "klocalesample.h"
#include "toplevel.h"
#include "localemon.h"
#include "localemon.moc"

KLocaleConfigMoney::KLocaleConfigMoney(KLocale *locale,
				       QWidget *parent, const char*name)
  : QWidget(parent, name),
    m_locale(locale)
{
  // Money
  QGridLayout *lay = new QGridLayout(this, 5, 2,
				     KDialog::marginHint(),
				     KDialog::spacingHint());

  labMonCurSym = new QLabel(this, I18N_NOOP("Currency symbol:"));
  lay->addWidget(labMonCurSym, 0, 0);
  edMonCurSym = new QLineEdit(this);
  lay->addWidget(edMonCurSym, 0, 1);
  connect( edMonCurSym, SIGNAL( textChanged(const QString &) ),
	   SLOT( slotMonCurSymChanged(const QString &) ) );
  
  labMonDecSym = new QLabel(this, I18N_NOOP("Decimal symbol:"));
  lay->addWidget(labMonDecSym, 1, 0);
  edMonDecSym = new QLineEdit(this);
  lay->addWidget(edMonDecSym, 1, 1);
  connect( edMonDecSym, SIGNAL( textChanged(const QString &) ),
	   SLOT( slotMonDecSymChanged(const QString &) ) );

  labMonThoSep = new QLabel(this, I18N_NOOP("Thousands separator:"));
  lay->addWidget(labMonThoSep, 2, 0);
  edMonThoSep = new QLineEdit(this);
  lay->addWidget(edMonThoSep, 2, 1);
  connect( edMonThoSep, SIGNAL( textChanged(const QString &) ),
	   SLOT( slotMonThoSepChanged(const QString &) ) );

  labMonFraDig = new QLabel(this, I18N_NOOP("Fract digits:"));
  lay->addWidget(labMonFraDig, 3, 0);
  edMonFraDig = new QLineEdit(this);
  lay->addWidget(edMonFraDig, 3, 1);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ),
	   SLOT( slotMonFraDigChanged(const QString &) ) );

  QWidget *vbox = new QVBox(this);
  lay->addMultiCellWidget(vbox, 4, 4, 0, 1);
  QGroupBox *grp;
  grp = new QGroupBox( vbox, I18N_NOOP("Positive") );
  grp->setColumns(2);
  labMonPosPreCurSym = new QLabel(grp, I18N_NOOP("Prefix currency symbol:"));
  chMonPosPreCurSym = new QCheckBox(grp);
  connect( chMonPosPreCurSym, SIGNAL( clicked() ),
	   SLOT( slotMonPosPreCurSymChanged() ) );

  labMonPosMonSignPos = new QLabel(grp, I18N_NOOP("Sign position:"));
  cmbMonPosMonSignPos = new QComboBox(grp, "signpos");
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ),
	   SLOT( slotMonPosMonSignPosChanged(int) ) );

  grp = new QGroupBox( vbox, I18N_NOOP("Negative") );
  grp->setColumns(2);
  labMonNegPreCurSym = new QLabel(grp, I18N_NOOP("Prefix currency symbol:"));
  chMonNegPreCurSym = new QCheckBox(grp);
  connect( chMonNegPreCurSym, SIGNAL( clicked() ),
	   SLOT( slotMonNegPreCurSymChanged() ) );

  labMonNegMonSignPos = new QLabel(grp, I18N_NOOP("Sign position:"));
  cmbMonNegMonSignPos = new QComboBox(grp, "signpos");
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ),
	   SLOT( slotMonNegMonSignPosChanged(int) ) );

  // insert some items
  int i = 5;
  while (i--)
    {
      cmbMonPosMonSignPos->insertItem(QString::null);
      cmbMonNegMonSignPos->insertItem(QString::null);
    }

  lay->setColStretch(1, 1);
  lay->addRowSpacing(5, 0);

  adjustSize();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}

void KLocaleConfigMoney::save()
{
  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, "Locale");

  KSimpleConfig ent(locate("locale",
			   QString::fromLatin1("l10n/%1/entry.desktop")
			   .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  QString str;
  int i;
  bool b;

  str = ent.readEntry("CurrencySymbol", QString::fromLatin1("$"));
  config->deleteEntry("CurrencySymbol", false, true);
  if (str != m_locale->currencySymbol())
    config->writeEntry("CurrencySymbol",
		       m_locale->currencySymbol(), true, true);

  str = ent.readEntry("MonetaryDecimalSymbol", QString::fromLatin1("."));
  config->deleteEntry("MonetaryDecimalSymbol", false, true);
  if (str != m_locale->monetaryDecimalSymbol())
    config->writeEntry("MonetaryDecimalSymbol",
		       m_locale->monetaryDecimalSymbol(), true, true);

  str = ent.readEntry("MonetaryThousandsSeparator", QString::fromLatin1(","));
  str.replace(QRegExp(QString::fromLatin1("$0")), QString::null);
  config->deleteEntry("MonetaryThousandsSeparator", false, true);
  if (str != m_locale->monetaryThousandsSeparator())
    config->writeEntry("MonetaryThousandsSeparator",
		       QString::fromLatin1("$0%1$0")
		       .arg(m_locale->monetaryThousandsSeparator()),
		       true, true);

  i = ent.readNumEntry("FracDigits", 2);
  config->deleteEntry("FracDigits", false, true);
  if (i != m_locale->fracDigits())
    config->writeEntry("FracDigits", m_locale->fracDigits(), true, true);

  b = ent.readNumEntry("PositivePrefixCurrencySymbol", true);
  config->deleteEntry("PositivePrefixCurrencySymbol", false, true);
  if (b != m_locale->positivePrefixCurrencySymbol())
    config->writeEntry("PositivePrefixCurrencySymbol",
		       m_locale->positivePrefixCurrencySymbol(), true, true);

  b = ent.readNumEntry("NegativePrefixCurrencySymbol", true);
  config->deleteEntry("NegativePrefixCurrencySymbol", false, true);
  if (b != m_locale->negativePrefixCurrencySymbol())
    config->writeEntry("NegativePrefixCurrencySymbol",
		       m_locale->negativePrefixCurrencySymbol(), true, true);

  i = ent.readNumEntry("PositiveMonetarySignPosition",
		       (int)KLocale::BeforeQuantityMoney);
  config->deleteEntry("PositiveMonetarySignPosition", false, true);
  if (i != m_locale->positiveMonetarySignPosition())
    config->writeEntry("PositiveMonetarySignPosition",
		       (int)m_locale->positiveMonetarySignPosition(),
		       true, true);

  i = ent.readNumEntry("NegativeMonetarySignPosition",
		       (int)KLocale::ParensAround);
  config->deleteEntry("NegativeMonetarySignPosition", false, true);
  if (i != m_locale->negativeMonetarySignPosition())
    config->writeEntry("NegativeMonetarySignPosition",
		       (int)m_locale->negativeMonetarySignPosition(),
		       true, true);

  config->sync();
}

void KLocaleConfigMoney::slotLocaleChanged()
{
  edMonCurSym->setText( m_locale->currencySymbol() );
  edMonDecSym->setText( m_locale->monetaryDecimalSymbol() );
  edMonThoSep->setText( m_locale->monetaryThousandsSeparator() );
  edMonFraDig->setText( m_locale->formatNumber( m_locale->fracDigits(), 0) );

  chMonPosPreCurSym->setChecked( m_locale->positivePrefixCurrencySymbol() );
  chMonNegPreCurSym->setChecked( m_locale->negativePrefixCurrencySymbol() );
  cmbMonPosMonSignPos->setCurrentItem( m_locale->positiveMonetarySignPosition() );
  cmbMonNegMonSignPos->setCurrentItem( m_locale->negativeMonetarySignPosition() );
}

void KLocaleConfigMoney::slotMonCurSymChanged(const QString &t)
{
  m_locale->setCurrencySymbol(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonDecSymChanged(const QString &t)
{
  m_locale->setMonetaryDecimalSymbol(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonThoSepChanged(const QString &t)
{
  m_locale->setMonetaryThousandsSeparator(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonFraDigChanged(const QString &t)
{
  m_locale->setFracDigits((int)m_locale->readNumber(t));
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonPosPreCurSymChanged()
{
  m_locale->setPositivePrefixCurrencySymbol(chMonPosPreCurSym->isChecked());
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonNegPreCurSymChanged()
{
  m_locale->setNegativePrefixCurrencySymbol(chMonNegPreCurSym->isChecked());
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonPosMonSignPosChanged(int i)
{
  m_locale->setPositiveMonetarySignPosition((KLocale::SignPosition)i);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonNegMonSignPosChanged(int i)
{
  m_locale->setNegativeMonetarySignPosition((KLocale::SignPosition)i);
  emit localeChanged();
}

void KLocaleConfigMoney::slotTranslate()
{
  QObjectList list;
  list.append(cmbMonPosMonSignPos);
  list.append(cmbMonNegMonSignPos);

  QComboBox *wc;
  for(QObjectListIt li(list) ; (wc = (QComboBox *)li.current()) != 0; ++li)
  {
    wc->changeItem(m_locale->translate("Parens around"), 0);
    wc->changeItem(m_locale->translate("Before quantity money"), 1);
    wc->changeItem(m_locale->translate("After quantity money"), 2);
    wc->changeItem(m_locale->translate("Before money"), 3);
    wc->changeItem(m_locale->translate("After money"), 4);
  }

  QString str;

  str = m_locale->translate( "Here you can enter your usual currency "
			     "symbol, e.g. $ or DM."
			     "<p>Please note that the Euro symbol may not be "
			     "available on your system, depending on the "
			     "distribution you use." );
  QWhatsThis::add( labMonCurSym, str );
  QWhatsThis::add( edMonCurSym, str );
  str = m_locale->translate( "Here you can define the decimal separator used "
			   "to display monetary values."
			   "<p>Note that the decimal separator used to "
			   "display other numbers has to be defined "
			   "separately (see the 'Numbers' tab)." );
  QWhatsThis::add( labMonDecSym, str );
  QWhatsThis::add( edMonDecSym, str );

  str = m_locale->translate( "Here you can define the thousands separator "
			   "used to display monetary values."
			   "<p>Note that the thousands separator used to "
			   "display other numbers has to be defined "
			   "separately (see the 'Numbers' tab)." );
  QWhatsThis::add( labMonThoSep, str );
  QWhatsThis::add( edMonThoSep, str );

  str = m_locale->translate( "This determines the number of fract digits for "
			   "monetary values, i.e. the number of digits you "
			   "find <em>behind</em> the decimal separator. "
			   "Correct value is 2 for almost all people." );
  QWhatsThis::add( labMonFraDig, str );
  QWhatsThis::add( edMonFraDig, str );

  str = m_locale->translate( "If this option is checked, the currency sign "
			   "will be prefixed (i.e. to the left of the "
			   "value) for all positive monetary values. If "
			   "not, it will be postfixed (i.e. to the right)." );
  QWhatsThis::add( labMonPosPreCurSym, str );
  QWhatsThis::add( chMonPosPreCurSym, str );

  str = m_locale->translate( "If this option is checked, the currency sign "
			   "will be prefixed (i.e. to the left of the "
			   "value) for all negative monetary values. If "
			   "not, it will be postfixed (i.e. to the right)." );
  QWhatsThis::add( labMonNegPreCurSym, str );
  QWhatsThis::add( chMonNegPreCurSym, str );

  str = m_locale->translate( "Here you can select how a positive sign will be "
			   "positioned. This only affects monetary values." );
  QWhatsThis::add( labMonPosMonSignPos, str );
  QWhatsThis::add( cmbMonPosMonSignPos, str );

  str = m_locale->translate( "Here you can select how a negative sign will "
			   "be positioned. This only affects monetary "
			   "values." );
  QWhatsThis::add( labMonNegMonSignPos, str );
  QWhatsThis::add( cmbMonNegMonSignPos, str );
}
