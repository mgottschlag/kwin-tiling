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
    setContentSize(QSizeF(88, 44));
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
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId)), this, SLOT(windowChanged(WId)));
    connect(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)), this, SLOT(showingDesktopChanged(bool)));

    m_desktopLayoutOwner = new KSelectionOwner( QString( "_NET_DESKTOP_LAYOUT_S%1" )
        .arg( QX11Info::appScreen()).toLatin1().data(), QX11Info::appScreen(), this );
    connect( m_desktopLayoutOwner, SIGNAL( lostOwnership()), SLOT( lostDesktopLayoutOwner()));
    if ( !m_desktopLayoutOwner->claim( false ))
        lostDesktopLayoutOwner();

    m_currentDesktop = KWindowSystem::currentDesktop();
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
        return;
    }

    qreal itemHeight = (contentSize().height() - ((m_rows - 1) * 2)) / m_rows;
    //kDebug() << "************************* itemHeight is" << itemHeight << contentSize().height();
    m_scaleFactor = itemHeight / QApplication::desktop()->height();
    qreal itemWidth = QApplication::desktop()->width() * m_scaleFactor;
    m_rects.clear();

    int columns = m_desktopCount/m_rows + m_desktopCount%m_rows;
    for (int i = 0; i < m_desktopCount; i++) {
        m_rects.append(QRectF((i%columns)*itemWidth+2*(i%columns),
                               itemHeight*(i/columns) + 2*(i/columns),
                               itemWidth,
                               itemHeight));
    }

    m_size = QSizeF(columns*itemWidth + 2*columns - 1,
                    contentSize().height());
    setContentSize(m_size);
    //kDebug() << "new size set" << m_size << m_rows << m_columns << columns << itemWidth;

    if (m_desktopLayoutOwner && columns != m_columns)
    { // must own manager selection before setting global desktop layout
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
    m_showDesktopNumber = ui.showDesktopNumberCheckBox->isChecked();
    cg.writeEntry("showDesktopNumber", m_showDesktopNumber);

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
        // force an update of the column count in recalculateGeometry
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
    m_timer->start();
}

void Pager::windowAdded(WId id)
{
    Q_UNUSED(id)
    m_timer->start();
}

void Pager::windowRemoved(WId id)
{
    Q_UNUSED(id)
    m_timer->start();
}

void Pager::activeWindowChanged(WId id)
{
    Q_UNUSED(id)
    m_timer->start();
}

void Pager::numberOfDesktopsChanged(int num)
{
    m_desktopCount = num;
    if (m_rows > m_desktopCount) {
        m_rows = m_desktopCount;
    }
    m_rects.clear();
    recalculateGeometry();
    m_timer->start();
}

void Pager::stackingOrderChanged()
{
    m_timer->start();
}

void Pager::windowChanged(WId id)
{
    Q_UNUSED(id)
    m_timer->start();
}

void Pager::showingDesktopChanged(bool showing)
{
    Q_UNUSED(showing)
    m_timer->start();
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
    if (m_dragId) {
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
    } else if (m_dragStartDesktop != -1 && (event->pos() - m_dragOriginalPos).toPoint().manhattanLength() > KGlobalSettings::dndEventDelay()) {
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
    if (m_dragId) {
        if (m_dragHighlightedDesktop != -1) {
            QPointF dest = m_dragCurrentPos - m_rects[m_dragHighlightedDesktop].topLeft() - m_dragOriginalPos + m_dragOriginal.topLeft();
            dest = QPointF(dest.x()/m_scaleFactor, dest.y()/m_scaleFactor);
            KWindowSystem::setOnDesktop(m_dragId, m_dragHighlightedDesktop+1);
            XMoveWindow(QX11Info::display(), m_dragId, dest.toPoint().x(), dest.toPoint().y());
        }
        m_timer->start();
    } else {
        for (int i = 0; i < m_desktopCount; i++) {
            if (m_rects[i].contains(event->pos().toPoint()) && m_currentDesktop != i+1) {
                KWindowSystem::setCurrentDesktop(i+1);
                m_currentDesktop = i+1;
                update();
                break;
            }
        }
    }

    m_dragId = 0;
    m_dragOriginal = QRect();
    m_dragHighlightedDesktop = -1;
    m_dragStartDesktop = -1;
    m_dragOriginalPos = m_dragCurrentPos = QPointF();

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
    Applet::hoverLeaveEvent(event);
}

void Pager::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );
    Q_UNUSED( contentsRect );

    // paint desktop background
    QBrush hoverBrush(QColor(255,255,255,50));
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
    QPen windowPen(Qt::white);
    QBrush windowBrush(QColor(200,200,200));
    QBrush activeWindowBrush(QColor(100,100,255));
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
    QPen activePen(Qt::white);
    QPen defaultPen(QColor(120,120,120));
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

#include "pager.moc"
