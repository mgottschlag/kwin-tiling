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
#include "../common/nettoolbox.h"

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


#include <KAction>
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

    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(updateSize()));

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
    spacer1->setPreferredHeight(0);
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGraphicsWidget *spacer2 = new QGraphicsWidget(m_mainWidget);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    spacer2->setPreferredHeight(0);
    m_leftLayout->addItem(spacer1);
    m_rightLayout->addItem(spacer2);

    Containment::init();
    setHasConfigurationInterface(true);

    m_toolBox = new NetToolBox(this);
    setToolBox(m_toolBox);
    connect(m_toolBox, SIGNAL(toggled()), this, SIGNAL(toolBoxToggled()));
    connect(m_toolBox, SIGNAL(visibilityChanged(bool)), this, SIGNAL(toolBoxVisibilityChanged(bool)));
    m_toolBox->show();

    QAction *a = action("add widgets");
    if (a) {
        m_toolBox->addTool(a);
    }

    a = new QAction(KIcon("view-pim-news"), i18n("Add page"), this);
    addAction("add page", a);
    m_toolBox->addTool(a);
    connect(a, SIGNAL(triggered()), this, SLOT(addNewsPaper()));

    a = action("configure");
    if (a) {
        a->setText(i18n("Configure page"));
        m_toolBox->addTool(a);
    }

    KAction *lockAction = new KAction(this);
    addAction("lock page", lockAction);
    lockAction->setText(i18n("Lock page"));
    lockAction->setIcon(KIcon("object-locked"));
    QObject::connect(lockAction, SIGNAL(triggered(bool)), this, SLOT(toggleImmutability()));
    m_toolBox->addTool(lockAction);



    a = action("remove");
    if (a) {
        a->setText(i18n("Remove page"));
        m_toolBox->addTool(a);
    }
}

void Newspaper::toggleImmutability()
{
    if (immutability() == Plasma::UserImmutable) {
        setImmutability(Plasma::Mutable);
    } else if (immutability() == Plasma::Mutable) {
        setImmutability(Plasma::UserImmutable);
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
        m_mainWidget->resize(hint.width(), m_mainWidget->size().height());
    } else {
        m_mainWidget->resize(m_mainWidget->size().width(), hint.height());
    }
}

void Newspaper::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::SizeConstraint && m_appletOverlay) {
        m_appletOverlay->resize(size());
    }

    if (constraints & Plasma::ImmutableConstraint) {
        QAction *a = action("lock page");
        if (a) {
            switch (immutability()) {
                case Plasma::SystemImmutable:
                    a->setEnabled(false);
                    a->setVisible(false);
                    break;

                case Plasma::UserImmutable:
                    a->setText(i18n("Unlock Page"));
                    a->setIcon(KIcon("object-unlocked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;

                case Plasma::Mutable:
                    a->setText(i18n("Lock Page"));
                    a->setIcon(KIcon("object-locked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;
            }
        }

        a = action("add page");
        if (a) {
            if (immutability() == Plasma::Mutable) {
                a->setEnabled(true);
                a->setVisible(true);
            } else {
                a->setEnabled(false);
                a->setVisible(false);
            }
        }

        if (immutability() == Plasma::Mutable && !m_appletOverlay && m_toolBox->isShowing()) {
            m_appletOverlay = new AppletOverlay(this, this);
            m_appletOverlay->resize(size());
        } else if (immutability() != Plasma::Mutable && m_appletOverlay && m_toolBox->isShowing()) {
            m_appletOverlay->deleteLater();
            m_appletOverlay = 0;
        }
    }
}

void Newspaper::updateConfigurationMode(bool config)
{
    qreal extraLeft = 0;
    qreal extraTop = 0;
    qreal extraRight = 0;
    qreal extraBottom = 0;

    switch (m_toolBox->location()) {
        case Plasma::LeftEdge:
        extraLeft= m_toolBox->expandedGeometry().width();
        break;
    case Plasma::RightEdge:
        extraRight = m_toolBox->expandedGeometry().width();
        break;
    case Plasma::TopEdge:
        extraTop = m_toolBox->expandedGeometry().height();
        break;
    case Plasma::BottomEdge:
    default:
        extraBottom = m_toolBox->expandedGeometry().height();
    }

    if (config && !m_appletOverlay && immutability() == Plasma::Mutable) {
        m_appletOverlay = new AppletOverlay(this, this);
        m_appletOverlay->resize(size());

        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setContentsMargins(left + extraLeft, top + extraTop, right + extraRight, bottom + extraBottom);
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;

        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        setContentsMargins(left - extraLeft, top - extraTop, right - extraRight, bottom - extraBottom);
    }
}

void Newspaper::createAppletTitle(Plasma::Applet *applet)
{
    AppletTitleBar *appletTitleBar = new AppletTitleBar(applet);
    appletTitleBar->show();
}

void Newspaper::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ContentsRectChange && !m_toolBox->isShowing()) {
        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        //left preferred over right
        if (left > top && left > right && left > bottom) {
            m_toolBox->setLocation(Plasma::RightEdge);
        } else if (right > top && right >= left && right > bottom) {
            m_toolBox->setLocation(Plasma::LeftEdge);
        } else if (bottom > top && bottom > left && bottom > right) {
            m_toolBox->setLocation(Plasma::TopEdge);
        //bottom is the default
        } else {
            m_toolBox->setLocation(Plasma::BottomEdge);
        }
    }
}

void Newspaper::addNewsPaper()
{
    Plasma::Corona *c = corona();
    if (!c) {
        return;
    }

    Plasma::Containment *cont = c->addContainment("newspaper");
    cont->setScreen(0);
    cont->setToolBoxOpen(true);
}

void Newspaper::restore(KConfigGroup &group)
{
    Containment::restore(group);

    KConfigGroup appletsConfig(&group, "Applets");

    //FIXME: generic number of columns
    QMap<int, Applet *> oderedAppletsLeft;
    QMap<int, Applet *> oderedAppletsRight;
    QList<Applet *> unoderedApplets;

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

        int column = layoutConfig.readEntry("Column", 0);
        int order = layoutConfig.readEntry("Order", -1);

        if (order > -1) {
            if (column == 0) {
                oderedAppletsLeft[order] = applet;
            } else if (column == 1) {
                oderedAppletsRight[order] = applet;
            } else {
                unoderedApplets.append(applet);
            }
        //if LayoutInformation is not available use the usual way, as a bonus makes it retrocompatible with oler configs
        } else {
            unoderedApplets.append(applet);
        }

        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    }

    foreach (Applet *applet, oderedAppletsLeft) {
        m_leftLayout->insertItem(m_leftLayout->count()-1, applet);
        createAppletTitle(applet);
    }
    foreach (Applet *applet, oderedAppletsRight) {
        m_rightLayout->insertItem(m_rightLayout->count()-1, applet);
        createAppletTitle(applet);
    }

    foreach (Applet *applet, unoderedApplets) {
        layoutApplet(applet, applet->pos());
    }

    updateSize();

    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
}

void Newspaper::saveContents(KConfigGroup &group) const
{
    Containment::saveContents(group);

    KConfigGroup appletsConfig(&group, "Applets");
    for (int column = 0; column < m_mainLayout->count(); ++column) {
        QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(column));
        for (int row = 0; row < lay->count(); ++row) {
            const Applet *applet = dynamic_cast<Applet *>(lay->itemAt(row));
            if (applet) {
                KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
                KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

                layoutConfig.writeEntry("Column", column);
                layoutConfig.writeEntry("Order", row);
            }
        }
    }
}


K_EXPORT_PLASMA_APPLET(newspaper, Newspaper)

#include "newspaper.moc"

