/*
 * klocalesample.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
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

#include <qdatetime.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>

#include "klocaleadv.h"
#include "klocalesample.h"

extern KLocaleAdvanced *locale;

KLocaleSample::KLocaleSample(QWidget *parent, const char*name)
  : QWidget(parent, name)
{
  QGridLayout *lay = new QGridLayout(this, 5, 2);
  lay->setAutoAdd(TRUE);

  labNumber = new QLabel(this, I18N_NOOP("Numbers:"));
  numberSample = new QLabel(this);

  labMoney = new QLabel(this, I18N_NOOP("Money:"));
  moneySample = new QLabel(this);

  labDate = new QLabel(this, I18N_NOOP("Date:"));
  dateSample = new QLabel(this);

  labDateShort = new QLabel(this, I18N_NOOP("Short date:"));
  dateShortSample = new QLabel(this);

  labTime = new QLabel(this, I18N_NOOP("Time:"));
  timeSample = new QLabel(this);

  lay->setColStretch(0, 1);
  lay->setColStretch(1, 3);
}

KLocaleSample::~KLocaleSample()
{
}

void KLocaleSample::update()
{
  numberSample->setText(locale->formatNumber(1234567.89) +
			QString::fromLatin1(" / ") +
			locale->formatNumber(-1234567.89));

  moneySample->setText(locale->formatMoney(123456789.00) +
		       QString::fromLatin1(" / ") +
		       locale->formatMoney(-123456789.00));
  dateSample->setText(locale->formatDate(QDate::currentDate(), false));
  dateShortSample->setText(locale->formatDate(QDate::currentDate(), true));
  timeSample->setText(locale->formatTime(QTime::currentTime()));

  QString str;

  str = locale->translate("This is how numbers will be displayed.");
  QWhatsThis::add( labNumber,  str );
  QWhatsThis::add( numberSample, str );

  str = locale->translate("This is how monetary values will be displayed.");
  QWhatsThis::add( labMoney,    str );
  QWhatsThis::add( moneySample, str );   

  str = locale->translate("This is how date values will be displayed.");
  QWhatsThis::add( labDate,    str );
  QWhatsThis::add( dateSample, str ); 

  str = locale->translate("This is how date values will be displayed using "
			  "a short notation.");
  QWhatsThis::add( labDateShort, str );
  QWhatsThis::add( dateShortSample, str );    

  str = locale->translate("This is how the time will be displayed.");
  QWhatsThis::add( labTime,    str );
  QWhatsThis::add( timeSample, str );       
}
