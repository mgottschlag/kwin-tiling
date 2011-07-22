/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef THEMEPAGE_H
#define THEMEPAGE_H

#include <QWidget>
#include <QModelIndex>

#include "ui_themepage.h"

class CursorThemeModel;
class SortProxyModel;
class CursorTheme;

class ThemePage : public QWidget, private Ui::ThemePage
{
    Q_OBJECT

    public:
        ThemePage(QWidget* parent = 0);
        ~ThemePage();

        // Called by the KCM
        void save();
        void load();
        void defaults();

        static bool haveXfixes();

    signals:
        void changed(bool);

    private slots:
        void selectionChanged();
        void getNewClicked();
        void installClicked();
        void removeClicked();

    private:
        void selectRow(int) const;
        void selectRow(const QModelIndex &index) const { selectRow(index.row()); }
        QModelIndex selectedIndex() const;
        bool installThemes(const QString &file);
        bool applyTheme(const CursorTheme *theme);
        bool iconsIsWritable() const;

        CursorThemeModel *model;
        SortProxyModel *proxy;

        // This index refers to the CursorThemeModel, not the proxy or the view
        QPersistentModelIndex appliedIndex;
};

#endif // THEMEPAGE_H
