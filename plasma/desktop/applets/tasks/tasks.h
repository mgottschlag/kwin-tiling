/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef TASKS_H
#define TASKS_H

// Own
#include "ui_tasksConfig.h"

// Qt
#include <QWeakPointer>
#include <QTimer>
#include <QSize>

// KDE
#include <taskmanager/taskmanager.h>
#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/startup.h>

// Plasma
#include <Plasma/Applet>

class QGraphicsLinearLayout;


namespace Plasma
{
    class LayoutAnimator;
    class FrameSvg;
} // namespace Plasma

namespace TaskManager
{
    class GroupManager;
} // namespace TaskManager

class TaskGroupItem;
class GroupManager;

/**
 * An applet which provides a visual representation of running
 * graphical tasks (ie. tasks that have some form of visual interface),
 * and allows the user to perform various actions on those tasks such
 * as bringing them to the foreground, sending them to the background
 * or closing them.
 */
class Tasks : public Plasma::Applet
{
    Q_OBJECT
public:
        /**
         * Constructs a new tasks applet
         * With the specified parent.
         */
        explicit Tasks(QObject *parent, const QVariantList &args = QVariantList());
        ~Tasks();

        void init();

        void constraintsEvent(Plasma::Constraints constraints);

        Plasma::FrameSvg *itemBackground();

        qreal itemLeftMargin() { return m_leftMargin; }
        qreal itemRightMargin() { return m_rightMargin; }
        qreal itemTopMargin() { return m_topMargin; }
        qreal itemBottomMargin() { return m_bottomMargin; }
        qreal offscreenLeftMargin() { return m_offscreenLeftMargin; }
        qreal offscreenRightMargin() { return m_offscreenRightMargin; }
        qreal offscreenTopMargin() { return m_offscreenTopMargin; }
        qreal offscreenBottomMargin() { return m_offscreenBottomMargin; }
        void resizeItemBackground(const QSizeF &newSize);

        TaskGroupItem *rootGroupItem();
        TaskManager::GroupManager &groupManager() const;

        Qt::KeyboardModifiers groupModifierKey() const;

        bool showToolTip() const;
        bool highlightWindows() const;

        void needsVisualFocus(bool focus);
        QWidget *popupDialog() const;

        bool isPopupShowing() const;

signals:
        /**
         * emitted whenever we receive a constraintsEvent
         */
        void constraintsChanged(Plasma::Constraints);
        void settingsChanged();

public slots:
        void configChanged();
        void publishIconGeometry();

protected slots:
        void configAccepted();
        void setPopupDialog(bool status);

protected:
        void createConfigurationInterface(KConfigDialog *parent);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
        void adjustGroupingStrategy();

private slots:
        /**
        * Somthing has changed in the tree of the GroupingStrategy
        */
        void reload();
        void changeSizeHint(Qt::SizeHint which);
        void dialogGroupingChanged(int index);

private:
        bool m_showTooltip;
        bool m_highlightWindows;
        Plasma::LayoutAnimator *m_animator;
        QGraphicsLinearLayout *layout;

        Ui::tasksConfig m_ui;
        QTimer m_screenTimer;

        Plasma::FrameSvg *m_taskItemBackground;
        qreal m_leftMargin;
        qreal m_topMargin;
        qreal m_rightMargin;
        qreal m_bottomMargin;
        qreal m_offscreenLeftMargin;
        qreal m_offscreenTopMargin;
        qreal m_offscreenRightMargin;
        qreal m_offscreenBottomMargin;

        TaskGroupItem *m_rootGroupItem;
        GroupManager *m_groupManager;
        TaskManager::GroupManager::TaskGroupingStrategy m_groupingStrategy;
        bool m_groupWhenFull;
        Qt::KeyboardModifier m_groupModifierKey;

        int m_currentDesktop;
        QWeakPointer<QWidget> m_popupDialog;
};

#endif
