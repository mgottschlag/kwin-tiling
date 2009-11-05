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
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QAction>
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <QGraphicsSceneWheelEvent>

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
#include <Plasma/ToolButton>

using namespace Plasma;


Newspaper::Newspaper(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_orientation(Qt::Vertical),
      m_appletOverlay(0),
      m_dragging(0),
      m_leftArrow(0),
      m_rightArrow(0)
{
    setContainmentType(Containment::CustomContainment);

    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(updateSize()));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(cleanupColumns()));

    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));

    m_updateSizeTimer = new QTimer(this);
    m_updateSizeTimer->setSingleShot(false);
    connect(m_updateSizeTimer, SIGNAL(timeout()), this, SLOT(updateSize()));
}

Newspaper::~Newspaper()
{
    config().writeEntry("orientation", (int)m_orientation);
}

void Newspaper::init()
{
    m_externalLayout = new QGraphicsLinearLayout(this);
    m_externalLayout->setContentsMargins(0, 0, 0, 0);
    m_scrollWidget = new Plasma::ScrollWidget(this);
    m_externalLayout->addItem(m_scrollWidget);
    m_mainWidget = new QGraphicsWidget(m_scrollWidget);
    m_scrollWidget->setWidget(m_mainWidget);

    m_orientation = (Qt::Orientation)config().readEntry("orientation", (int)Qt::Vertical);

    m_mainLayout = new QGraphicsLinearLayout(m_mainWidget);
    addColumn();
    setOrientation(m_orientation);


    Plasma::Svg *borderSvg = new Plasma::Svg(this);
    borderSvg->setImagePath("newspaper/border");

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

    a = new QAction(i18n("Next activity"), this);
    addAction("next containment", a);
    a = new QAction(i18n("Previous activity"), this);
    addAction("previous containment", a);
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

    QPointF localPos = mapToItem(m_mainWidget, pos);

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

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(appletSizeHintChanged()));
    updateSize();
    createAppletTitle(applet);
}

void Newspaper::cleanupColumns()
{
    //clean up all empty columns
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (!lay) {
            continue;
        }

        if (lay->count() == 1) {
            removeColumn(i);
        }
    }
}

void Newspaper::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    m_mainLayout->setOrientation(orientation==Qt::Vertical?Qt::Horizontal:Qt::Vertical);
    m_externalLayout->setOrientation(m_orientation);

    if (m_orientation == Qt::Vertical) {
        m_mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        if (m_leftArrow) {
            m_leftArrow->deleteLater();
            m_rightArrow->deleteLater();
        }
    } else {
        m_mainWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        if (!m_leftArrow) {
            m_leftArrow = new Plasma::ToolButton(this);
            m_leftArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            m_leftArrow->setPreferredWidth(KIconLoader::SizeMedium);
            m_leftArrow->setImage("widgets/arrows", "left-arrow");
            connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(goLeft()));
            connect(m_leftArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

            m_rightArrow = new Plasma::ToolButton(this);
            m_rightArrow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            m_rightArrow->setPreferredWidth(KIconLoader::SizeMedium);
            m_rightArrow->setImage("widgets/arrows", "right-arrow");
            connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(goRight()));
            connect(m_rightArrow, SIGNAL(pressed()), this, SLOT(scrollTimeout()));

            m_externalLayout->insertItem(0, m_leftArrow);
            m_externalLayout->addItem(m_rightArrow);

            m_scrollTimer = new QTimer(this);
            m_scrollTimer->setSingleShot(false);
            connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
        }
    }

    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (!lay) {
            continue;
        }

        lay->setOrientation(orientation);
    }
}

void Newspaper::goRight()
{
    QGraphicsSceneWheelEvent ev(QEvent::GraphicsSceneWheel);
    ev.setDelta(-120);
    scene()->sendEvent(m_scrollWidget, &ev);

    if (m_mainWidget->geometry().right()-2 <= m_scrollWidget->viewportGeometry().right() && sender() == m_rightArrow) {
        action("next containment")->trigger();
    }
}

void Newspaper::goLeft()
{
    QGraphicsSceneWheelEvent ev(QEvent::GraphicsSceneWheel);
    ev.setDelta(120);
    scene()->sendEvent(m_scrollWidget, &ev);

    if (m_mainWidget->geometry().left() >= -2 && sender() == m_leftArrow) {
        action("previous containment")->trigger();
    }
}

void Newspaper::scrollTimeout()
{
    if (!m_scrollTimer->isActive()) {
        m_scrollTimer->start(250);
    } else if (m_leftArrow->isDown()) {
        goLeft();
    } else if (m_rightArrow->isDown()) {
        goRight();
    } else {
        m_scrollTimer->stop();
    }
}

Qt::Orientation Newspaper::orientation() const
{
    return m_orientation;
}

void Newspaper::appletSizeHintChanged()
{
    if (m_updateSizeTimer) {
        m_updateSizeTimer->start(200);
    }
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

    if (constraints & Plasma::StartupCompletedConstraint) {
        connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
                this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
        Plasma::Corona *c = corona();
        if (c) {
            connect(c, SIGNAL(containmentAdded(Plasma::Containment *)),
                    this, SLOT(containmentAdded(Plasma::Containment *)));

            foreach (Plasma::Containment *containment, corona()->containments()) {
                Newspaper *news = qobject_cast<Newspaper *>(containment);
                if (news) {
                    connect(news, SIGNAL(destroyed(QObject *)),
                            this, SLOT(containmentRemoved(QObject *)));
                }
             }
             QTimer::singleShot(100, this, SLOT(updateRemoveActionVisibility()));
        }
    }

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

    if (config) {
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
    }

    if (config && !m_appletOverlay && immutability() == Plasma::Mutable) {
        m_appletOverlay = new AppletOverlay(this, this);
        m_appletOverlay->resize(size());
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
    }

    m_externalLayout->setContentsMargins(extraLeft, extraTop, extraRight, extraBottom);

    if (!config) {
        cleanupColumns();
    }

}

void Newspaper::createAppletTitle(Plasma::Applet *applet)
{
    AppletTitleBar *appletTitleBar = new AppletTitleBar(applet);
    appletTitleBar->show();
}

void Newspaper::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ContentsRectChange) {
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

        if (m_toolBox->isShowing()) {
            updateConfigurationMode(true);
        }
    }
}

void Newspaper::addNewsPaper()
{
    Plasma::Corona *c = corona();
    if (!c) {
        return;
    }

    //count the pages
    int numNewsPapers = 0;
    if (corona()) {
        foreach (Plasma::Containment *containment, corona()->containments()) {
            if (qobject_cast<Newspaper *>(containment)) {
                ++numNewsPapers;
            }
        }
    }

    Plasma::Containment *cont = c->addContainment("newspaper");
    cont->setActivity(i18nc("Page number", "Page %1", numNewsPapers+1));
    cont->setScreen(0);
    cont->setToolBoxOpen(true);
}

void Newspaper::restore(KConfigGroup &group)
{
    Containment::restore(group);

    KConfigGroup appletsConfig(&group, "Applets");

    //the number of items in orderedApplets is the number of columns
    QMap<int, QMap<int, Applet *> > orderedApplets;
    QList<Applet *> unorderedApplets;

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

        int column = layoutConfig.readEntry("Column", -1);
        int order = layoutConfig.readEntry("Order", -1);

        if (order > -1) {
            if (column > -1) {
                orderedApplets[column][order] = applet;
            } else {
                unorderedApplets.append(applet);
            }
        //if LayoutInformation is not available use the usual way, as a bonus makes it retrocompatible with oler configs
        } else {
            unorderedApplets.append(applet);
        }

        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(appletSizeHintChanged()));
    }

    //if the required columns does not exist, create them
    if (m_mainLayout->count() < orderedApplets.count()) {
        int columnsToAdd = orderedApplets.count()-m_mainLayout->count();
        for (int i = 0; i < columnsToAdd; ++i) {
            addColumn();
        }
    }

    //finally add all the applets that had a layout information
    int column = 0;
    QMap<int, QMap<int, Applet *> >::const_iterator it = orderedApplets.constBegin();

    while (it != orderedApplets.constEnd()) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(column));
        ++column;

        //this should never happen
        if (!lay) {
            ++it;
            continue;
        }

        foreach (Applet *applet, it.value()) {
            lay->insertItem(lay->count()-1, applet);
            createAppletTitle(applet);
        }
        ++it;
    }

    //add all the remaining applets
    foreach (Applet *applet, unorderedApplets) {
        layoutApplet(applet, applet->pos());
    }

    updateSize();
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

QGraphicsLinearLayout *Newspaper::addColumn()
{
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(m_orientation);
    m_mainLayout->addItem(lay);

    QGraphicsWidget *spacer = new QGraphicsWidget(m_mainWidget);
    spacer->setPreferredSize(0, 0);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lay->addItem(spacer);

    return lay;
}

void Newspaper::removeColumn(int column)
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

    delete lay;
}

void Newspaper::updateRemoveActionVisibility()
{
    int newspapers = 0;

    foreach (Plasma::Containment *containment, corona()->containments()) {
        if (qobject_cast<Newspaper *>(containment)) {
            ++newspapers;
        }
    }

    QAction *a = action("remove");
    if (a) {
        a->setEnabled(newspapers > 1);
        a->setVisible(newspapers > 1);
    }
}

void Newspaper::containmentAdded(Plasma::Containment *containment)
{
    //we now are sure there are at least two pages
    Newspaper *news = qobject_cast<Newspaper *>(containment);
    if (news) {
        connect(news, SIGNAL(destroyed(QObject *)), this, SLOT(containmentRemoved(QObject *)));
        QAction *a = action("remove");
        if (a) {
            a->setEnabled(true);
            a->setVisible(true);
        }
    }
}

void Newspaper::containmentRemoved(QObject *containment)
{
    if (!corona()) {
        return;
    }

    if (qobject_cast<Newspaper *>(containment)) {
        updateRemoveActionVisibility();
    }
}

K_EXPORT_PLASMA_APPLET(newspaper, Newspaper)

#include "newspaper.moc"

