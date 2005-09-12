/*
 * klocalesample.cpp
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
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

#include <qdatetime.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qtimer.h>

#include <stdio.h>

#include <klocale.h>

#include "klocalesample.h"
#include "klocalesample.moc"

KLocaleSample::KLocaleSample(KLocale *locale,
                             QWidget *parent, const char*name)
  : QWidget(parent, name),
    m_locale(locale)
{
  QGridLayout *lay = new QGridLayout(this, 5, 2);
  lay->setAutoAdd(TRUE);

  // Whatever the color scheme is, we want black text
  QColorGroup a = palette().active();
  a.setColor(QColorGroup::Foreground, Qt::black);
  QPalette pal(a, a, a);

  m_labNumber = new QLabel(this, I18N_NOOP("Numbers:"));
  m_labNumber->setPalette(pal);
  m_numberSample = new QLabel(this);
  m_numberSample->setPalette(pal);

  m_labMoney = new QLabel(this, I18N_NOOP("Money:"));
  m_labMoney->setPalette(pal);
  m_moneySample = new QLabel(this);
  m_moneySample->setPalette(pal);

  m_labDate = new QLabel(this, I18N_NOOP("Date:"));
  m_labDate->setPalette(pal);
  m_dateSample = new QLabel(this);
  m_dateSample->setPalette(pal);

  m_labDateShort = new QLabel(this, I18N_NOOP("Short date:"));
  m_labDateShort->setPalette(pal);
  m_dateShortSample = new QLabel(this);
  m_dateShortSample->setPalette(pal);

  m_labTime = new QLabel(this, I18N_NOOP("Time:"));
  m_labTime->setPalette(pal);
  m_timeSample = new QLabel(this);
  m_timeSample->setPalette(pal);

  lay->setColStretch(0, 1);
  lay->setColStretch(1, 3);

  QTimer *timer = new QTimer(this, "clock_timer");
  connect(timer, SIGNAL(timeout()), this, SLOT(slotUpdateTime()));
  timer->start(1000);
}

KLocaleSample::~KLocaleSample()
{
}

void KLocaleSample::slotUpdateTime()
{
  QDateTime dt = QDateTime::currentDateTime();

  m_dateSample->setText(m_locale->formatDate(dt.date(), false));
  m_dateShortSample->setText(m_locale->formatDate(dt.date(), true));
  m_timeSample->setText(m_locale->formatTime(dt.time(), true));
}

void KLocaleSample::slotLocaleChanged()
{
  m_numberSample->setText(m_locale->formatNumber(1234567.89) +
                          QLatin1String(" / ") +
                          m_locale->formatNumber(-1234567.89));

  m_moneySample->setText(m_locale->formatMoney(123456789.00) +
                         QLatin1String(" / ") +
                         m_locale->formatMoney(-123456789.00));

  slotUpdateTime();

  QString str;

  str = m_locale->translate("This is how numbers will be displayed.");
  QWhatsThis::add( m_labNumber,  str );
  QWhatsThis::add( m_numberSample, str );

  str = m_locale->translate("This is how monetary values will be displayed.");
  QWhatsThis::add( m_labMoney,    str );
  QWhatsThis::add( m_moneySample, str );

  str = m_locale->translate("This is how date values will be displayed.");
  QWhatsThis::add( m_labDate,    str );
  QWhatsThis::add( m_dateSample, str );

  str = m_locale->translate("This is how date values will be displayed using "
                            "a short notation.");
  QWhatsThis::add( m_labDateShort, str );
  QWhatsThis::add( m_dateShortSample, str );

  str = m_locale->translate("This is how the time will be displayed.");
  QWhatsThis::add( m_labTime,    str );
  QWhatsThis::add( m_timeSample, str );
}
