/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// FIXME QFrame included only for placeholders

#include <QtGui/QHeaderView>

#include <KColorButton>
#include <KGenericFactory>
#include <KGlobalSettings>
#include <KAboutData>
#include <KListWidget>

#include "colorscm.h"

K_PLUGIN_FACTORY( KolorFactory, registerPlugin<KColorCm>(); )
K_EXPORT_PLUGIN( KolorFactory("kcmcolors") )

KColorCm::KColorCm(QWidget *parent, const QVariantList &)
    : KCModule( KolorFactory::componentData(), parent )
{
    KAboutData* about = new KAboutData(
        "kcmcolors", 0, ki18n("Colors"), 0, KLocalizedString(),
        KAboutData::License_GPL,
        ki18n("(c) 2007 Matthew Woehlke")
    );
    about->addAuthor( ki18n("Matthew Woehlke"), KLocalizedString(),
                     "mw_triad@users.sourceforge.net" );
    setAboutData( about );

    setupUi(this);
    
    setupColorTable();

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
}

KColorCm::~KColorCm()
{
}

void KColorCm::setupColorTable()
{
    colorTable->verticalHeader()->hide();
    colorTable->horizontalHeader()->hide();
    QTableWidgetItem *label = new QTableWidgetItem(i18n("Normal Background"));
    colorTable->setItem(0, 0, label);
    KColorButton *button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(0, 1, button);
    
    label = new QTableWidgetItem(i18n("Alternate Background"));
    colorTable->setItem(1, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(1, 1, button);    
    
    label = new QTableWidgetItem(i18n("Active Background"));
    colorTable->setItem(2, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(2, 1, button);    
    
    label = new QTableWidgetItem(i18n("Link Background"));
    colorTable->setItem(3, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(3, 1, button);    
    
    label = new QTableWidgetItem(i18n("Visited Background"));
    colorTable->setItem(4, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(4, 1, button);    
    
    label = new QTableWidgetItem(i18n("Negative Background"));
    colorTable->setItem(5, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(5, 1, button);    
    
    label = new QTableWidgetItem(i18n("Neutral Background"));
    colorTable->setItem(6, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(6, 1, button);    
    
    label = new QTableWidgetItem(i18n("Positive Background"));
    colorTable->setItem(7, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(7, 1, button);    
    
    label = new QTableWidgetItem(i18n("Normal Text"));
    colorTable->setItem(8, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(8, 1, button);    
    
    label = new QTableWidgetItem(i18n("Inactive Text"));
    colorTable->setItem(9, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(9, 1, button);    
    
    label = new QTableWidgetItem(i18n("Active Text"));
    colorTable->setItem(10, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(10, 1, button);    
    
    label = new QTableWidgetItem(i18n("Link Text"));
    colorTable->setItem(11, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(11, 1, button);    
    
    label = new QTableWidgetItem(i18n("Negative Text"));
    colorTable->setItem(12, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(12, 1, button);    
    
    label = new QTableWidgetItem(i18n("Neutral Text"));
    colorTable->setItem(13, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(13, 1, button);    
    
    label = new QTableWidgetItem(i18n("Positive Text"));
    colorTable->setItem(14, 0, label);
    button = new KColorButton(this);
    m_backgroundButtons.append(button);
    colorTable->setCellWidget(14, 1, button);    

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}

#include "colorscm.moc"
