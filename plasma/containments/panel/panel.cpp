/*
*   Copyright 2007 by Alex Merry <alex.merry@kdemail.net>
*   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QComboBox>
#include <QDesktopWidget>
#include <QGraphicsLinearLayout>
#include <QGridLayout>
#include <QGraphicsLayout>
#include <QGraphicsSceneDragDropEvent>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include <QPainter>
#include <QSignalMapper>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KIntNumInput>
#include <KMessageBox>

#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/View>
#include <Plasma/PaintUtils>

#include <kephal/screens.h>

using namespace Plasma;

class Spacer : public QGraphicsWidget
{
public:
    Spacer(QGraphicsWidget *parent)
         : QGraphicsWidget(parent)
    {
        setAcceptDrops(true);
    }

    ~Spacer()
    {}

    Panel *panel;

protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        event->setPos(mapToParent(event->pos()));
        panel->dropEvent(event);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        //TODO: make this a pretty gradient?
        painter->setRenderHint(QPainter::Antialiasing);
        QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(1, 1, -2, -2), 4);
        QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        c.setAlphaF(0.3);

        painter->fillPath(p, c);
    }
};

Panel::Panel(QObject *parent, const QVariantList &args)
    : Containment(parent, args),
      m_addPanelsMenu(0),
      m_configureAction(0),
      m_addPanelAction(0),
      m_currentSize(QSize(Kephal::ScreenUtils::screenSize(screen()).width(), 35)),
      m_maskDirty(true),
      m_spacerIndex(-1),
      m_spacer(0),
      m_lastSpace(0)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/panel-background");
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(backgroundChanged()));
    setZValue(150);
    resize(m_currentSize);
    setMinimumSize(m_currentSize);
    setMaximumSize(m_currentSize);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    connect(this, SIGNAL(appletAdded(Plasma::Applet*,QPointF)),
            this, SLOT(layoutApplet(Plasma::Applet*,QPointF)));
    connect(this, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));
}

Panel::~Panel()
{
    delete m_addPanelsMenu;
}

void Panel::init()
{
    setContainmentType(Containment::PanelContainment);

    Containment::init();
    //FIXME: This should be enabled, but in that case proxywidgets won't get rendered
    //setFlag(ItemClipsChildrenToShape, true);

    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);
    lay->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setLayout(lay);
    updateBorders(geometry().toRect());
    lay->setMaximumSize(size());

    KConfigGroup cg = config("Configuration");

    m_currentSize = cg.readEntry("minimumSize", m_currentSize);
    if (formFactor() == Plasma::Vertical) {
        m_currentSize.expandedTo(QSize(0, 35));
    } else {
        m_currentSize.expandedTo(QSize(35, 0));
    }

    setMinimumSize(cg.readEntry("minimumSize", m_currentSize));
    setMaximumSize(cg.readEntry("maximumSize", m_currentSize));
    setDrawWallpaper(false);

    m_lastSpaceTimer = new QTimer(this);
    m_lastSpaceTimer->setSingleShot(true);
    connect(m_lastSpaceTimer, SIGNAL(timeout()), this, SLOT(adjustLastSpace()));
}

QList<QAction*> Panel::contextualActions()
{
    if (!m_configureAction) {
        m_configureAction = new QAction(i18n("Panel Settings"), this);
        m_configureAction->setIcon(KIcon("configure"));
        connect(m_configureAction, SIGNAL(triggered()), this, SIGNAL(toolBoxToggled()));

        KPluginInfo::List panelPlugins = listContainmentsOfType("panel");

        if (panelPlugins.size() == 1) {
            m_addPanelAction = new QAction(i18n("Add Panel"), this);
            connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
        } else if (!panelPlugins.isEmpty()) {
            m_addPanelsMenu = new QMenu();
            m_addPanelAction = m_addPanelsMenu->menuAction();
            m_addPanelAction->setText(i18n("Add Panel"));

            QSignalMapper *mapper = new QSignalMapper(this);
            connect(mapper, SIGNAL(mapped(QString)), this, SLOT(addPanel(QString)));

            foreach (const KPluginInfo &plugin, panelPlugins) {
                QAction *action = new QAction(plugin.name(), this);
                if (!plugin.icon().isEmpty()) {
                    action->setIcon(KIcon(plugin.icon()));
                }

                mapper->setMapping(action, plugin.pluginName());
                connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
                m_addPanelsMenu->addAction(action);
            }
        }

        if (m_addPanelAction) {
            m_addPanelAction->setIcon(KIcon("list-add"));
        }

        constraintsEvent(Plasma::ImmutableConstraint);
    }

    QList<QAction*> actions;

    QAction *a = action("add widgets");
    if (a) {
        actions.append(a);
    }

    if (m_addPanelAction) {
        actions.append(m_addPanelAction);
    }

    a = action("lock widgets");
    if (a) {
        actions.append(a);
    }

    actions.append(m_configureAction);

    a = action("remove");
    if (a) {
        actions.append(a);
    }

    return actions;
}

void Panel::backgroundChanged()
{
    constraintsEvent(Plasma::LocationConstraint);
}

void Panel::adjustLastSpace()
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());
    if (!lay) {
        return;
    }

    bool useSpacer = true;

    if (formFactor() == Plasma::Vertical) {
        foreach (Applet *applet, applets()) {
            if (applet->sizePolicy().verticalPolicy() == QSizePolicy::Expanding || applet->sizePolicy().verticalPolicy() == QSizePolicy::MinimumExpanding) {
                useSpacer = false;
                break;
            }
        }
    } else {
        foreach (Applet *applet, applets()) {
            if (applet->sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag) {
                useSpacer = false;
                break;
            }
        }
    }

    if (useSpacer) {
        if (!m_lastSpace) {
            m_lastSpace = new QGraphicsWidget(this);
            m_lastSpace->setPreferredSize(0,0);
            m_lastSpace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            lay->addItem(m_lastSpace);
        }
    } else {
        lay->removeItem(m_lastSpace);
        delete m_lastSpace;
        m_lastSpace = 0;
    }
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

    //Enlarge the panel if possible and needed
    QSizeF appletHint = applet->preferredSize();
    QSizeF panelHint = layout()->preferredSize();
    if (f == Plasma::Horizontal) {
        if (panelHint.width() + appletHint.width() > size().width()) {
            resize(panelHint.width() + appletHint.width(), size().height());
        }
    } else {
        if (panelHint.height() + appletHint.height() > size().height()) {
            resize(size().width(), panelHint.height() + appletHint.height());
        }
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

    lay->removeItem(m_lastSpace);

    if (insertIndex == -1) {
        lay->addItem(applet);
    } else {
        lay->insertItem(insertIndex, applet);
    }

    if (m_lastSpace) {
        lay->addItem(m_lastSpace);
    }

    //FIXME: there must be some beter way to do this rather than this rather error prone arbitrary wait
    m_lastSpaceTimer->start(2000);

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(updateSize()));
}

void Panel::appletRemoved(Plasma::Applet* applet)
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());
    if (!lay) {
        return;
    }

    lay->removeItem(applet);

    //shrink the panel if possible
    if (formFactor() == Plasma::Horizontal) {
        resize(size().width() - applet->size().width(), size().height());
    } else {
        resize(size().width(), size().height() - applet->size().height());
    }
    layout()->setMaximumSize(size());

    m_lastSpaceTimer->start(200);
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

void Panel::addPanel()
{
    KPluginInfo::List panelPlugins = listContainmentsOfType("panel");

    if (!panelPlugins.isEmpty()) {
        addPanel(panelPlugins.first().pluginName());
    }
}

void Panel::addPanel(const QString &plugin)
{
    if (corona()) {
        Containment* panel = corona()->addContainment(plugin);
        panel->showConfigurationInterface();
        panel->setScreen(screen());

        QList<Plasma::Location> freeEdges = corona()->freeEdges(screen());
        //kDebug() << freeEdges;
        Plasma::Location destination;
        if (freeEdges.contains(Plasma::TopEdge)) {
            destination = Plasma::TopEdge;
        } else if (freeEdges.contains(Plasma::BottomEdge)) {
            destination = Plasma::BottomEdge;
        } else if (freeEdges.contains(Plasma::LeftEdge)) {
            destination = Plasma::LeftEdge;
        } else if (freeEdges.contains(Plasma::RightEdge)) {
            destination = Plasma::RightEdge;
        } else destination = Plasma::TopEdge;

        panel->setLocation(destination);

        // trigger an instant layout so we immediately have a proper geometry
        // rather than waiting around for the event loop
        panel->updateConstraints(Plasma::StartupCompletedConstraint);
        panel->flushPendingConstraintsEvents();

        const QRect screenGeom = corona()->screenGeometry(screen());
        const QRegion availGeom = corona()->availableScreenRegion(screen());
        int minH = 10;
        int minW = 10;
        int w = 35;
        int h = 35;

        if (destination == Plasma::LeftEdge) {
            QRect r = availGeom.intersected(QRect(0, 0, w, screenGeom.height())).boundingRect();
            h = r.height();
            minW = 35;
        } else if (destination == Plasma::RightEdge) {
            QRect r = availGeom.intersected(QRect(screenGeom.width() - w, 0, w, screenGeom.height())).boundingRect();
            h = r.height();
            minW = 35;
        } else if (destination == Plasma::TopEdge) {
            QRect r = availGeom.intersected(QRect(0, 0, screenGeom.width(), h)).boundingRect();
            w = r.width();
            minH = 35;
        } else if (destination == Plasma::BottomEdge) {
            QRect r = availGeom.intersected(QRect(0, screenGeom.height() - h, screenGeom.width(), h)).boundingRect();
            w = r.width();
            minH = 35;
        }

        panel->setMinimumSize(minW, minH);
        panel->setMaximumSize(w, h);
        panel->resize(w, h);
    }
}

void Panel::updateBorders(const QRect &geom)
{
    Plasma::Location loc = location();
    FrameSvg::EnabledBorders enabledBorders = FrameSvg::AllBorders;

    int s = screen();
    //kDebug() << loc << s << formFactor() << geometry();

    qreal topHeight = m_background->marginSize(Plasma::TopMargin);
    qreal bottomHeight = m_background->marginSize(Plasma::BottomMargin);
    qreal leftWidth = m_background->marginSize(Plasma::LeftMargin);
    qreal rightWidth = m_background->marginSize(Plasma::RightMargin);

    //remove unwanted borders
    if (s < 0) {
        // do nothing in this case, we want all the borders
    } else if (loc == BottomEdge || loc == TopEdge) {
        QRect r = Kephal::ScreenUtils::screenGeometry(s);

        if (loc == BottomEdge) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        } else {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }

        if (geom.x() <= r.x()) {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.right() >= r.right()) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        }

        //kDebug() << "top/bottom: Width:" << width << ", height:" << height;
    } else if (loc == LeftEdge || loc == RightEdge) {
        QRect r = Kephal::ScreenUtils::screenGeometry(s);

        if (loc == RightEdge) {
            enabledBorders ^= FrameSvg::RightBorder;
            rightWidth = 0;
        } else {
            enabledBorders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
        }
        if (geom.y() <= r.y()) {
            enabledBorders ^= FrameSvg::TopBorder;
            topHeight = 0;
        }
        if (geom.bottom() >= r.bottom()) {
            enabledBorders ^= FrameSvg::BottomBorder;
            bottomHeight = 0;
        }

        //kDebug() << "left/right: Width:" << width << ", height:" << height;
    } else {
        kDebug() << "no location!?";
    }

    //activate borders and fetch sizes again
    m_background->setEnabledBorders(enabledBorders);
    m_background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    //calculation of extra margins has to be done after getMargins
    const QGraphicsItem *box = toolBoxItem();
    if (box && immutability() == Mutable) {
        QSizeF s = box->boundingRect().size();
        if (formFactor() == Vertical) {
            //hardcoded extra margin for the toolbox right now
            bottomHeight += s.height();
            //Default to horizontal for now
        } else {
            rightWidth += s.width();
        }
    }

    //invalidate the layout and set again
    if (layout()) {
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
        qreal oldLeft, oldTop, oldRight, oldBottom;
        layout()->getContentsMargins(&oldLeft, &oldTop, &oldRight, &oldBottom);

        layout()->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        if (formFactor() == Plasma::Vertical) {
            setPreferredHeight(preferredHeight() - (oldBottom-bottomHeight));
        } else if (QApplication::layoutDirection() == Qt::LeftToRight) {
            setPreferredWidth(preferredWidth() - (oldRight-rightWidth));
        } else {
            setPreferredWidth(preferredWidth() - (oldLeft-leftWidth));
        }
        layout()->invalidate();
        resize(preferredSize());
    }

    update();
}

void Panel::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug() << "constraints updated with" << constraints << "!!!!!!";

    m_maskDirty = true;

    if (constraints & Plasma::FormFactorConstraint) {
        Plasma::FormFactor form = formFactor();
        Qt::Orientation layoutDirection = form == Plasma::Vertical ? Qt::Vertical : Qt::Horizontal;
        // create or set up our layout!
        if (layout()) {
            QGraphicsLinearLayout * linearLay = static_cast<QGraphicsLinearLayout *>(layout());
            linearLay->setMaximumSize(size());
            linearLay->setOrientation(layoutDirection);
        }
    }

    //we need to know if the width or height is 100%
    if (constraints & Plasma::LocationConstraint || constraints & Plasma::SizeConstraint) {
        m_currentSize = geometry().size().toSize();
        QRectF screenRect = screen() >= 0 ? Kephal::ScreenUtils::screenGeometry(screen()) :
                                            geometry();

        if ((formFactor() == Horizontal && m_currentSize.width() >= screenRect.width()) ||
            (formFactor() == Vertical && m_currentSize.height() >= screenRect.height())) {
            m_background->setElementPrefix(location());
        } else {
            switch (location()) {
            case LeftEdge:
                //this call will automatically fallback at no prefix if the element isn't available
                m_background->setElementPrefix("west-mini");
                break;
            case RightEdge:
                m_background->setElementPrefix("east-mini");
                break;
            case TopEdge:
                m_background->setElementPrefix("north-mini");
                break;
            case BottomEdge:
            default:
                m_background->setElementPrefix("south-mini");
                break;
            }
        }

        m_background->resizeFrame(m_currentSize);

        //FIXME: this seems the only way to correctly resize the layout the first time when the
        // saved panel size is less than the default is to setting a maximum size.
        // this shouldn't happen. maybe even a qgraphicslayout bug?
        if (layout() && (constraints & Plasma::SizeConstraint)) {
            layout()->setMaximumSize(size());
        }

        if (constraints & Plasma::LocationConstraint) {
            setFormFactorFromLocation(location());
        }
    }

    if (constraints & Plasma::ImmutableConstraint) {
        bool unlocked = immutability() == Plasma::Mutable;

        if (m_addPanelAction) {
            m_addPanelAction->setEnabled(unlocked);
            m_addPanelAction->setVisible(unlocked);
        }

        if (m_configureAction) {
            m_configureAction->setEnabled(unlocked);
            m_configureAction->setVisible(unlocked);
        }

        updateBorders(geometry().toRect());
    }
}

void Panel::saveState(KConfigGroup &config) const
{
    config.writeEntry("minimumSize", minimumSize());
    config.writeEntry("maximumSize", maximumSize());
}

void Panel::themeUpdated()
{
    if (!layout()) {
        return;
    }

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
    painter->resetTransform();

    const Containment::StyleOption *containmentOpt = qstyleoption_cast<const Containment::StyleOption *>(option);

    QRect viewGeom;
    if (containmentOpt && containmentOpt->view) {
        viewGeom = containmentOpt->view->geometry();
    }

    if (m_maskDirty || m_lastViewGeom != viewGeom) {
        m_maskDirty = false;
        m_lastViewGeom = viewGeom;

        updateBorders(viewGeom);
        if (containmentOpt && containmentOpt->view && !m_background->mask().isEmpty()) {
            containmentOpt->view->setMask(m_background->mask());
        }
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->setRenderHint(QPainter::Antialiasing);

    m_background->paintFrame(painter, option->exposedRect);
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

void Panel::showDropZone(const QPoint pos)
{
    if (!scene()) {
        return;
    }

    // if the drop isn't happening on the outer edges and is instead
    // actually poised over an applet, ignore it
    if (((formFactor() == Plasma::Vertical && pos.y() > 1 && pos.y() > size().height() - 2) ||
         (pos.x() > 1 && pos.x() < size().width() - 2)) &&
        scene()->itemAt(mapToScene(pos)) != this) {
        return;
    }

    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout*>(layout());

    if (!lay) {
        return;
    }

    if (pos == QPoint()) {
        if (m_spacer) {
            lay->removeItem(m_spacer);
            m_spacer->hide();
        }
        return;
    }

    //lucky case: the spacer is already in the right position
    if (m_spacer && m_spacer->geometry().contains(pos)) {
        return;
    }

    Plasma::FormFactor f = formFactor();
    int insertIndex = -1;

    //FIXME: needed in two places, make it a function?
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

    m_spacerIndex = insertIndex;
    if (insertIndex != -1) {
        if (!m_spacer) {
            m_spacer = new Spacer(this);
            m_spacer->panel = this;
        }
        lay->removeItem(m_spacer);
        m_spacer->show();
        lay->insertItem(insertIndex, m_spacer);
    }
}


K_EXPORT_PLASMA_APPLET(panel, Panel)

#include "panel.moc"

