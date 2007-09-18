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

void KColorCm::createColorEntry(QString text, QList<KColorButton *> &list, int index)
{
    QTableWidgetItem *label = new QTableWidgetItem(text);

    KColorButton *button = new KColorButton(this);
    button->setObjectName(QString::number(index));
    connect(button, SIGNAL(colorChanged(const QColor &)), this, SLOT(colorChanged(const QColor &)));
    list.append(button);

    colorTable->setItem(index, 0, label);
    colorTable->setCellWidget(index, 1, button);
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
    colorTable->setShowGrid(false);
    colorTable->setRowCount(12);

    createColorEntry(i18n("Normal Background"),    m_backgroundButtons, 0);
    createColorEntry(i18n("Alternate Background"), m_backgroundButtons, 1);
    createColorEntry(i18n("Normal Text"),          m_foregroundButtons, 2);
    createColorEntry(i18n("Inactive Text"),        m_foregroundButtons, 3);
    createColorEntry(i18n("Active Text"),          m_foregroundButtons, 4);
    createColorEntry(i18n("Link Text"),            m_foregroundButtons, 5);
    createColorEntry(i18n("Visited Text"),         m_foregroundButtons, 6);
    createColorEntry(i18n("Negative Text"),        m_foregroundButtons, 7);
    createColorEntry(i18n("Neutral Text"),         m_foregroundButtons, 8);
    createColorEntry(i18n("Positive Text"),        m_foregroundButtons, 9);
    createColorEntry(i18n("Hover Decoration"),     m_decorationButtons, 10);
    createColorEntry(i18n("Focus Decoration"),     m_decorationButtons, 11);

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    // TODO make wide enough for "varies" button, at least for "Common Colors"
    colorTable->horizontalHeader()->setMinimumSectionSize(24);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

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
        for (int i = KColorScheme::NormalBackground; i <= KColorScheme::AlternateBackground; ++i)
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

        for (int i = KColorScheme::FocusColor; i <= KColorScheme::HoverColor; ++i)
        {
            m_decorationButtons[i]->blockSignals(true);
            m_decorationButtons[i]->setColor(m_colorSchemes[currentSet].decoration(KColorScheme::DecorationRole(i)).color());
            m_decorationButtons[i]->blockSignals(false);
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
        if (row <= KColorScheme::AlternateBackground)
        {
            // TODO: need a way to modify this colorscheme
            m_colorSchemes[currentSet].background(KColorScheme::BackgroundRole(row));
        }
        else
        {
            row -= KColorScheme::AlternateBackground;
            if (row <= KColorScheme::PositiveForeground) {
                m_colorSchemes[currentSet].foreground(KColorScheme::ForegroundRole(row));
            }
            else {
                row -= KColorScheme::PositiveForeground;
                m_colorSchemes[currentSet].decoration(KColorScheme::DecorationRole(row));
            }
        }
    }
}

void KColorCm::save()
{

}

#include "colorscm.moc"
