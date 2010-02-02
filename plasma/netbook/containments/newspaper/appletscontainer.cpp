/////////////////////////////////////////////////////////////////////////
// appletscontainer.cpp                                                //
//                                                                     //
// Copyright 2007 by Alex Merry <alex.merry@kdemail.net>               //
// Copyright 2008 by Alexis Ménard <darktears31@gmail.com>            //
// Copyright 2008 by Aaron Seigo <aseigo@kde.org>                      //
// Copyright 2009 by Marco Martin <notmart@gmail.com>                  //
// Copyright 2010 by Igor Oliveira <igor.oliveira@>openbossa.org>      //
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "appletscontainer.h"
#include "applettitlebar.h"

#include <QGraphicsLinearLayout>
#include <QWidget>

#include <Plasma/Applet>

using namespace Plasma;

AppletsContainer::AppletsContainer(QGraphicsItem *item)
 : QGraphicsWidget(item),
   m_orientation(Qt::Vertical)
{
    m_mainLayout = new QGraphicsLinearLayout(this);
}

AppletsContainer::~AppletsContainer()
{
}

void AppletsContainer::syncColumnSizes()
{
    QSizeF viewportSize = parentWidget()->size();
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (m_orientation == Qt::Vertical) {
            lay->setMaximumWidth(viewportSize.width()/m_mainLayout->count());
            lay->setMaximumHeight(QWIDGETSIZE_MAX);
        } else {
            lay->setMaximumHeight(viewportSize.height()/m_mainLayout->count());
            lay->setMaximumWidth(QWIDGETSIZE_MAX);
        }
    }
}

void AppletsContainer::updateSize()
{
    QSizeF hint = effectiveSizeHint(Qt::PreferredSize);

    //FIXME: it appears to work only with hardcoded values
    if (m_orientation == Qt::Horizontal) {
        resize(qMax((int)hint.width(), 300), size().height());
    } else {
        resize(size().width(), qMax((int)hint.height(), 300));
    }
}


QGraphicsLinearLayout *AppletsContainer::addColumn()
{
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(m_orientation);
    m_mainLayout->addItem(lay);

    QGraphicsWidget *spacer = new QGraphicsWidget(this);
    spacer->setPreferredSize(0, 0);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lay->addItem(spacer);

    syncColumnSizes();

    return lay;
}

void AppletsContainer::cleanupColumns()
{
    //clean up all empty columns
    for (int i = 0; i < count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (!lay) {
            continue;
        }

        if (lay->count() == 1) {
            removeColumn(i);
        }
    }
}

void AppletsContainer::removeColumn(int column)
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(column));

    if (!lay) {
        return;
    }

    m_mainLayout->removeAt(column);

    for (int i = 0; i < lay->count(); ++i) {
        QGraphicsLayoutItem *item = lay->itemAt(i);
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(item);
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

        //find a new home for the applet
        if (applet) {
            layoutApplet(applet, applet->pos());
        //delete spacers
        } else if (widget) {
            widget->deleteLater();
        }
    }

    syncColumnSizes();

    delete lay;
}

void AppletsContainer::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    QGraphicsLinearLayout *lay = 0;

    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        //normally should never happen
        if (!candidateLay) {
            continue;
        }

        if (m_orientation == Qt::Horizontal) {
            if (pos.y() < candidateLay->geometry().bottom()) {
                lay = candidateLay;
                break;
            }
        //vertical
        } else {
            if (pos.x() < candidateLay->geometry().right()) {
                lay = candidateLay;
                break;
            }
        }
    }

    //couldn't decide: is the last column empty?
    if (!lay) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(m_mainLayout->count()-1));

        if (candidateLay && candidateLay->count() == 1) {
            lay = candidateLay;
        }
    }

    //give up, make a new column
    if (!lay) {
        lay = addColumn();
    }

    int insertIndex = -1;

    QPointF localPos = mapToItem(this, pos);

    //if localPos is (-1,-1) insert at the end of the Newspaper
    if (localPos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QGraphicsLayoutItem *li = lay->itemAt(i);

            QRectF siblingGeometry = li->geometry();
            if (m_orientation == Qt::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (localPos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (localPos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { //Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;

                if (localPos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (localPos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }


    if (insertIndex == -1) {
        lay->insertItem(lay->count()-1, applet);
    } else {
        lay->insertItem(qMin(insertIndex, lay->count()-1), applet);
    }

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SIGNAL(appletSizeHintChanged()));
    updateSize();
    createAppletTitle(applet);
    syncColumnSizes();
}

void AppletsContainer::createAppletTitle(Plasma::Applet *applet)
{
    AppletTitleBar *appletTitleBar = new AppletTitleBar(applet);
    appletTitleBar->show();
}

void AppletsContainer::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    m_mainLayout->setOrientation(orientation==Qt::Vertical?Qt::Horizontal:Qt::Vertical);
}

int AppletsContainer::count() const
{
    return m_mainLayout->count();
}

QGraphicsLayoutItem *AppletsContainer::itemAt(int i)
{
    return m_mainLayout->itemAt(i);
}
