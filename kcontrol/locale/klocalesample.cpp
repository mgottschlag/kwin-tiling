/*
 * klocalesample.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
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

#include <qdatetime.h>
#include <qlabel.h>

#include <kglobal.h>
#include <klocale.h>

#include "klocalesample.h"

#define i18n(a) (a)
KLocaleSample::KLocaleSample(QWidget *parent, const char*name)
  : QGridLayout(parent, 6, 4, 5, -1, name)
{
    QLabel *label;

    addRowSpacing(0, 15);
    addRowSpacing(5, 10);
    addColSpacing(0, 10);
    addColSpacing(3, 10);
    setColStretch(2, 1);

    label = new QLabel("1", parent, i18n("Numbers:"));
    addWidget(label, 1, 1);

    numberSample = new QLabel(parent);
    addWidget(numberSample, 1, 2);

    label = new QLabel("1", parent, i18n("Money:"));
    addWidget(label, 2, 1);

    moneySample = new QLabel(parent);
    addWidget(moneySample, 2, 2);

    label = new QLabel("1", parent, i18n("Date:"));
    addWidget(label, 3, 1);

    dateSample = new QLabel(parent);
    addWidget(dateSample, 3, 2);

    label = new QLabel("1", parent, i18n("Short date:"));
    addWidget(label, 4, 1);

    dateShortSample = new QLabel(parent);
    addWidget(dateShortSample, 4, 2);

    label = new QLabel("1", parent, i18n("Time:"));
    addWidget(label, 5, 1);

    timeSample = new QLabel(parent);
    addWidget(timeSample, 5, 2);
}

KLocaleSample::~KLocaleSample()
{
}

void KLocaleSample::update()
{
  KLocale *locale = KGlobal::locale();

  numberSample->setText(locale->formatNumber(1234567.89) +
			" / " +
			locale->formatNumber(-1234567.89));

  moneySample->setText(locale->formatMoney(123456789.00) +
		       " / " +
		       locale->formatMoney(-123456789.00));
  dateSample->setText(locale->formatDate(QDate::currentDate()));
  dateShortSample->setText(locale->formatDate(QDate::currentDate(), true));
  timeSample->setText(locale->formatTime(QTime::currentTime()));
}
