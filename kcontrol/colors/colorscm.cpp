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

#include "colorscm.h"

#include <QtGui/QHeaderView>
#include <QtGui/QStackedWidget>
#include <QtDBus/QtDBus>

#include <KColorButton>
#include <KColorDialog>
#include <KGenericFactory>
#include <KGlobal>
#include <KGlobalSettings>
#include <KAboutData>
#include <KListWidget>

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
    about->addAuthor( ki18n("Jeremy Whiting"), KLocalizedString(), "jeremy@scitools.com");
    setAboutData( about );

    m_config = KSharedConfig::openConfig("kdeglobals");

    setupUi(this);

    connect(colorSet, SIGNAL(currentIndexChanged(int)), this, SLOT(updateColorTable()));

    load();
}

KColorCm::~KColorCm()
{
    m_config->rollback();
}

void KColorCm::createColorEntry(QString text, QString key, QList<KColorButton *> &list, int index)
{
    KColorButton *button = new KColorButton(this);
    button->setObjectName(QString::number(index));
    connect(button, SIGNAL(changed(const QColor &)), this, SLOT(colorChanged(const QColor &)));
    list.append(button);

    m_colorKeys.insert(index, key);

    KPushButton * variesButton = new KPushButton(NULL);
    variesButton->setText(i18n("Varies"));
    variesButton->setObjectName(QString::number(index));
    connect(variesButton, SIGNAL(clicked()), this, SLOT(variesClicked()));
    
    QStackedWidget * widget = new QStackedWidget(this);
    widget->addWidget(button);
    widget->addWidget(variesButton);
    m_stackedWidgets.append(widget);

    QTableWidgetItem *label = new QTableWidgetItem(text);
    colorTable->setItem(index, 0, label);
    colorTable->setCellWidget(index, 1, widget);
}

void KColorCm::variesClicked()
{
    // find which button was changed
    int row = sender()->objectName().toInt();

    QColor color;
    if(KColorDialog::getColor(color, this ) != QDialog::Rejected ) 
    {
        changeColor(row, color);
        m_stackedWidgets[row]->setCurrentIndex(0);
    }
}

void KColorCm::setupColorTable()
{
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::View, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Window, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Button, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Selection, m_config));
    m_colorSchemes.append(KColorScheme(QPalette::Active, KColorScheme::Tooltip, m_config));

    colorTable->verticalHeader()->hide();
    colorTable->horizontalHeader()->hide();
    colorTable->setShowGrid(false);
    colorTable->setRowCount(12);

    createColorEntry(i18n("Normal Background"),    "BackgroundNormal",    m_backgroundButtons, 0);
    createColorEntry(i18n("Alternate Background"), "BackgroundAlternate", m_backgroundButtons, 1);
    createColorEntry(i18n("Normal Text"),          "ForegroundNormal",    m_foregroundButtons, 2);
    createColorEntry(i18n("Inactive Text"),        "ForegroundInactive",  m_foregroundButtons, 3);
    createColorEntry(i18n("Active Text"),          "ForegroundActive",    m_foregroundButtons, 4);
    createColorEntry(i18n("Link Text"),            "ForegroundLink",      m_foregroundButtons, 5);
    createColorEntry(i18n("Visited Text"),         "ForegroundVisited",   m_foregroundButtons, 6);
    createColorEntry(i18n("Negative Text"),        "ForegroundNegative",  m_foregroundButtons, 7);
    createColorEntry(i18n("Neutral Text"),         "ForegroundNeutral",   m_foregroundButtons, 8);
    createColorEntry(i18n("Positive Text"),        "ForegroundPositive",  m_foregroundButtons, 9);
    createColorEntry(i18n("Focus Decoration"),     "DecorationFocus",     m_decorationButtons, 10);
    createColorEntry(i18n("Hover Decoration"),     "DecorationHover",     m_decorationButtons, 11);

    colorTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    int minWidth = QPushButton(i18n("Varies")).minimumSizeHint().width();
    colorTable->horizontalHeader()->setMinimumSectionSize(minWidth);
    colorTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);

    updateColorTable();
}

QColor KColorCm::commonBackground(KColorScheme::BackgroundRole index)
{
    QColor temp = m_colorSchemes[KColorScheme::View].background(index).color();
    for (int i = KColorScheme::Window; i < KColorScheme::Tooltip; ++i)
    {
        if (i != KColorScheme::Selection && m_colorSchemes[i].background(index).color() != temp)
        {
            temp = QColor(); // make it an invalid color
            break;
        }
    }

    return temp;
}

QColor KColorCm::commonForeground(KColorScheme::ForegroundRole index)
{
    QColor temp = m_colorSchemes[KColorScheme::View].foreground(index).color();
    for (int i = KColorScheme::Window; i < KColorScheme::Tooltip; ++i)
    {
        if (i != KColorScheme::Selection && m_colorSchemes[i].foreground(index).color() != temp)
        {
            temp = QColor(); // make it an invalid color
            break;
        }
    }

    return temp;
}

QColor KColorCm::commonDecoration(KColorScheme::DecorationRole index)
{
    QColor temp = m_colorSchemes[KColorScheme::View].decoration(index).color();
    for (int i = KColorScheme::Window; i < KColorScheme::Tooltip; ++i)
    {
        if (i != KColorScheme::Selection && m_colorSchemes[i].decoration(index).color() != temp)
        {
            temp = QColor(); // make it an invalid color
            break;
        }
    }

    return temp;
}

void KColorCm::updateColorTable()
{
    // subtract one here since the 0 item  is "Common Colors"
    int currentSet = colorSet->currentIndex() - 1;

    if (currentSet == -1)
    {
        // common colors is selected
        for (int i = KColorScheme::NormalBackground; i <= KColorScheme::AlternateBackground; ++i)
        {
            QColor backgroundColor = commonBackground(KColorScheme::BackgroundRole(i));
            if (backgroundColor.isValid())
            {
                m_backgroundButtons[i]->blockSignals(true);
                m_backgroundButtons[i]->setColor(backgroundColor);
                m_stackedWidgets[i]->setCurrentIndex(0);
                m_backgroundButtons[i]->blockSignals(false);
            }
            else
            {
                // replace background button i with a KPushButton with text "Varies"
                m_stackedWidgets[i]->setCurrentIndex(1);
            }
        }

        for (int i = KColorScheme::NormalText; i <= KColorScheme::PositiveText; ++i)
        {
            int row = i + KColorScheme::AlternateBackground;
            QColor foregroundColor = commonForeground(KColorScheme::ForegroundRole(i));
            if (foregroundColor.isValid())
            {
                m_foregroundButtons[i]->blockSignals(true);
                m_foregroundButtons[i]->setColor(foregroundColor);
                m_stackedWidgets[row]->setCurrentIndex(0);
                m_foregroundButtons[i]->blockSignals(false);
            }
            else
            {
                // replace foreground button i with a KPushButton with text "Varies"
                m_stackedWidgets[row]->setCurrentIndex(1);
            }
        }

        for (int i = KColorScheme::FocusColor; i <= KColorScheme::HoverColor; ++i)
        {
            int row = i + KColorScheme::AlternateBackground + KColorScheme::PositiveText;
            QColor decorationColor = commonDecoration(KColorScheme::DecorationRole(i));
            if (decorationColor.isValid())
            {
                m_decorationButtons[i]->blockSignals(true);
                m_decorationButtons[i]->setColor(decorationColor);
                m_stackedWidgets[row]->setCurrentIndex(0);
                m_decorationButtons[i]->blockSignals(false);
            }
            else
            {
                // replace decoration button i with a KPushButton with text "Varies"
                m_stackedWidgets[row]->setCurrentIndex(1);
            }
        }
    }
    else
    {
        // a real color set is selected
        for (int i = KColorScheme::NormalBackground; i <= KColorScheme::AlternateBackground; ++i)
        {
            m_backgroundButtons[i]->blockSignals(true);
            m_backgroundButtons[i]->setColor(m_colorSchemes[currentSet].background(KColorScheme::BackgroundRole(i)).color());
            m_stackedWidgets[i]->setCurrentIndex(0);
            m_backgroundButtons[i]->blockSignals(false);
        }

        for (int i = KColorScheme::NormalText; i <= KColorScheme::PositiveText; ++i)
        {
            int row = i + KColorScheme::AlternateBackground;
            m_foregroundButtons[i]->blockSignals(true);
            m_foregroundButtons[i]->setColor(m_colorSchemes[currentSet].foreground(KColorScheme::ForegroundRole(i)).color());
            m_stackedWidgets[row]->setCurrentIndex(0);
            m_foregroundButtons[i]->blockSignals(false);
        }

        for (int i = KColorScheme::FocusColor; i <= KColorScheme::HoverColor; ++i)
        {
            int row = i + KColorScheme::AlternateBackground + KColorScheme::PositiveText;
            m_decorationButtons[i]->blockSignals(true);
            m_decorationButtons[i]->setColor(m_colorSchemes[currentSet].decoration(KColorScheme::DecorationRole(i)).color());
            m_stackedWidgets[row]->setCurrentIndex(0);
            m_decorationButtons[i]->blockSignals(false);
        }
    }
}

void KColorCm::colorChanged( const QColor &newColor )
{
    // find which button was changed
    int row = sender()->objectName().toInt();
    changeColor(row, newColor);
}

void KColorCm::changeColor(int row, const QColor &newColor)
{
    // update the m_colorSchemes for the selected colorSet
    int currentSet = colorSet->currentIndex() - 1;

    if (currentSet == -1)
    {
        // common colors is selected
        KConfigGroup(m_config, "Colors:Window").writeEntry(m_colorKeys[row], newColor);
        KConfigGroup(m_config, "Colors:Button").writeEntry(m_colorKeys[row], newColor);
        //KConfigGroup(m_config, "Colors:Selection").writeEntry(m_colorKeys[row], newColor);
        KConfigGroup(m_config, "Colors:Tooltip").writeEntry(m_colorKeys[row], newColor);
        KConfigGroup(m_config, "Colors:View").writeEntry(m_colorKeys[row], newColor);
    }
    else
    {
        const char *group;
        switch (currentSet) {
            case KColorScheme::Window:
                group = "Colors:Window";
                break;
            case KColorScheme::Button:
                group = "Colors:Button";
                break;
            case KColorScheme::Selection:
                group = "Colors:Selection";
                break;
            case KColorScheme::Tooltip:
                group = "Colors:Tooltip";
                break;
            default:
                group = "Colors:View";
        }
        KConfigGroup(m_config, group).writeEntry(m_colorKeys[row], newColor);
    }

    schemePreview->setPalette(m_config);
    inactivePreview->setPalette(m_config, QPalette::Inactive);
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    emit changed(true);
}

void KColorCm::on_contrastSlider_valueChanged(int value)
{
    KConfigGroup group(m_config, "KDE");
    group.writeEntry("contrast", value);

    schemePreview->setPalette(m_config);
    inactivePreview->setPalette(m_config, QPalette::Inactive);
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    emit changed(true);
}

void KColorCm::on_shadeSortedColumn_stateChanged(int state)
{
    KConfigGroup group(m_config, "General");
    group.writeEntry("shadeSortColumn", (bool)state);

    emit changed(true);
}

void KColorCm::load()
{
    // rollback the config, in case we have changed the in-memory kconfig
    m_config->rollback();
    
    setupColorTable();
    contrastSlider->setValue(KGlobalSettings::contrast());
    shadeSortedColumn->setCheckState(KGlobalSettings::shadeSortColumn() ?
        Qt::Checked : Qt::Unchecked);

    schemePreview->setPalette(m_config);
    inactivePreview->setPalette(m_config, QPalette::Inactive);
    disabledPreview->setPalette(m_config, QPalette::Disabled);

    emit changed(false);
}

void KColorCm::save()
{
    m_config->sync();
    KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
#ifdef Q_WS_X11
    // Send signal to all kwin instances
    QDBusMessage message =
       QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(message);
#endif

    emit changed(false);
}

#include "colorscm.moc"
