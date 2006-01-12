/*
 * localeother.cpp
 *
 * Copyright (c) 2001-2003 Hans Petter Bieker <bieker@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qprinter.h>

#include <kdialog.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "localeother.h"
#include "localeother.moc"


KLocaleConfigOther::KLocaleConfigOther(KLocale *locale,
                                       QWidget *parent, const char*name)
  : QWidget(parent, name),
    m_locale(locale)
{
  // Other
  QGridLayout *lay = new QGridLayout(this, 3, 2,
                                     KDialog::marginHint(),
                                     KDialog::spacingHint());

  m_labPageSize = new QLabel(this, I18N_NOOP("Paper format:"));
  lay->addWidget(m_labPageSize, 0, 0);
  m_combPageSize = new QComboBox(this);
  lay->addWidget(m_combPageSize, 0, 1);
  connect( m_combPageSize, SIGNAL( activated(int) ),
           SLOT( slotPageSizeChanged(int) ) );

  m_labMeasureSystem = new QLabel(this, I18N_NOOP("Measure system:"));
  lay->addWidget(m_labMeasureSystem, 1, 0);
  m_combMeasureSystem = new QComboBox(this);
  lay->addWidget(m_combMeasureSystem, 1, 1);
  connect( m_combMeasureSystem, SIGNAL( activated(int) ),
           SLOT( slotMeasureSystemChanged(int) ) );

  m_combPageSize->insertItem(QString());
  m_combPageSize->insertItem(QString());
  m_combMeasureSystem->insertItem(QString());
  m_combMeasureSystem->insertItem(QString());

  lay->setColStretch(1, 1);
  lay->addRowSpacing(2, 0);

  adjustSize();
}

KLocaleConfigOther::~KLocaleConfigOther()
{
}

void KLocaleConfigOther::save()
{
  KConfig *config = KGlobal::config();
  KConfigGroup group(config, "Locale");

  KSimpleConfig ent(locate("locale",
                           QString::fromLatin1("l10n/%1/entry.desktop")
                           .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  // ### HPB: Add code here
  int i;
  i = ent.readEntry("PageSize", (int)QPrinter::A4);
  group.deleteEntry("PageSize", KConfigBase::Global);
  if (i != m_locale->pageSize())
    group.writeEntry("PageSize",
                       m_locale->pageSize(), KConfigBase::Persistent|KConfigBase::Global);

  i = ent.readEntry("MeasureSystem", (int)KLocale::Metric);
  group.deleteEntry("MeasureSystem", KConfigBase::Global);
  if (i != m_locale->measureSystem())
    group.writeEntry("MeasureSystem",
                       int(m_locale->measureSystem()), KConfigBase::Persistent|KConfigBase::Global);

  group.sync();
}

void KLocaleConfigOther::slotLocaleChanged()
{
  m_combMeasureSystem->setCurrentItem(m_locale->measureSystem());

  int pageSize = m_locale->pageSize();

  int i = 0; // default to A4
  if ( pageSize == (int)QPrinter::Letter )
    i = 1;
  m_combPageSize->setCurrentItem(i);
}

void KLocaleConfigOther::slotTranslate()
{
  m_combMeasureSystem->changeItem( m_locale->translate("The Metric System",
                                                       "Metric"), 0 );
  m_combMeasureSystem->changeItem( m_locale->translate("The Imperial System",
                                                       "Imperial"), 1 );

  m_combPageSize->changeItem( m_locale->translate("A4"), 0 );
  m_combPageSize->changeItem( m_locale->translate("US Letter"), 1 );
}

void KLocaleConfigOther::slotPageSizeChanged(int i)
{
  QPrinter::PageSize pageSize = QPrinter::A4;

  if ( i == 1 )
    pageSize = QPrinter::Letter;

  m_locale->setPageSize((int)pageSize);
  emit localeChanged();
}

void KLocaleConfigOther::slotMeasureSystemChanged(int i)
{
  m_locale->setMeasureSystem((KLocale::MeasureSystem)i);
  emit localeChanged();
}
