/*
 * localetime.cpp
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

#define KLocaleConfigAdvanced KLocaleConfigTime
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "main.h"
#include "localetime.h"
#include "localetime.moc"

#define i18n(a) (a)

KLocaleConfigTime::KLocaleConfigTime(QWidget *parent, const char*name)
 : QWidget(parent, name)
{
  QLabel *label;
  QGroupBox *gbox;

  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);

  // Time
  gbox = new QGroupBox(this, i18n("Date && Time"));
  tl->addWidget(gbox);

  QGridLayout *tl1 = new QGridLayout(gbox, 3, 4, 5);
  tl1->addRowSpacing(0, 15);
  tl1->addRowSpacing(4, 10);
  tl1->addColSpacing(0, 10);
  tl1->addColSpacing(3, 10);
  tl1->setColStretch(2, 1); 

  label = new QLabel("1", gbox, i18n("Time format"));
  edTimeFmt = new QLineEdit(gbox);
  connect( edTimeFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotTimeFmtChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edTimeFmt, 1, 2);

  label = new QLabel("1", gbox, i18n("Date format"));
  edDateFmt = new QLineEdit(gbox);
  connect( edDateFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edDateFmt, 2, 2);

  label = new QLabel("1", gbox, i18n("Short date format"));
  edDateFmtShort = new QLineEdit(gbox);
  connect( edDateFmtShort, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtShortChanged(const QString &) ) );
  tl1->addWidget(label, 3, 1);
  tl1->addWidget(edDateFmtShort, 3, 2);

  tl1->activate();

  tl->addStretch(1);
  tl->activate();

  load();
}

KLocaleConfigTime::~KLocaleConfigTime()
{
}

void KLocaleConfigTime::load()
{
  KLocale *locale = KGlobal::locale();

  edTimeFmt->setText(locale->_timefmt);
  edDateFmt->setText(locale->_datefmt);
  edDateFmtShort->setText(locale->_datefmtshort);
}

void KLocaleConfigTime::save()
{
  KSimpleConfig a(0, false);
  KLocale *locale = KGlobal::locale();
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("TimeFormat", "%I:%M:%S %p");
  str = str==locale->_timefmt?QString::null:locale->_timefmt;
  config->writeEntry("TimeFormat", str, true, true);

  str = ent.readEntry("DateFormat", "%A %d %B %Y");
  str = str==locale->_datefmt?QString::null:locale->_datefmt;
  config->writeEntry("DateFormat", str, true, true);

  str = ent.readEntry("DateFormatShort", "%m/%d/%y");
  str = str==locale->_datefmtshort?QString::null:locale->_datefmtshort;
  config->writeEntry("DateFormatShort", str, true, true);

  config->sync();
}

void KLocaleConfigTime::defaults()
{
}

void KLocaleConfigTime::slotTimeFmtChanged(const QString &t)
{
  KGlobal::locale()->_timefmt = t;
  emit resample();
}

void KLocaleConfigTime::slotDateFmtChanged(const QString &t)
{
  KGlobal::locale()->_datefmt = t;
  emit resample();
}

void KLocaleConfigTime::slotDateFmtShortChanged(const QString &t)
{
  KGlobal::locale()->_datefmtshort = t;
  emit resample();
}

void KLocaleConfigTime::reset()
{
  KLocale *locale = KGlobal::locale();

  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  locale->_timefmt = ent.readEntry("TimeFormat", "%I:%M:%S %p");
  locale->_datefmt = ent.readEntry("DateFormat", "%A %d %B %Y");
  locale->_datefmtshort = ent.readEntry("DateFormatShort", "%m/%d/%y");

  load();
}
