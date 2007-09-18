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

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <kcmodule.h>
#include <kcolorscheme.h>

#include "ui_colorsettings.h"

class QSlider;
class QPushButton;
class QCheckBox;
class QPalette;
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

    // save the current settings
    virtual void save();

private slots:

    /** set the colortable color buttons up according to the current colorset */
    void updateColorTable();

    /** slot called when color on a KColorButton changes */
    void colorChanged( const QColor &newColor );

private:

    /** setup the colortable with its buttons and labels */
    void setupColorTable();

    /** helper to create color entries */
    void createColorEntry(QString text, QList<KColorButton *> &list, int index);

    // these are lists of QPushButtons so they can be KColorButtons, or KPushButtons when
    // they say "Varies"
    QList<KColorButton *> m_backgroundButtons;
    QList<KColorButton *> m_foregroundButtons;
    QList<KColorButton *> m_decorationButtons;

    QList<KColorScheme> m_colorSchemes;
};

#endif
