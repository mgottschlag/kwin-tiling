/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
*   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
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

#include "panel.h"
#include "dummytoolbox.h"
#include "../common/linearappletoverlay.h"

#include <limits>

#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QDesktopWidget>
#include <QAction>
#include <QGraphicsLayout>


#include <KAction>
#include <KDebug>
#include <KIcon>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/View>

using namespace Plasma;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_layout(0),
      m_appletOverlay(0)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);

    QSize size = QSize(args.count() ? args[0].toInt() : 1024, 22);
    kDebug() << "**********" << size;
    resize(size);
    setMinimumSize(size);
    setMaximumSize(size);
    setDrawWallpaper(false);

    DummyToolBox *toolBox = new DummyToolBox(this);
    setToolBox(toolBox);

    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
    connect(this, SIGNAL(toolBoxVisibilityChanged(bool)),
            this, SLOT(updateConfigurationMode(bool)));
}

Panel::~Panel()
{
}

void Panel::init()
{
    Containment::init();

    Plasma::Corona *c = corona();
    connect(c, SIGNAL(containmentAdded(Plasma::Containment*)),
            this, SLOT(containmentAdded(Plasma::Containment*)));

    KAction *lockAction = new KAction(this);
    addAction("lock panel", lockAction);
    lockAction->setText(i18n("Lock Panel"));
    lockAction->setIcon(KIcon("object-locked"));
    QObject::connect(lockAction, SIGNAL(triggered(bool)), this, SLOT(toggleImmutability()));
    lockAction->setShortcut(KShortcut("alt+d, l"));
    lockAction->setShortcutContext(Qt::ApplicationShortcut);
}

void Panel::toggleImmutability()
{
    if (immutability() == Plasma::UserImmutable) {
        setImmutability(Plasma::Mutable);
    } else if (immutability() == Plasma::Mutable) {
        setImmutability(Plasma::UserImmutable);
    }
}

void Panel::containmentAdded(Plasma::Containment *containment)
{
    connect(containment, SIGNAL(toolBoxVisibilityChanged(bool)),
            this, SLOT(updateConfigurationMode(bool)));
}

void Panel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    // this gets called whenever an applet is added, and we add it to our m_layoutout

    if (!m_layout) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < m_layout->count(); ++i) {
            QRectF siblingGeometry = m_layout->itemAt(i)->geometry();
            if (f == Plasma::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (pos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (pos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { // Plasma::Vertical
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
        m_layout->addItem(applet);
    } else {
        m_layout->insertItem(insertIndex, applet);
    }

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
}

void Panel::appletRemoved(Plasma::Applet* applet)
{
    //shrink the panel if possible
    if (formFactor() == Plasma::Horizontal) {
        resize(size().width() - applet->size().width(), size().height());
    } else {
        resize(size().width(), size().height() - applet->size().height());
    }
    layout()->setMaximumSize(size());
}

void Panel::updateSize()
{
    Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(sender());

    if (applet) {
        if (formFactor() == Plasma::Horizontal) {
            const int delta = applet->preferredWidth() - applet->size().width();
            //setting the preferred width when delta = 0 and preferredWidth() < minimumWidth()
            // leads to the same thing as setPreferredWidth(minimumWidth())
            if (delta != 0) {
                setPreferredWidth(preferredWidth() + delta);
            }
        } else if (formFactor() == Plasma::Vertical) {
            const int delta = applet->preferredHeight() - applet->size().height();
            if (delta != 0) {
                setPreferredHeight(preferredHeight() + delta);
            }
        }

        resize(preferredSize());
    }
}

void Panel::updateBorders()
{
    FrameSvg::EnabledBorders enabledBorders = FrameSvg::AllBorders;

    kDebug() << "!!!!!!!!!!!!!!!! location be:" << location();
    switch (location()) {
        case BottomEdge:
            enabledBorders = FrameSvg::TopBorder;
            break;
        case TopEdge:
            enabledBorders = FrameSvg::BottomBorder;
            break;
        case LeftEdge:
            enabledBorders = FrameSvg::RightBorder;
            break;
        case RightEdge:
            enabledBorders = FrameSvg::LeftBorder;
            break;
        default:
            break;
    }

    qreal topHeight = 0;
    qreal bottomHeight = 0;
    qreal leftWidth = 0;
    qreal rightWidth = 0;

    //activate borders and fetch sizes again
    m_background->setEnabledBorders(enabledBorders);
    m_background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    //FIXME: this will be probably just killed not kept in this zombie state :)
#if 0
    //calculation of extra margins has to be done after getMargins
    if (formFactor() == Vertical) {
        //hardcoded extra margin for the toolbox right now
        if (immutability() == Mutable) {
            bottomHeight += 20;
        }
    //Default to horizontal for now
    } else {
        //hardcoded extra margin for the toolbox for now
        if (immutability() == Mutable) {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                leftWidth += 20;
            } else {
                rightWidth += 20;
            }
        }
    }
#endif

    switch (location()) {
    case LeftEdge:
        rightWidth = qMin(rightWidth, qMax(qreal(1), size().width() - KIconLoader::SizeMedium));
        break;
    case RightEdge:
        leftWidth = qMin(leftWidth, qMax(qreal(1), size().width() - KIconLoader::SizeMedium));
        break;
    case TopEdge:
        bottomHeight = qMin(bottomHeight, qMax(qreal(1), size().height() - KIconLoader::SizeMedium));
        break;
    case BottomEdge:
        topHeight = qMin(topHeight, qMax(qreal(1), size().height() - KIconLoader::SizeMedium));
        break;
    default:
        break;
    }

    //invalidate the layout and set again
    if (layout()) {
        layout()->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
        layout()->invalidate();
    }

    update();
}

void Panel::constraintsEvent(Plasma::Constraints constraints)
{
    kDebug() << "constraints updated with" << constraints << "!!!!!!";

    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal;

        // create our layout!
        if (layout()) {
            QGraphicsLayout *lay = layout();
            QGraphicsLinearLayout * linearLay = dynamic_cast<QGraphicsLinearLayout *>(lay);
            if (linearLay) {
                linearLay->setOrientation(layoutDirection);
            }
        } else {
            m_layout = new QGraphicsLinearLayout(this);
            m_layout->setOrientation(layoutDirection);
            m_layout->setContentsMargins(0, 0, 0, 0);
            m_layout->setSpacing(4);
            m_layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            setLayout(m_layout);
            updateBorders();

            foreach (Applet *applet, applets()) {
                m_layout->addItem(applet);
            }
        }
    }

    //we need to know if the width or height is 100%
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        QSize size = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? QApplication::desktop()->screenGeometry(screen()) : geometry();

        if (formFactor() == Horizontal ||
            formFactor() == Vertical) {
            m_background->setElementPrefix(location());
        } else {
            m_background->setElementPrefix(QString());
        }

        m_background->resizeFrame(size);
        updateBorders();
    }

    if (constraints & Plasma::SizeConstraint && m_appletOverlay) {
        m_appletOverlay->resize(size());
    }

    //FIXME: this seems the only way to correctly resize the layout the first time when the
    // saved panel size is less than the default is to setting a maximum size.
    // this shouldn't happen. maybe even a qgraphicslayout bug?
    if (layout() && (constraints & Plasma::SizeConstraint)) {
        layout()->setMaximumSize(size());
    }

    if (constraints & Plasma::LocationConstraint) {
        setFormFactorFromLocation(location());
    }

    if (constraints & Plasma::ImmutableConstraint) {
        updateBorders();

        QAction *a = action("lock panel");
        if (a) {
            switch (immutability()) {
                case Plasma::SystemImmutable:
                    a->setEnabled(false);
                    a->setVisible(false);
                    break;

                case Plasma::UserImmutable:
                    a->setText(i18n("Unlock Panel"));
                    a->setIcon(KIcon("object-unlocked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;

                case Plasma::Mutable:
                    a->setText(i18n("Lock Panel"));
                    a->setIcon(KIcon("object-locked"));
                    a->setEnabled(true);
                    a->setVisible(true);
                    break;
            }
        }
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
                this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
        delete action("remove");
    }
}

QList<QAction *> Panel::contextualActions()
{
    QList<QAction *> actions;

    QAction *a = action("lock panel");
    if (a) {
        actions << a;
    }

    return actions;
}

void Panel::updateConfigurationMode(bool config)
{
    if (config && !m_appletOverlay && immutability() == Plasma::Mutable) {
        m_appletOverlay = new LinearAppletOverlay(this, m_layout);
        m_appletOverlay->resize(size());
        connect (m_appletOverlay, SIGNAL(dropRequested(QGraphicsSceneDragDropEvent*)),
                 this, SLOT(overlayRequestedDrop(QGraphicsSceneDragDropEvent*)));
    } else if (!config) {
        delete m_appletOverlay;
        m_appletOverlay = 0;
    }
}

void Panel::overlayRequestedDrop(QGraphicsSceneDragDropEvent *event)
{
    dropEvent(event);
}

//TODO: fold it into saveContents even if it's not strictly contents related?
void Panel::saveState(KConfigGroup &config) const
{
    config.writeEntry("minimumSize", minimumSize());
    config.writeEntry("maximumSize", maximumSize());
}

void Panel::paintInterface(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           const QRect& contentsRect)
{
    Q_UNUSED(contentsRect)

    //FIXME: this background drawing is bad and ugly =)
    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    const Containment::StyleOption *containmentOpt = qstyleoption_cast<const Containment::StyleOption *>(option);

    QRect viewGeom;
    if (containmentOpt) {
        viewGeom = containmentOpt->view->geometry();
    }

    painter->fillRect(option->exposedRect, Qt::transparent);
    m_background->paintFrame(painter, option->exposedRect, option->exposedRect);


    // restore transformation and composition mode
    painter->restore();
}

void Panel::setFormFactorFromLocation(Plasma::Location loc) {
    switch (loc) {
        case BottomEdge:
        case TopEdge:
            //kDebug() << "setting horizontal form factor";
            setFormFactor(Plasma::Horizontal);
            break;
        case RightEdge:
        case LeftEdge:
            //kDebug() << "setting vertical form factor";
            setFormFactor(Plasma::Vertical);
            break;
        case Floating:
            //TODO: implement a form factor for floating panels
            kDebug() << "Floating is unimplemented.";
            break;
        default:
            kDebug() << "invalid location!!";
    }
}

void Panel::restore(KConfigGroup &group)
{
    Containment::restore(group);

    KConfigGroup appletsConfig(&group, "Applets");

    QMap<int, Applet *> oderedApplets;
    QList<Applet *> unoderedApplets;

    foreach (Applet *applet, applets()) {
        KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
        KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

        int order = layoutConfig.readEntry("Order", -1);

        if (order > -1) {
            oderedApplets[order] = applet;
        //if LayoutInformation is not available use the usual way, as a bonus makes it retrocompatible with oler configs
        } else {
            unoderedApplets.append(applet);
        }

        connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
    }

    foreach (Applet *applet, oderedApplets) {
        m_layout->addItem(applet);
    }

    foreach (Applet *applet, unoderedApplets) {
        layoutApplet(applet, applet->pos());
    }

    updateSize();
}

void Panel::saveContents(KConfigGroup &group) const
{
    Containment::saveContents(group);

    if (!m_layout) {
        return;
    }

    KConfigGroup appletsConfig(&group, "Applets");
    for (int order = 0; order < m_layout->count(); ++order) {
        const Applet *applet = dynamic_cast<Applet *>(m_layout->itemAt(order));
        if (applet) {
            KConfigGroup appletConfig(&appletsConfig, QString::number(applet->id()));
            KConfigGroup layoutConfig(&appletConfig, "LayoutInformation");

            layoutConfig.writeEntry("Order", order);
        }
    }
}

K_EXPORT_PLASMA_APPLET(netpanel, Panel)

#include "panel.moc"

