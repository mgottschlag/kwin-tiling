/*
 *   Copyright (C) 2005 by Matt Williams <matt@milliams.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CONTROL_BOX_H
#define CONTROL_BOX_H

#include <QtGui/QWidget>

class QLabel;
class QTimeLine;
class DisplayLabel;
class ControlWidget;
class QListView;
class QStringListModel;
class QModelIndex;

/**
 * @short The Desktop configuration widget
 */
class ControlBox : public QWidget
{
    Q_OBJECT

    public:
        ControlBox(QWidget* parent);
        ~ControlBox();
        bool eventFilter(QObject *watched, QEvent *event);

    Q_SIGNALS:
        void boxRequested();
        void addPlasmoid(const QString&);

    protected:
        void mousePressEvent (QMouseEvent* event);
        void setupBox(); ///<Create contents of the config dialog

    protected Q_SLOTS:
        void showBox(); ///<Show the config widget
        void hideBox(); ///<Hide the config widget
        void animateBox(int frame); ///<Process the frames to create an animation

    private:
        ControlWidget* m_box; ///<The configuraion dialog widget
        DisplayLabel* m_displayLabel; ///<The 'show config' button
        QTimeLine* m_timeLine;
        QTimer* m_exitTimer;

        bool boxIsShown;
};

/**
 * @short The widget that contains the actual settings
 */
class ControlWidget : public QWidget
{
    Q_OBJECT

    public:
        ControlWidget(QWidget* parent);
        ~ControlWidget();

    protected:
        void refreshPlasmiodList();

        QLabel* m_label;
        QListView* appletList;
        QStringListModel* appletListModel;

    protected Q_SLOTS:
        void addPlasmoidSlot(const QModelIndex& plasmoidIndex);

    Q_SIGNALS:
        void addPlasmoid(const QString&);

    private:
};

#endif // multiple inclusion guard
