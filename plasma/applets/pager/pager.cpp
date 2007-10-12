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

#include <KSharedConfig>
#include <KDialog>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KWindowSystem>
#include <NETRootInfo>

#include <plasma/svg.h>
#include <plasma/theme.h>

Pager::Pager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args), m_dialog(0), m_dragId(0), m_dragHighlightedDesktop(-1)
{
    setAcceptsHoverEvents(true);
    setHasConfigurationInterface(true);

    KConfigGroup cg = config();
    m_showDesktopNumber = cg.readEntry("showDesktopNumber", true);
    m_itemHeight = cg.readEntry("height", 22);
    m_rows = cg.readEntry("rows", 2);

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

    m_currentDesktop = KWindowSystem::currentDesktop();
    numberOfDesktopsChanged(KWindowSystem::numberOfDesktops());
}

Pager::~Pager()
{
}

QSizeF Pager::contentSizeHint() const
{
    return m_size;
}

void Pager::constraintsUpdated(Plasma::Constraints)
{
    if (formFactor() == Plasma::Vertical ||
        formFactor() == Plasma::Horizontal) {
        setDrawStandardBackground(false);
    } else {
        setDrawStandardBackground(true);
    }

     recalculateGeometry();
     recalculateWindowRects();
}

Qt::Orientations Pager::expandingDirections() const
{
    return 0;
}

void Pager::showConfigurationInterface()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption(i18n("Configure Pager"));

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

    }
    ui.showDesktopNumberCheckBox->setChecked(m_showDesktopNumber);
    
    ui.spinHeight->setValue(m_itemHeight);
    ui.spinRows->setValue(m_rows);
    ui.spinRows->setMaximum(m_desktopCount);
    m_dialog->show();
}

void Pager::recalculateGeometry()
{
    prepareGeometryChange();
    m_scaleFactor = qreal(m_itemHeight) / QApplication::desktop()->height();
    qreal itemWidth = QApplication::desktop()->width() * m_scaleFactor;
    m_rects.clear();
    
    int columns = m_desktopCount/m_rows + m_desktopCount%m_rows;
    for(int i = 0; i < m_desktopCount; i++) {
	m_rects.append(QRectF((i%columns)*itemWidth+2*(i%columns),
			       m_itemHeight*(i/columns) + 2*(i/columns),
			       itemWidth,
			       m_itemHeight));
    }
    m_size = QSizeF(columns*itemWidth + 2*columns - 1,
		     m_itemHeight*m_rows + 2*m_rows - 1);

    updateGeometry();
}

void Pager::recalculateWindowRects()
{
    QList<WId> windows = KWindowSystem::stackingOrder();
    m_windowRects.clear();
    for(int i = 0; i < m_desktopCount; i++) {
	m_windowRects.append(QList<QPair<WId, QRect> >());
    }
    m_activeWindows.clear();
    foreach(WId window, windows) {
	KWindowInfo info = KWindowSystem::windowInfo(window, NET::WMGeometry | NET::WMFrameExtents | NET::WMWindowType | NET::WMDesktop | NET::WMState | NET::XAWMState);
	NET::WindowType type = info.windowType(NET::NormalMask | NET::DialogMask);
	if(type == -1 || info.hasState(NET::SkipPager) || info.isMinimized()) {
	    continue;
	}
	for(int i = 0; i < m_desktopCount; i++) {
	    if(!info.isOnDesktop(i+1)) {
		continue;
	    }
	    QRect windowRect = info.frameGeometry();
	    windowRect = QRectF(windowRect.x() * m_scaleFactor,
				 windowRect.y() * m_scaleFactor,
				 windowRect.width() * m_scaleFactor, 
				 windowRect.height() * m_scaleFactor).toRect();
	    windowRect.translate(m_rects[i].topLeft().toPoint());
	    m_windowRects[i].append(QPair<WId, QRect>(window, windowRect));
	    if(window == KWindowSystem::activeWindow()) {
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
    
    m_itemHeight = ui.spinHeight->value();
    cg.writeEntry("height", m_itemHeight);
    
    m_rows = ui.spinRows->value();
    if(m_rows > m_desktopCount) {
	m_rows = m_desktopCount;
    }
    cg.writeEntry("rows", m_rows);
    updateConstraints();
    cg.config()->sync();
}

void Pager::currentDesktopChanged(int desktop)
{
    m_currentDesktop = desktop;
    m_timer->start();
}

void Pager::windowAdded(WId id)
{
    m_timer->start();
}

void Pager::windowRemoved(WId id)
{
    m_timer->start();
}

void Pager::activeWindowChanged(WId id)
{
    m_timer->start();
}

void Pager::numberOfDesktopsChanged(int num)
{
    m_desktopCount = num;
    if(m_rows > m_desktopCount) {
	m_rows = m_desktopCount;
    }
    m_timer->start();
}

void Pager::stackingOrderChanged()
{
    m_timer->start();
}

void Pager::windowChanged(WId id)
{
    m_timer->start();
}

void Pager::showingDesktopChanged(bool showing)
{
    m_timer->start();
}


void Pager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    for(int i = 0; i < m_desktopCount; i++) {
	if(m_rects[i].contains(event->pos())) {
	    for(int j = 0; j < m_windowRects.count(); j++) {
		for(int k = m_windowRects[j].count() - 1; k >= 0 ; k--) {
		    if(m_windowRects[j][k].second.contains(event->pos().toPoint()) && m_rects[i].contains(event->pos().toPoint())) {
			m_dragOriginal = m_windowRects[j][k].second;
			m_dragOriginalPos = m_dragCurrentPos = event->pos();
			m_dragId = m_windowRects[j][k].first;
			break;
		    }
		}
	    }
	    if(m_dragOriginal.isEmpty()) {
		m_dragOriginal = m_rects[i].toRect();
		m_dragOriginalPos = m_dragCurrentPos = event->pos();
	    }
	    for(int i = 0; i < m_desktopCount; i++) {
		if(m_rects[i].contains(event->pos().toPoint())) {
		    m_dragHighlightedDesktop = i;
		    break;
		}
	    }
	    return;
	}
    }
    Applet::mousePressEvent(event);
}

void Pager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_dragId != 0) {
	m_dragCurrentPos = event->pos();
	m_dragHighlightedDesktop = -1;
	for(int i = 0; i < m_desktopCount; i++) {
	    if(m_rects[i].contains(event->pos().toPoint())) {
		m_dragHighlightedDesktop = i;
		break;
	    }
	}
	m_hoverRect = QRectF();
	foreach(QRectF rect, m_rects) {
	    if(rect.contains(event->pos())) {
		m_hoverRect = rect;
		break;
	    }
	}
	update();
    } else if (m_dragOriginal.isEmpty()) {
	Applet::mouseMoveEvent(event);
    }
}

void Pager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->pos() == m_dragOriginalPos) {
	m_dragOriginalPos = m_dragCurrentPos = QPointF();
	m_dragId = 0;
	m_dragOriginal = QRect();
	m_dragHighlightedDesktop = -1;
	for(int i = 0; i < m_desktopCount; i++) {
	    if(m_rects[i].contains(event->pos().toPoint()) && m_currentDesktop != i+1) {
		KWindowSystem::setCurrentDesktop(i+1);
		m_currentDesktop = i+1;
		update();
		return;
	    }
	}
    } else {
	if(m_dragHighlightedDesktop != -1) {
	    QPointF dest = m_dragCurrentPos - m_rects[m_dragHighlightedDesktop].topLeft() - m_dragOriginalPos + m_dragOriginal.topLeft();
	    dest = QPointF(dest.x()/m_scaleFactor, dest.y()/m_scaleFactor);
	    KWindowSystem::setOnDesktop(m_dragId, m_dragHighlightedDesktop+1);
            XMoveWindow(QX11Info::display(), m_dragId, dest.toPoint().x(), dest.toPoint().y());
	}
	m_dragOriginalPos = m_dragCurrentPos = QPointF();
	m_dragId = 0;
	m_dragOriginal = QRect();
	m_dragHighlightedDesktop = -1;
	m_timer->start();
    }
}

void Pager::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hoverMoveEvent(event);
}

void Pager::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    foreach(QRectF rect, m_rects) {
	if(rect.contains(event->pos())) {
	    if(m_hoverRect != rect) {
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
    if(m_hoverRect != QRectF()) {
	m_hoverRect = QRectF();
	update();
    }
}

void Pager::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );
    Q_UNUSED( contentsRect );
    
    // paint desktop background
    QBrush hoverBrush(QColor(255,255,255,50));
    QBrush defaultBrush(Qt::NoBrush);
    painter->setPen(Qt::NoPen);
    for(int i = 0; i < m_desktopCount; i++) {
	if(m_rects[i] == m_hoverRect) {
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
    for(int i = 0; i < m_windowRects.count(); i++) {
	for(int j = 0; j < m_windowRects[i].count(); j++) {
	    QRect rect = m_windowRects[i][j].second;
	    if(m_activeWindows.contains(rect)) {
		painter->setBrush(activeWindowBrush);
	    } else {
		painter->setBrush(windowBrush);
	    }
	    if(m_dragId == m_windowRects[i][j].first) {
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
    for(int i = 0; i < m_desktopCount; i++) {
	if(i + 1 == m_currentDesktop || i == m_dragHighlightedDesktop) {
	    painter->setPen(activePen);
	} else {
	    painter->setPen(defaultPen);
	}
	painter->drawRect(m_rects[i]);
	if(m_showDesktopNumber) {
	    painter->drawText(m_rects[i], Qt::AlignCenter, QString::number(i+1));
	}
    }
}

#include "pager.moc"
