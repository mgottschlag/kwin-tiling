/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2009 by Marco Martin <notmart@gmail.com>
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
#include "appletoverlay.h"
#include "applettitlebar.h"

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
#include <Plasma/ScrollWidget>
#include <Plasma/PopupApplet>
#include <Plasma/Frame>

using namespace Plasma;


Newspaper::Newspaper(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_orientation(Qt::Vertical),
      m_appletOverlay(0),
      m_dragging(false)
{
    setContainmentType(Containment::CustomContainment);

    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));

    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
}

Newspaper::~Newspaper()
{
}

void Newspaper::init()
{
    m_externalLayout = new QGraphicsLinearLayout(this);
    m_externalLayout->setContentsMargins(0, 0, 0, 0);
    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_externalLayout->addItem(m_scrollWidget);
    m_mainWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_mainWidget);
    m_mainLayout = new QGraphicsLinearLayout(Qt::Horizontal, m_mainWidget);
    m_leftLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_rightLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainLayout->addItem(m_leftLayout);
    m_mainLayout->addItem(m_rightLayout);


    Plasma::Svg *borderSvg = new Plasma::Svg(this);
    borderSvg->setImagePath("newspaper/border");

    QGraphicsWidget *spacer1 = new QGraphicsWidget(m_mainWidget);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGraphicsWidget *spacer2 = new QGraphicsWidget(m_mainWidget);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_leftLayout->addItem(spacer1);
    m_rightLayout->addItem(spacer2);

    Containment::init();
    setHasConfigurationInterface(true);

    QAction *a = action("add widgets");
    if (a) {
        addToolBoxAction(a);
    }

    a = action("configure");
    if (a) {
        addToolBoxAction(a);
    }
}

void Newspaper::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    QGraphicsLinearLayout *lay;

    if (m_orientation == Qt::Horizontal) {
        int limitY;
        if (m_dragging) {
            limitY = m_rightLayout->geometry().height();
        } else {
            limitY = size().height()/4;
        }
        if (pos.x() >= limitY) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    } else {
        int limitX;
        //if we are initializing all high value x are to be considered secon coulmn, if we are dropping by hand, we drop on the second column when the value is > of the actual second column geometry
        if (m_dragging) {
            limitX = m_rightLayout->geometry().width();
        } else {
            limitX = size().width()/4;
        }
        if (pos.x() >= limitX) {
            lay = m_rightLayout;
        } else {
            lay = m_leftLayout;
        }
    }

    int insertIndex = -1;

    QPointF localPos = mapToItem(m_mainWidget, pos);

    //if localPos is (-1,-1) insert at the end of the Newspaper
    if (localPos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QGraphicsLayoutItem *li = lay->itemAt(i);
           /* if (!dynamic_cast<Plasma::Applet *>(li)) {
                continue;
            }*/

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

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    updateSize();
    createAppletTitle(applet);
}

void Newspaper::updateSize()
{
    QSizeF hint = m_mainWidget->effectiveSizeHint(Qt::PreferredSize);
    if (m_orientation == Qt::Horizontal) {
        const qreal proposedWidth = hint.width();
        hint.scale(QWIDGETSIZE_MAX, m_mainWidget->size().height(), Qt::KeepAspectRatio);
        hint.setWidth(qMax(proposedWidth, hint.width()));
        m_mainWidget->resize(hint);
    } else {
        const qreal proposedHeight = hint.height();
        hint.scale(m_mainWidget->size().width(), QWIDGETSIZE_MAX, Qt::KeepAspectRatio);
        hint.setHeight(qMax(hint.height(), proposedHeight));
        m_mainWidget->resize(hint);
    }
}

void Newspaper::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::SizeConstraint && m_appletOverlay) {
        m_appletOverlay->resize(size());
    }
}

void Newspaper::updateConfigurationMode(bool config)
{
    if (config && !m_appletOverlay) {
        m_appletOverlay = new AppletOverlay(this, this);
        m_appletOverlay->resize(size());
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
    }
}

void Newspaper::createAppletTitle(Plasma::Applet *applet)
{
    AppletTitleBar *appletTitleBar = new AppletTitleBar(applet);
    appletTitleBar->show();
}


K_EXPORT_PLASMA_APPLET(newspaper, Newspaper)

#include "newspaper.moc"

