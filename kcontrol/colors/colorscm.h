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

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <KCModule>
#include <KColorScheme>
#include <KPushButton>

#include "ui_colorsettings.h"

class QSlider;
class QPushButton;
class QCheckBox;
class QPalette;
class QStackedWidget;
class KListWidget;

/**
 * The Desktop/Colors tab in kcontrol.
 */
class KColorCm : public KCModule, public Ui::colorSettings
{
    Q_OBJECT

public:
    KColorCm(QWidget *parent, const QVariantList &);
    ~KColorCm();

public Q_SLOTS:

    // load the settings from the config
    virtual void load();

    // save the current settings
    virtual void save();

private slots:

    /** set the colortable color buttons up according to the current colorset */
    void updateColorTable();

    /** slot called when color on a KColorButton changes */
    void colorChanged( const QColor &newColor );

    /** slot called when the contrast slider on the main page changes */
    void on_contrastSlider_valueChanged(int value);

    /** slot called when the shadeSortedColumn checkbox is checked/unchecked */
    void on_shadeSortedColumn_stateChanged(int state);

    /** slot called when any varies button is clicked */
    void variesClicked();

    /** slot called when the schemeList selection changes */
    void loadScheme();
    
    /** slot called when the remove button is clicked*/
    void on_schemeRemoveButton_clicked();

private:

    /** populate the schemeList with color schemes found on the system */
    void populateSchemeList();

    /** update m_colorSchemes contents from the values in m_config */
    void updateColorSchemes();

    /** setup the colortable with its buttons and labels */
    void setupColorTable();
    
    /** setup the effects page */
    void setupEffectsPage();

    /** helper to create color entries */
    void createColorEntry(QString text, QString key, QList<KColorButton *> &list, int index);
    
    /** copy color entries from color schemes into m_config */
    void updateFromColorSchemes();
    
    void changeColor(int row, const QColor &newColor);
    
    /** get the groupKey for the given colorSet */
    static QString colorSetGroupKey(int colorSet);
    
    QColor commonForeground(KColorScheme::ForegroundRole index);

    QList<KColorButton *> m_backgroundButtons;
    QList<KColorButton *> m_foregroundButtons;
    QList<KColorButton *> m_decorationButtons;
    QList<KColorButton *> m_commonColorButtons;
    QList<QStackedWidget *> m_stackedWidgets;
    QStringList m_colorKeys;

    QList<KColorScheme> m_colorSchemes;

    KSharedConfigPtr m_config;
};

#endif
