/* KDE Display color scheme setup module
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jeremy@scitools.com>
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
    
    connect(colorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(updateColorTable()));

    // connect signals/slots
    // TODO

    // finally, add UI's to tab widget
}

KColorCm::~KColorCm()
{
}

void KColorCm::setupColorTable()
{
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::View));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Window));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Button));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Selection));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Tooltip));
    
    colorTable->verticalHeader()->hide();
    colorTable->horizontalHeader()->hide();
    QTableWidgetItem *label = new QTableWidgetItem(i18n("Normal Background"));
    colorTable->setItem(0, 0, label);
    label = new QTableWidgetItem(i18n("Alternate Background"));
    colorTable->setItem(1, 0, label);
    label = new QTableWidgetItem(i18n("Active Background"));
    colorTable->setItem(2, 0, label);
    label = new QTableWidgetItem(i18n("Link Background"));
    colorTable->setItem(3, 0, label);
    label = new QTableWidgetItem(i18n("Visited Background"));
    colorTable->setItem(4, 0, label);
    label = new QTableWidgetItem(i18n("Negative Background"));
    colorTable->setItem(5, 0, label);
    label = new QTableWidgetItem(i18n("Neutral Background"));
    colorTable->setItem(6, 0, label);
    label = new QTableWidgetItem(i18n("Positive Background"));
    colorTable->setItem(7, 0, label);

    label = new QTableWidgetItem(i18n("Normal Text"));
    colorTable->setItem(8, 0, label);
    label = new QTableWidgetItem(i18n("Inactive Text"));
    colorTable->setItem(9, 0, label);
    label = new QTableWidgetItem(i18n("Active Text"));
    colorTable->setItem(10, 0, label);
    label = new QTableWidgetItem(i18n("Link Text"));
    colorTable->setItem(11, 0, label);
    label = new QTableWidgetItem(i18n("Visited Text"));
    colorTable->setItem(12, 0, label);
    label = new QTableWidgetItem(i18n("Negative Text"));
    colorTable->setItem(13, 0, label);
    label = new QTableWidgetItem(i18n("Neutral Text"));
    colorTable->setItem(14, 0, label);
    label = new QTableWidgetItem(i18n("Positive Text"));
    colorTable->setItem(15, 0, label);

    KColorButton *button;
    
    for (int i = KColorScheme::NormalBackground; i <= KColorScheme::PositiveBackground; ++i)
    {
        button = new KColorButton(this);
        button->setObjectName(QString::number(i));
        colorTable->setCellWidget(i, 1, button);
        connect(button, SIGNAL(colorChanged(const QColor &)), this, SLOT(colorChanged(const QColor &)));
        m_backgroundButtons.append(button);
    }

    for (int i = KColorScheme::NormalText; i <= KColorScheme::PositiveText; ++i)
    {
        button = new KColorButton(this);
        button->setObjectName(QString::number(i + m_backgroundButtons.size()));
        colorTable->setCellWidget(i + m_backgroundButtons.size(), 1, button);
        connect(button, SIGNAL(colorChanged(const QColor &)), this, SLOT(colorChanged(const QColor &)));
        m_foregroundButtons.append(button);
    }

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    
    updateColorTable();
}

void KColorCm::updateColorTable()
{
    // subtract one here since the 0 item  is "Common Colors"
    int currentSet = colorSet->currentIndex() - 1;

    if (currentSet < 0)
    {
        // common colors is selected
    }
    else
    {
        // a real color set is selected
        for (int i = KColorScheme::NormalBackground; i <= KColorScheme::PositiveBackground; ++i)
        {
            m_backgroundButtons[i]->blockSignals(true);
            m_backgroundButtons[i]->setColor(m_colorSchemes[currentSet].background(KColorScheme::BackgroundRole(i)).color());
            m_backgroundButtons[i]->blockSignals(false);
        }
        
        for (int i = KColorScheme::NormalText; i <= KColorScheme::PositiveText; ++i)
        {
            m_foregroundButtons[i]->blockSignals(true);
            m_foregroundButtons[i]->setColor(m_colorSchemes[currentSet].foreground(KColorScheme::ForegroundRole(i)).color());
            m_foregroundButtons[i]->blockSignals(false);
        }
    }
}

void KColorCm::colorChanged( const QColor &newColor )
{
    // find which button was changed
    int row = sender()->objectName().toInt();
    // update the m_colorSchemes for the selected colorSet
    int currentSet = colorSet->currentIndex() - 1;

    if (currentSet < 0)
    {
        // common colors is selected
    }
    else
    {
        // NOTE: this is dependent upon the background color buttons all being before
        // any text color buttons
        if (row <= KColorScheme::PositiveBackground)
        {
            // TODO: need a way to modify this colorscheme
            m_colorSchemes[currentSet].background(KColorScheme::BackgroundRole(row));
        }
        else 
        {
            row -= KColorScheme::PositiveBackground;
            m_colorSchemes[currentSet].foreground(KColorScheme::ForegroundRole(row));
        }
    }
}

void KColorCm::save()
{
    
}

#include "colorscm.moc"
