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

  label = new QLabel("1", gbox, i18n("Positive sign"));
  edMonPosSign = new QLineEdit(gbox);
  connect( edMonPosSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosSignChanged(const QString &) ) );
  tl1->addWidget(label, 4, 1);
  tl1->addWidget(edMonPosSign, 4, 2);

  label = new QLabel("1", gbox, i18n("Negative sign"));
  edMonNegSign = new QLineEdit(gbox);
  connect( edMonNegSign, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegSignChanged(const QString &) ) );
  tl1->addWidget(label, 5, 1);
  tl1->addWidget(edMonNegSign, 5, 2);

  label = new QLabel("1", gbox, i18n("Fract digits"));
  edMonFraDig = new QLineEdit(gbox);
  connect( edMonFraDig, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonFraDigChanged(const QString &) ) );
  tl1->addWidget(label, 6, 1);
  tl1->addWidget(edMonFraDig, 6, 2);

  label = new QLabel("1", gbox, i18n("Positive currency prefix"));
  chMonPosPreCurSym = new QCheckBox(gbox);
  connect( chMonPosPreCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonPosPreCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 7, 1);
  tl1->addWidget(chMonPosPreCurSym, 7, 2);

  label = new QLabel("1", gbox, i18n("Negative currency prefix"));
  chMonNegPreCurSym = new QCheckBox(gbox);
  connect( chMonNegPreCurSym, SIGNAL( textChanged(const QString &) ), this, SLOT( slotMonNegPreCurSymChanged(const QString &) ) );
  tl1->addWidget(label, 8, 1);
  tl1->addWidget(chMonNegPreCurSym, 8, 2);

  label = new QLabel("1", gbox, i18n("Sign position, positive"));
  cmbMonPosMonSignPos = new QComboBox(gbox);
  connect( cmbMonPosMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonPosMonSignPosChanged(int) ) );
  tl1->addWidget(label, 9, 1);
  tl1->addWidget(cmbMonPosMonSignPos, 9, 2);
  cmbMonPosMonSignPos->insertItem("0");
  cmbMonPosMonSignPos->insertItem("1");
  cmbMonPosMonSignPos->insertItem("2");
  cmbMonPosMonSignPos->insertItem("3");
  cmbMonPosMonSignPos->insertItem("4");

  label = new QLabel("1", gbox, i18n("Sign position, negative"));
  cmbMonNegMonSignPos = new QComboBox(gbox);
  connect( cmbMonNegMonSignPos, SIGNAL( activated(int) ), this, SLOT( slotMonNegMonSignPosChanged(int) ) );
  tl1->addWidget(label, 10, 1);
  tl1->addWidget(cmbMonNegMonSignPos, 10, 2);
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

  syncWithKLocaleMon();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}

void KLocaleConfigMoney::loadSettings()
{
}

void KLocaleConfigMoney::applySettings()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->money + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;
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

void KLocaleConfigMoney::slotMonNegSignChanged(const QString &t)
{
  KGlobal::locale()->_negativeSign = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonPosSignChanged(const QString &t)
{
  KGlobal::locale()->_positiveSign = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigMoney::slotMonFraDigChanged(const QString &t)
{
  KGlobal::locale()->_fracDigits = (int)KGlobal::locale()->readNumber(t);
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

void KLocaleConfigMoney::syncWithKLocaleMon()
{
  // Money
  edMonCurSym->setText(KGlobal::locale()->_currencySymbol);
  edMonDecSym->setText(KGlobal::locale()->_monetaryDecimalSymbol);
  edMonThoSep->setText(KGlobal::locale()->_monetaryThousandsSeparator);
  edMonPosSign->setText(KGlobal::locale()->_positiveSign);
  edMonNegSign->setText(KGlobal::locale()->_negativeSign);
  edMonFraDig->setText(KGlobal::locale()->formatNumber(KGlobal::locale()->_fracDigits, 0));

  chMonPosPreCurSym->setChecked(KGlobal::locale()->_positivePrefixCurrencySymbol);
  chMonNegPreCurSym->setChecked(KGlobal::locale()->_negativePrefixCurrencySymbol);
  cmbMonPosMonSignPos->setCurrentItem(KGlobal::locale()->_positiveMonetarySignPosition);
  cmbMonNegMonSignPos->setCurrentItem(KGlobal::locale()->_negativeMonetarySignPosition);
}
