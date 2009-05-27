/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2009 vy Marco Martin <notmart@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#include "newspaper.h"

#include <limits>

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>
#include <QGraphicsLayout>


#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/View>

using namespace Plasma;


Newspaper::Newspaper(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_orientation(Qt::Vertical)
{
    setContainmentType(Containment::CustomContainment);

    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
}

Newspaper::~Newspaper()
{
}

void Newspaper::init()
{
    m_mainLayout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    m_leftLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_rightLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainLayout->addItem(m_leftLayout);
    m_mainLayout->addItem(m_rightLayout);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/background");

    Containment::init();
    setHasConfigurationInterface(true);
}


void Newspaper::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    QGraphicsLinearLayout *lay;

    if (m_orientation == Qt::Horizontal) {
        if (pos.y() >= size().height()/4) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    } else {
        if (pos.x() >= size().width()/4) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    }

    int insertIndex = -1;

    //if pos is (-1,-1) insert at the end of the Newspaper
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QRectF siblingGeometry = lay->itemAt(i)->geometry();
            if (m_orientation == Qt::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { //Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
                if (pos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }

    if (insertIndex == -1) {
        lay->addItem(applet);
    } else {
        lay->insertItem(insertIndex, applet);
    }

    applet->setBackgroundHints(NoBackground);
}


void Newspaper::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint) {

        foreach (Applet *applet, applets()) {
            applet->setBackgroundHints(NoBackground);
        }
    }
}




void Newspaper::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)

    Containment::paintInterface(painter, option, contentsRect);

    m_background->resizeFrame(contentsRect.size());
    m_background->paintFrame(painter, contentsRect.topLeft());
}



K_EXPORT_PLASMA_APPLET(newspaper, Newspaper)

#include "newspaper.moc"

