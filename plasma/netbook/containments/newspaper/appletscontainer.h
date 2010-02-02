/*********************************************************************/
/*                                                                   */
/* Copyright 2007 by Alex Merry <alex.merry@kdemail.net>             */
/* Copyright 2008 by Alexis Ménard <darktears31@gmail.com>          */
/* Copyright 2008 by Aaron Seigo <aseigo@kde.org>                    */
/* Copyright 2009 by Marco Martin <notmart@gmail.com>                */
/* Copyright 2010 by Igor Oliveira <igor.oliveira@openbossa.org      */
/*                                                                   */
/* This program is free software; you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License       */
/* as published by the Free Software Foundation; either version 2    */
/* of the License, or (at your option) any later version.            */
/*                                                                   */
/* This program is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     */
/* GNU General Public License for more details.                      */
/*                                                                   */
/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software       */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA     */
/* 02110-1301, USA.                                                  */
/*********************************************************************/

#ifndef APPLETSCONTAINER_H
#define APPLETSCONTAINER_H

#include <QGraphicsWidget>

namespace Plasma
{
    class Applet;
}

class QGraphicsLinearLayout;

class AppletsContainer : public QGraphicsWidget
{
    Q_OBJECT
public:
    AppletsContainer(QGraphicsItem *item = 0);
    ~AppletsContainer();

    void syncColumnSizes();
    void createAppletTitle(Plasma::Applet *applet);

    QGraphicsLinearLayout *addColumn();
    void removeColumn(int column);

    void setOrientation(Qt::Orientation orientation);

    int count() const;
    QGraphicsLayoutItem *itemAt(int i);

public slots:
    void layoutApplet(Plasma::Applet *applet, const QPointF &post);
    void updateSize();
    void cleanupColumns();

Q_SIGNALS:
    void appletSizeHintChanged();

private:
    QGraphicsLinearLayout *m_mainLayout;
    Qt::Orientation m_orientation;
};

#endif
