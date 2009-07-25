/***************************************************************************
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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

#include <KDebug>

#include <QGraphicsView>
#include <QToolButton>
#include <QPaintEvent>
#include <QMutex>
#include <QScrollBar>

#include <Plasma/AbstractRunner>
#include <Plasma/Svg>


#include "resultsview.h"
#include "resultitem.h"

ResultsView::ResultsView(QWidget * parent)
    : QGraphicsView(parent)
{

    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    setInteractive(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_arrowSvg = new Plasma::Svg(this);
    m_arrowSvg->setImagePath("widgets/arrows");

    m_previousPage = new QToolButton(this);
    m_previousPage->setIcon(m_arrowSvg->pixmap("up-arrow"));
    m_previousPage->setAutoRaise(true);
    m_previousPage->setVisible(false);
    m_previousPage->adjustSize();
    connect(m_previousPage, SIGNAL(clicked(bool)), SLOT(previousPage()));

    m_nextPage = new QToolButton(this);
    m_nextPage->setIcon(m_arrowSvg->pixmap("down-arrow"));
    m_nextPage->setAutoRaise(true);
    m_nextPage->setVisible(false);
    m_nextPage->adjustSize();
    connect(m_nextPage, SIGNAL(clicked(bool)), SLOT(nextPage()));

    updateArrowsVisibility();

    connect(verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(updateArrowsVisibility()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateArrowsVisibility()));
}

ResultsView::~ResultsView()
{
}

void ResultsView::previousPage()
{
    QGraphicsItem * currentItem = scene()->selectedItems().first();
    QGraphicsItem * item = itemAt(0,-height()*0.4);

    if (!item) {
        item = scene()->itemAt(0,0);
    }
    if (item && (item != currentItem)) {
        scene()->setFocusItem(item);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value()-height()*0.4);
    }
}


void ResultsView::nextPage()
{
    QGraphicsItem * currentItem = scene()->selectedItems().first();
    QGraphicsItem * item = itemAt(0,height()*1.4);
    if (!item) {
        item = scene()->itemAt(0,sceneRect().height()-1);
    }
    if (item && (item != currentItem)) {
        scene()->setFocusItem(item);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value()+height()*0.4);
    }
}

void ResultsView::resizeEvent( QResizeEvent * event)
{
    updateArrowsVisibility();
    QGraphicsView::resizeEvent(event);
}

void ResultsView::updateArrowsVisibility()
{
    m_previousPage->move(width()/2-m_previousPage->width()/2,0);
    m_nextPage->move(width()/2-m_nextPage->width()/2,height()-m_nextPage->height());

    if (scene()) {
        m_previousPage->setVisible(mapFromScene(QPointF(0,0)).y()<0);
        m_nextPage->setVisible(mapFromScene(QPointF(0,sceneRect().height())).y()>height());
    }
}

void ResultsView::paintEvent(QPaintEvent *event)
{
    QPixmap backBuffer(viewport()->size());
    backBuffer.fill(Qt::transparent);
    QPainter backBufferPainter(&backBuffer);

    QPainter painter(viewport());

    // Here we need to redirect to a backBuffer since krunnerdialog
    // draws its own background; the QPainter is then propagated to the widgets so that
    // we cannot directly blit with destinationIn because the destination has
    // already the (plasma-themed) background painted.

    painter.setRedirected(viewport(),&backBuffer);
    QGraphicsView::paintEvent(event);
    painter.restoreRedirected(viewport());

    if (m_previousFadeout.isNull() || m_previousFadeout.width() != width()) {
        QLinearGradient g(0, 0, 0, m_previousPage->height());
        g.setColorAt(1, Qt::white );
        g.setColorAt(0, Qt::transparent );
        m_previousFadeout = QPixmap(width(), m_previousPage->height());
        m_previousFadeout.fill(Qt::transparent);
        QPainter p(&m_previousFadeout);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(m_previousFadeout.rect(), g);
    }

    if (m_nextFadeout.isNull() || m_nextFadeout.width() != width()) {
        QLinearGradient g(0, 0, 0, m_nextPage->height());
        g.setColorAt(0, Qt::white );
        g.setColorAt(1, Qt::transparent );
        m_nextFadeout = QPixmap(width(), m_nextPage->height());
        m_nextFadeout.fill(Qt::transparent);
        QPainter p(&m_nextFadeout);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(m_nextFadeout.rect(), g);
    }

    backBufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    if (m_previousPage->isVisible()) {
        backBufferPainter.drawPixmap(QPoint(0,0), m_previousFadeout);
    }
    if (m_nextPage->isVisible()) {
        backBufferPainter.drawPixmap(QPoint(0,height()-m_nextFadeout.height()), m_nextFadeout);
    }
     painter.drawPixmap(event->rect(), backBuffer, event->rect());
}

#include "resultsview.moc"
