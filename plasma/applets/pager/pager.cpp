/***************************************************************************
 *   Copyright (C) 2007 by Daniel Laidig <d.laidig@gmx.de>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "pager.h"

#include <math.h>

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QDesktopWidget>
#include <QTimer>
#include <QX11Info>

#include <KDialog>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KSharedConfig>
#include <KWindowSystem>
#include <NETRootInfo>
#include <KToolInvocation>
#include <kmanagerselection.h>

#include <plasma/svg.h>
#include <plasma/theme.h>

const int WINDOW_UPDATE_DELAY = 50;

Pager::Pager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0),
      m_showDesktopNumber(true),
      m_rows(2),
      m_columns(0),
      m_dragId(0),
      m_dragHighlightedDesktop(-1)
{
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);

    // initialize with a decent default
    m_desktopCount = KWindowSystem::numberOfDesktops();
    m_size = QSizeF(88, 44);
    setContentSize(m_size);
}

void Pager::init()
{
    createMenu();

    KConfigGroup cg = config();
    m_showDesktopNumber = cg.readEntry("showDesktopNumber", m_showDesktopNumber);
    m_rows = globalConfig().readEntry("rows", m_rows);

    if (m_rows < 1) {
        m_rows = 1;
    } else if (m_rows > m_desktopCount) {
        m_rows = m_desktopCount;
    }

    recalculateGeometry();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(recalculateWindowRects()));

    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(currentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(activeWindowChanged(WId)));
    connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(numberOfDesktopsChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(stackingOrderChanged()), this, SLOT(stackingOrderChanged()));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,unsigned int)), this, SLOT(windowChanged(WId,unsigned int)));
    connect(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)), this, SLOT(showingDesktopChanged(bool)));

    m_desktopLayoutOwner = new KSelectionOwner( QString( "_NET_DESKTOP_LAYOUT_S%1" )
        .arg( QX11Info::appScreen()).toLatin1().data(), QX11Info::appScreen(), this );
    connect( m_desktopLayoutOwner, SIGNAL( lostOwnership()), SLOT( lostDesktopLayoutOwner()));
    if ( !m_desktopLayoutOwner->claim( false ))
        lostDesktopLayoutOwner();

    m_currentDesktop = KWindowSystem::currentDesktop();
}

QPainterPath Pager::shape() const
{
    if (drawStandardBackground()) {
        return Plasma::Applet::shape();
    }

    return Plasma::Widget::shape();
}

void Pager::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        recalculateGeometry();
        recalculateWindowRects();
    }
}

void Pager::createMenu()
{
    QAction* configureDesktop = new QAction(SmallIcon("configure"),i18n("&Configure Desktops..."), this);
    actions.append(configureDesktop);
    connect(configureDesktop, SIGNAL(triggered(bool)), this , SLOT(slotConfigureDesktop()));
}

QList<QAction*> Pager::contextActions()
{
  return actions;
}

Qt::Orientations Pager::expandingDirections() const
{
    return 0;
}

QSizeF Pager::contentSizeHint() const
{
    return m_size;
}

void Pager::slotConfigureDesktop()
{
  QString error;
  KToolInvocation::startServiceByDesktopName("desktop", QStringList(), &error);
}

void Pager::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Pager"));

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

    }
    ui.showDesktopNumberCheckBox->setChecked(m_showDesktopNumber);

    ui.spinRows->setValue(m_rows);
    ui.spinRows->setMaximum(m_desktopCount);
    m_dialog->show();
}

void Pager::recalculateGeometry()
{
    if (!m_rects.isEmpty() && contentSize() == m_size) {
        //kDebug() << "leaving because" << !m_rects.isEmpty() << " and " << contentSize() << "==" << m_size;
        return;
    }

    const int padding = 2;

    int columns = m_desktopCount / m_rows + m_desktopCount % m_rows;
    qreal itemHeight;
    qreal itemWidth;

    if (formFactor() == Plasma::Vertical) {
        itemWidth = (contentSize().width() - padding * (columns - 1)) / columns;
        m_scaleFactor = itemWidth / QApplication::desktop()->width();
        itemHeight = QApplication::desktop()->height() * m_scaleFactor;
    } else {
        itemHeight = (contentSize().height() - padding * (m_rows - 1)) / m_rows;
        m_scaleFactor = itemHeight / QApplication::desktop()->height();
        itemWidth = QApplication::desktop()->width() * m_scaleFactor;
    }

    m_rects.clear();
    QRectF itemRect;
    itemRect.setWidth(floor(itemWidth - 1));
    itemRect.setHeight(floor(itemHeight - 1));
    for (int i = 0; i < m_desktopCount; i++) {
        itemRect.moveLeft(floor((i % columns) * (itemWidth + padding)));
        itemRect.moveTop(floor((i / columns) * (itemHeight + padding)));
        m_rects.append(itemRect);
    }

    m_size = QSizeF(ceil(columns * itemWidth + padding * (columns - 1)),
                    ceil(m_rows * itemHeight + padding * (m_rows - 1)));

    updateGeometry();
    kDebug() << "new size set" << m_size << m_rows << m_columns << columns << itemWidth;

    if (m_desktopLayoutOwner && columns != m_columns) {
        // must own manager selection before setting global desktop layout
        m_columns = columns;
        NET::Orientation orient = NET::OrientationHorizontal;
        NETRootInfo i( QX11Info::display(), 0 );
        i.setDesktopLayout( orient, columns, m_rows, NET::DesktopLayoutCornerTopLeft );
    }
}

void Pager::recalculateWindowRects()
{
    QList<WId> windows = KWindowSystem::stackingOrder();
    m_windowRects.clear();
    for (int i = 0; i < m_desktopCount; i++) {
        m_windowRects.append(QList<QPair<WId, QRect> >());
    }
    m_activeWindows.clear();
    foreach(WId window, windows) {
        KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMGeometry | NET::WMFrameExtents | NET::WMWindowType | NET::WMDesktop | NET::WMState | NET::XAWMState);
        NET::WindowType type = info.windowType(NET::NormalMask | NET::DialogMask | NET::OverrideMask |
                                               NET::UtilityMask | NET::DesktopMask | NET::DockMask |
                                               NET::TopMenuMask | NET::SplashMask | NET::ToolbarMask |
                                               NET::MenuMask);

        // the reason we don't check for -1 or Net::Unknown here is that legitimate windows, such
        // as some java application windows, may not have a type set for them.
        // apparently sane defaults on properties is beyond the wisdom of x11.
        if (type == NET::Desktop || type == NET::Dock || type == NET::TopMenu ||
            type == NET::Splash || type == NET::Menu || type == NET::Toolbar ||
            info.hasState(NET::SkipPager) || info.isMinimized()) {
            continue;
        }

        for (int i = 0; i < m_desktopCount; i++) {
            if (!info.isOnDesktop(i+1)) {
                continue;
            }
            QRect windowRect = info.frameGeometry();
            if( KWindowSystem::mapViewport())
                windowRect = fixViewportPosition( windowRect );
            windowRect = QRectF(windowRect.x() * m_scaleFactor,
                                windowRect.y() * m_scaleFactor,
                                windowRect.width() * m_scaleFactor, 
                                windowRect.height() * m_scaleFactor).toRect();
            windowRect.translate(m_rects[i].topLeft().toPoint());
            m_windowRects[i].append(QPair<WId, QRect>(window, windowRect));
            if (window == KWindowSystem::activeWindow()) {
                m_activeWindows.append(windowRect);
            }
        }
    }
    update();
}

void Pager::configAccepted()
{
    KConfigGroup cg = config();
    bool changed = false;

    if (m_showDesktopNumber != ui.showDesktopNumberCheckBox->isChecked()) {
        m_showDesktopNumber = ui.showDesktopNumberCheckBox->isChecked();
        cg.writeEntry("showDesktopNumber", m_showDesktopNumber);
        changed = true;
    }

    // we need to keep all pager applets consistent since this affects
    // the layout of the desktops as used by the window manager,
    // so we store the row count in the applet global configuration
    if (m_rows != ui.spinRows->value()) {
        KConfigGroup globalcg = globalConfig();
        m_rows = ui.spinRows->value();
        if (m_rows > m_desktopCount) {
            m_rows = m_desktopCount;
        }
        globalcg.writeEntry("rows", m_rows);
        changed = true;
    }

    if (changed) {
        configNeedsSaving();
        // force an update
        m_columns = 0;
        m_size = QSizeF(-1, -1);
        recalculateGeometry();
        recalculateWindowRects();
        update();
    }
}

void Pager::currentDesktopChanged(int desktop)
{
    m_currentDesktop = desktop;

    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::windowAdded(WId id)
{
    Q_UNUSED(id)

    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::windowRemoved(WId id)
{
    Q_UNUSED(id)

    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::activeWindowChanged(WId id)
{
    Q_UNUSED(id)

    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::numberOfDesktopsChanged(int num)
{
    m_desktopCount = num;
    if (m_rows > m_desktopCount) {
        m_rows = m_desktopCount;
    }
    m_rects.clear();
    recalculateGeometry();

    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::stackingOrderChanged()
{
    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::windowChanged(WId id, unsigned int properties)
{
    Q_UNUSED(id)

    if (properties & NET::WMGeometry ||
        properties & NET::WMDesktop) {
        if (!m_timer->isActive()) {
            m_timer->start(WINDOW_UPDATE_DELAY);
        }
    }
}

void Pager::showingDesktopChanged(bool showing)
{
    Q_UNUSED(showing)
    if (!m_timer->isActive()) {
        m_timer->start(WINDOW_UPDATE_DELAY);
    }
}

void Pager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() != Qt::RightButton)
    {
        for (int i = 0; i < m_desktopCount; i++) {
            if (m_rects[i].contains(event->pos())) {
                m_dragStartDesktop = m_dragHighlightedDesktop = i;
                m_dragOriginalPos = m_dragCurrentPos = event->pos();
                if (m_dragOriginal.isEmpty()) {
                    m_dragOriginal = m_rects[i].toRect();
                }

                update();
                return;
            }
        }
    }
    Applet::mousePressEvent(event);
}

void Pager::wheelEvent(QGraphicsSceneWheelEvent *e)
{
    int newDesk;
    int desktops = KWindowSystem::numberOfDesktops();

    /*
       if (m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height() > 1 )
       desktops = m_kwin->numberOfViewports(0).width() * m_kwin->numberOfViewports(0).height();
       */
    if (e->delta() < 0)
    {
        newDesk = m_currentDesktop % desktops + 1;
    }
    else
    {
        newDesk = (desktops + m_currentDesktop - 2) % desktops + 1;
    }

    KWindowSystem::setCurrentDesktop(newDesk);
    m_currentDesktop = newDesk;
    update();

    Applet::wheelEvent(e);
}

void Pager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragId > 0) {
        m_dragCurrentPos = event->pos();
        m_dragHighlightedDesktop = -1;
        for (int i = 0; i < m_desktopCount; i++) {
            if (m_rects[i].contains(event->pos().toPoint())) {
                m_dragHighlightedDesktop = i;
                break;
            }
        }
        m_hoverRect = QRectF();
        foreach (QRectF rect, m_rects) {
            if (rect.contains(event->pos())) {
                m_hoverRect = rect;
                break;
            }
        }
        update();
        event->accept();
        return;
    } else if (m_dragId != -1 && m_dragStartDesktop != -1 &&
               (event->pos() - m_dragOriginalPos).toPoint().manhattanLength() > KGlobalSettings::dndEventDelay()) {
        m_dragId = -1; // prevent us from going through this more than once
        for (int k = m_windowRects[m_dragStartDesktop].count() - 1; k >= 0 ; k--) {
            if (m_windowRects[m_dragStartDesktop][k].second.contains(m_dragOriginalPos.toPoint())) {
                m_dragOriginal = m_windowRects[m_dragStartDesktop][k].second;
                m_dragId = m_windowRects[m_dragStartDesktop][k].first;
                event->accept();
                break;
            }
        }
    }

    if (m_dragOriginal.isEmpty()) {
        Applet::mouseMoveEvent(event);
    }
}

void Pager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragId > 0) {
        if (m_dragHighlightedDesktop != -1) {
            QPointF dest = m_dragCurrentPos - m_rects[m_dragHighlightedDesktop].topLeft() - m_dragOriginalPos + m_dragOriginal.topLeft();
            dest = QPointF(dest.x()/m_scaleFactor, dest.y()/m_scaleFactor);
            if( !KWindowSystem::mapViewport()) {
                KWindowSystem::setOnDesktop(m_dragId, m_dragHighlightedDesktop+1);
                // use _NET_MOVERESIZE_WINDOW rather than plain move, so that the WM knows this is a pager request
                NETRootInfo i( QX11Info::display(), 0 );
                int flags = ( 0x20 << 12 ) | ( 0x03 << 8 ) | 1; // from tool, x/y, northwest gravity
                i.moveResizeWindowRequest( m_dragId, flags, dest.toPoint().x(), dest.toPoint().y(), 0, 0 );
            } else {
                // setOnDesktop() with viewports is also moving a window, and since it takes a moment
                // for the WM to do the move, there's a race condition with figuring out how much to move,
                // so do it only as one move
                dest += KWindowSystem::desktopToViewport( m_dragHighlightedDesktop+1, false );
                QPoint d = KWindowSystem::constrainViewportRelativePosition( dest.toPoint());
                NETRootInfo i( QX11Info::display(), 0 );
                int flags = ( 0x20 << 12 ) | ( 0x03 << 8 ) | 1; // from tool, x/y, northwest gravity
                i.moveResizeWindowRequest( m_dragId, flags, d.x(), d.y(), 0, 0 );
            }
        }
        m_timer->start();
    } else if (m_dragStartDesktop != -1 && m_rects[m_dragStartDesktop].contains(event->pos().toPoint()) && 
               m_currentDesktop != m_dragStartDesktop + 1) {
        // only change the desktop if the user presses and releases the mouse on the same desktop
        KWindowSystem::setCurrentDesktop(m_dragStartDesktop + 1);
        m_currentDesktop = m_dragStartDesktop + 1;
    }

    m_dragId = 0;
    m_dragOriginal = QRect();
    m_dragHighlightedDesktop = -1;
    m_dragStartDesktop = -1;
    m_dragOriginalPos = m_dragCurrentPos = QPointF();

    update();
    Applet::mouseReleaseEvent(event);
}

void Pager::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hoverMoveEvent(event);
    Applet::hoverEnterEvent(event);
}

void Pager::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    foreach (QRectF rect, m_rects) {
        if (rect.contains(event->pos())) {
            if (m_hoverRect != rect) {
                m_hoverRect = rect;
                update();
            }
            return;
        }
    }
    m_hoverRect = QRectF();
    update();
}

void Pager::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event );
    if (m_hoverRect != QRectF()) {
        m_hoverRect = QRectF();
        update();
    }
    
    // The applet doesn't always get mouseReleaseEvents, for example when starting a drag
    // on the panel and releasing the mouse on the desktop or another window. This can cause
    // weird bugs because the pager still thinks a drag is going on.
    // The only reliable event I found is the hoverLeaveEvent, so we just stop the drag
    // on this event.
    if (m_dragId || m_dragStartDesktop != -1) {
        m_dragId = 0;
        m_dragOriginal = QRect();
        m_dragHighlightedDesktop = -1;
        m_dragStartDesktop = -1;
        m_dragOriginalPos = m_dragCurrentPos = QPointF();
        update();
    }
    
    Applet::hoverLeaveEvent(event);
}

void Pager::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );
    Q_UNUSED( contentsRect );

    // paint desktop background
    QBrush hoverBrush(QColor(255,255,255,50)); // FIXME: hardcoded colors == bad
    QBrush defaultBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);
    for (int i = 0; i < m_desktopCount; i++) {
        if (m_rects[i] == m_hoverRect) {
            painter->setBrush(hoverBrush);
        } else {
            painter->setBrush(defaultBrush);
        }
        painter->drawRect(m_rects[i]);
    }

    // draw window thumbnails
    QPen windowPen(KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors()).foreground().color());
    QBrush windowBrush(QColor(200,200,200)); // FIXME: hardcoded color == bad // LinkBackground
    QBrush activeWindowBrush(QColor(100,100,255)); // FIXME:: hardcoded colors == bad ActiveBackground
    painter->setPen(windowPen);
    for (int i = 0; i < m_windowRects.count(); i++) {
        for (int j = 0; j < m_windowRects[i].count(); j++) {
            QRect rect = m_windowRects[i][j].second;
            if (m_activeWindows.contains(rect)) {
                painter->setBrush(activeWindowBrush);
            } else {
                painter->setBrush(windowBrush);
            }
            if (m_dragId == m_windowRects[i][j].first) {
                rect.translate((m_dragCurrentPos - m_dragOriginalPos).toPoint());
                painter->setClipRect(option->exposedRect);
            } else {
                painter->setClipRect(m_rects[i]);
            }
            painter->drawRect(rect);
        }
    }

    // draw desktop foreground
    painter->setClipRect(option->exposedRect);
    QPen activePen(KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors()).foreground().color());
    QPen defaultPen(QColor(120,120,120)); // FIXME: hardcoded color == bad
    painter->setBrush(Qt::NoBrush);
    for( int i = 0; i < m_desktopCount; i++) {
        if (i + 1 == m_currentDesktop || i == m_dragHighlightedDesktop) {
            painter->setPen(activePen);
        } else {
            painter->setPen(defaultPen);
        }

        painter->drawRect(m_rects[i]);

        if (m_showDesktopNumber) {
            painter->drawText(m_rects[i], Qt::AlignCenter, QString::number(i+1));
        }
    }
}

void Pager::lostDesktopLayoutOwner()
{
    delete m_desktopLayoutOwner;
    m_desktopLayoutOwner = NULL;
}

// KWindowSystem does not translate position when mapping viewports
// to virtual desktops (it'd probably break more things than fix),
// so the offscreen coordinates need to be fixed
QRect Pager::fixViewportPosition( const QRect& r )
{
    int x = r.center().x() % qApp->desktop()->width();
    int y = r.center().y() % qApp->desktop()->height();
    if( x < 0 )
        x = x + qApp->desktop()->width();
    if( y < 0 )
        y = y + qApp->desktop()->height();
    return QRect( x - r.width() / 2, y - r.height() / 2, r.width(), r.height());
}

#include "pager.moc"
