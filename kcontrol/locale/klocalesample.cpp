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

#include <kglobal.h>
#include <klocale.h>

#include "klocaleadv.h"
#include "klocalesample.h"

extern KLocaleAdvanced *locale;

KLocaleSample::KLocaleSample(QWidget *parent, const char*name)
  : QGridLayout(parent, 6, 4, 5, -1, name)
{
    QLabel *label;
    QString wtstr;

    addRowSpacing(0, 15);
    addRowSpacing(5, 10);
    addColSpacing(0, 10);
    addColSpacing(3, 10);
    setColStretch(2, 1);

    label = new QLabel(parent, I18N_NOOP("Numbers:"));
    addWidget(label, 1, 1);

    numberSample = new QLabel(parent);
    addWidget(numberSample, 1, 2);

    wtstr = i18n("This is how numbers will be displayed.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( numberSample, wtstr );

    label = new QLabel(parent, I18N_NOOP("Money:"));
    addWidget(label, 2, 1);

    moneySample = new QLabel(parent);
    addWidget(moneySample, 2, 2);

    wtstr = i18n("This is how monetary values will be displayed.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( moneySample, wtstr );

    label = new QLabel(parent, I18N_NOOP("Date:"));
    addWidget(label, 3, 1);

    dateSample = new QLabel(parent);
    addWidget(dateSample, 3, 2);

    wtstr = i18n("This is how date values will be displayed.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( dateSample, wtstr );

    label = new QLabel(parent, I18N_NOOP("Short date:"));
    addWidget(label, 4, 1);

    dateShortSample = new QLabel(parent);
    addWidget(dateShortSample, 4, 2);

    wtstr = i18n("This is how date values will be displayed using a short notation.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( dateShortSample, wtstr );

    label = new QLabel(parent, I18N_NOOP("Time:"));
    addWidget(label, 5, 1);

    timeSample = new QLabel(parent);
    addWidget(timeSample, 5, 2);

    wtstr = i18n("This is how the time will be displayed.");
    QWhatsThis::add( label, wtstr );
    QWhatsThis::add( timeSample, wtstr );
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
}
