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

#include <plasma/corona.h>
#include <plasma/panelsvg.h>
#include <plasma/theme.h>
#include <plasma/view.h>

using namespace Plasma;

static const int CONTROL_BAR_HEIGHT = 22;

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_configureAction(0)
{
    m_background = new Plasma::PanelSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    setContainmentType(Containment::PanelContainment);

    QSize size = QSize(args.count() ? args[0].toInt() : 1024, 22);
    kDebug() << "**********" << size;
    resize(size);
    setMinimumSize(size);
    setMaximumSize(size);
    setDrawWallpaper(false);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
}

Panel::~Panel()
{
}

void Panel::init()
{
    Containment::init();
}

QList<QAction*> Panel::contextualActions()
{
    if (!m_configureAction) {
        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(toolBoxToggled()));
    }

    QList<QAction*> actions;
    actions << action("add widgets") << m_configureAction;
    return actions;
}

void Panel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    // this gets called whenever an applet is added, and we add it to our layout
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //Enlarge the panel if possible
    if (f == Plasma::Horizontal) {
        resize(size().width() + applet->preferredWidth(), size().height());
    } else {
        resize(size().width(), size().height() + applet->preferredHeight());
    }
    layout()->setMaximumSize(size());

    //if pos is (-1,-1) insert at the end of the panel
    if (pos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QRectF siblingGeometry = lay->itemAt(i)->geometry();
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
        lay->addItem(applet);
    } else {
        lay->insertItem(insertIndex, applet);
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

void Panel::updateBorders(const QRect &geom)
{
    Plasma::Location loc = location();
    PanelSvg::EnabledBorders enabledBorders = PanelSvg::AllBorders;

    kDebug() << "!!!!!!!!!!!!!!!! location be:" << location();
    switch (location()) {
        case BottomEdge:
            enabledBorders = PanelSvg::TopBorder;
            break;
        case TopEdge:
            enabledBorders = PanelSvg::BottomBorder;
            break;
        case LeftEdge:
            enabledBorders = PanelSvg::RightBorder;
            break;
        case RightEdge:
            enabledBorders = PanelSvg::LeftBorder;
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

        // create our layout!
        if (form == Plasma::Horizontal || Plasma::Vertical) {
            if (layout()) {
                QGraphicsLayout *lay = layout();
                QGraphicsLinearLayout * linearLay = dynamic_cast<QGraphicsLinearLayout *>(lay);
                if (linearLay) {
                    linearLay->setOrientation(form == Plasma::Horizontal ? Qt::Horizontal :
                                                                           Qt::Vertical);
                }
            } else {
                QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
                lay->setOrientation(form == Plasma::Horizontal ? Qt::Horizontal : Qt::Vertical);
                lay->setContentsMargins(0, 0, 0, 0);
                lay->setSpacing(4);
                lay->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
                setLayout(lay);
                updateBorders(geometry().toRect());

                foreach (Applet *applet, applets()) {
                    lay->addItem(applet);
                }
            }
        }
    }

    //we need to know if the width or height is 100%
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        QSize size = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? QApplication::desktop()->screenGeometry(screen()) : geometry();

        if ((formFactor() == Horizontal && size.width() >= screenRect.width()) ||
            (formFactor() == Vertical && size.height() >= screenRect.height())) {
            m_background->setElementPrefix(location());
        } else {
            m_background->setElementPrefix(QString());
        }

        m_background->resizePanel(size);
        updateBorders(geometry().toRect());
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
        bool unlocked = immutability() == Plasma::Mutable;

        if (m_configureAction) {
            m_configureAction->setEnabled(unlocked);
            m_configureAction->setVisible(unlocked);
        }

        QGraphicsView *panelView = view();
        if (panelView) {
            updateBorders(panelView->geometry());
        }
    }
}

void Panel::saveState(KConfigGroup &config) const
{
    config.writeEntry("minimumSize", minimumSize());
    config.writeEntry("maximumSize", maximumSize());
}

void Panel::themeUpdated()
{
    //if the theme is changed all the calculations needs to be done again
    //and resize based on the change in the theme bordersize

    qreal oldLeftWidth;
    qreal newLeftWidth;
    qreal oldTopHeight;
    qreal newTopHeight;
    qreal oldRightWidth;
    qreal newRightWidth;
    qreal oldBottomHeight;
    qreal newBottomHeight;

    layout()->getContentsMargins(&oldLeftWidth, &oldTopHeight, &oldRightWidth, &oldBottomHeight);
    m_background->getMargins(newLeftWidth, newTopHeight, newRightWidth, newBottomHeight);

    QSize newSize(size().width()-(oldLeftWidth - newLeftWidth)-(oldRightWidth - newRightWidth),
           size().height()-(oldTopHeight - newTopHeight)-(oldBottomHeight - newBottomHeight));

    resize(newSize);

    if (formFactor() == Plasma::Vertical) {
        setMaximumWidth(newSize.width());
        setMinimumWidth(newSize.width());
    } else {
        setMaximumHeight(newSize.height());
        setMinimumHeight(newSize.height());
    }

    updateBorders(geometry().toRect());
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

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintPanel(painter, option->exposedRect, option->exposedRect);

    if (containmentOpt && containmentOpt->view) {
        containmentOpt->view->setMask(m_background->mask());
    }

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

K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

