/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef NETTOOLBOX_H
#define NETTOOLBOX_H

#include <QGraphicsWidget>

class QGraphicsLinearLayout;

namespace Plasma
{
    class Containment;
    class Frame;
    class IconWidget;
};

class NebToolBox : public QGraphicsWidget
{
    Q_OBJECT
public:
    NebToolBox(Plasma::Containment *parent = 0);
    ~NebToolBox();

    bool showing() const;
    void setShowing(const bool show);

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate hte tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);

Q_SIGNALS:
    void toggled();
    void visibilityChanged(bool);

private:
    Plasma::Frame *m_toolContainer;
    QGraphicsLinearLayout *m_toolContainerLayout;
    QHash<QAction *, Plasma::IconWidget *> m_actionButtons;
};

#endif
