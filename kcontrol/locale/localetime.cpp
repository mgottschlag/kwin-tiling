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

#include <kglobal.h>

#define KLocaleConfigAdvanced KLocaleConfigTime
#include <klocale.h>
#undef KLocaleConfigAdvanced

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>

#include "klocalesample.h"
#include "main.h"
#include "localetime.h"
#include "localetime.moc"

#define i18n(a) (a)

KLocaleConfigTime::KLocaleConfigTime(QWidget *parent, const char*name)
 : KConfigWidget(parent, name)
{
  QLabel *label;
  QGroupBox *gbox;

  QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);

  // Time
  gbox = new QGroupBox(this, i18n("Date && Time"));
  tl->addWidget(gbox);

  QGridLayout *tl1 = new QGridLayout(gbox, 2, 4, 5);
  tl1->addRowSpacing(0, 15);
  tl1->addRowSpacing(4, 10);
  tl1->addColSpacing(0, 10);
  tl1->addColSpacing(3, 10);
  tl1->setColStretch(2, 1); 

  label = new QLabel(i18n("Time format"), gbox);
  edTimeFmt = new QLineEdit(gbox);
  connect( edTimeFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotTimeFmtChanged(const QString &) ) );
  tl1->addWidget(label, 1, 1);
  tl1->addWidget(edTimeFmt, 1, 2);

  label = new QLabel(i18n("Date format"), gbox);
  edDateFmt = new QLineEdit(gbox);
  connect( edDateFmt, SIGNAL( textChanged(const QString &) ), this, SLOT( slotDateFmtChanged(const QString &) ) );
  tl1->addWidget(label, 2, 1);
  tl1->addWidget(edDateFmt, 2, 2);

  tl1->activate();

  // Examples
  gbox = new QGroupBox("1", this, i18n("Examples"));
  tl->addWidget(gbox);
  sample = new KLocaleSample(gbox);

  syncWithKLocaleTime();
}

KLocaleConfigTime::~KLocaleConfigTime()
{
}

void KLocaleConfigTime::loadSettings()
{
}

void KLocaleConfigTime::applySettings()
{
  KConfigBase *config = KGlobal::config();

  config->setGroup("Locale");
  KSimpleConfig ent(locate("locale", "l10n/" + KGlobal::locale()->time + "/entry.desktop"), true);
  ent.setGroup("KCM Locale");

  QString str;

  str = ent.readEntry("TimeFormat");
  if (str != edTimeFmt->text())
    config->writeEntry("TimeFormat", edTimeFmt->text());

  str = ent.readEntry("DateFormat");
  if (str != edDateFmt->text())
    config->writeEntry("DateFormat", edDateFmt->text());
}

void KLocaleConfigTime::defaultSettings()
{
}

void KLocaleConfigTime::updateSample()
{
  if (sample)
    sample->update();
}

void KLocaleConfigTime::slotTimeFmtChanged(const QString &t)
{
  KGlobal::locale()->_timefmt = t;
  ((KLocaleApplication*)kapp)->updateSample();
}

void KLocaleConfigTime::slotDateFmtChanged(const QString &t)
{
  KGlobal::locale()->_datefmt = t;
  sample->update();
}

void KLocaleConfigTime::syncWithKLocaleTime()
{
  edTimeFmt->setText(KGlobal::locale()->_timefmt);
  edDateFmt->setText(KGlobal::locale()->_datefmt);
}
