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


#include "resultscene.h"
#include "resultview.h"
#include "resultitem.h"

ResultsView::ResultsView(ResultScene *scene, SharedResultData *resultData, QWidget *parent)
    : QGraphicsView(scene, parent),
      m_resultScene(scene),
      m_resultData(resultData)
{
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    setInteractive(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_arrowSvg = new Plasma::Svg(this);
    m_arrowSvg->setImagePath(QLatin1String( "widgets/arrows" ));

    m_previousPage = new QToolButton(this);
    m_previousPage->setAutoRaise(true);
    m_previousPage->setVisible(false);
    connect(m_previousPage, SIGNAL(clicked(bool)), SLOT(previousPage()));

    m_nextPage = new QToolButton(this);
    m_nextPage->setAutoRaise(true);
    m_nextPage->setVisible(false);
    connect(m_nextPage, SIGNAL(clicked(bool)), SLOT(nextPage()));

    connect(m_arrowSvg, SIGNAL(repaintNeeded()), this, SLOT(updateArrowsIcons()));
    updateArrowsIcons();

    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(updateArrowsVisibility()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateArrowsVisibility()));
    connect(m_resultScene, SIGNAL(ensureVisibility(QGraphicsItem*)), this, SLOT(ensureVisibility(QGraphicsItem*)));
}

ResultsView::~ResultsView()
{
}

void ResultsView::ensureVisibility(QGraphicsItem* item)
{
    m_resultData->processHoverEvents = false;
    ensureVisible(item, 0, 0);
    m_resultData->processHoverEvents = true;
}

void ResultsView::previousPage()
{
    QGraphicsItem *currentItem = m_resultScene->selectedItems().first();
    QGraphicsItem *item = itemAt(0, -height()*0.4);

    if (!item) {
        item = m_resultScene->itemAt(0,0);
    }

    if (item && (item != currentItem)) {
        m_resultScene->setFocusItem(item);
    } else {
        verticalScrollBar()->setValue(verticalScrollBar()->value()-height()*0.4);
    }
}


void ResultsView::nextPage()
{
    QGraphicsItem *currentItem = m_resultScene->selectedItems().first();
    QGraphicsItem *item = itemAt(0, height()*1.4);
    if (!item) {
        item = m_resultScene->itemAt(0,sceneRect().height()-1);
    }

    ResultItem *rItem = dynamic_cast<ResultItem *>(item);
    if (rItem && !rItem->isValid()) {
        item = m_resultScene->itemAt(0, m_resultScene->viewableHeight() - 1);
    }

    if (item && (item != currentItem)) {
        m_resultScene->setFocusItem(item);
    } else {
        verticalScrollBar()->setValue(qMin(m_resultScene->viewableHeight(),
                                           int(verticalScrollBar()->value()+ (height() * 0.4))));
    }
}

void ResultsView::resizeEvent(QResizeEvent * event)
{
    updateArrowsVisibility();
    QGraphicsView::resizeEvent(event);
}

void ResultsView::updateArrowsIcons()
{
    m_previousPage->setIcon(m_arrowSvg->pixmap(QLatin1String( "up-arrow" )));
    m_previousPage->adjustSize();

    m_nextPage->setIcon(m_arrowSvg->pixmap(QLatin1String( "down-arrow" )));
    m_nextPage->adjustSize();

    updateArrowsVisibility();
}

void ResultsView::updateArrowsVisibility()
{
    m_previousPage->move((width() / 2) - (m_previousPage->width() / 2), 0);
    m_nextPage->move((width() / 2) - (m_nextPage->width() / 2), height() - m_nextPage->height());

    m_previousPage->setVisible(mapFromScene(QPointF(0,0)).y() < 0);
    m_nextPage->setVisible(mapFromScene(QPointF(0, m_resultScene->viewableHeight())).y() > height());
    //kDebug() << m_resultScene->viewableHeight() << height() << mapFromScene(QPointF(0, m_resultScene->viewableHeight()));
}

void ResultsView::wheelEvent(QWheelEvent *e)
{
    if (e->delta() < 0 && !m_nextPage->isVisible()) {
        return;
    }

    QGraphicsView::wheelEvent(e);
}

void ResultsView::paintEvent(QPaintEvent *event)
{
    QPixmap backBuffer(viewport()->size());
    backBuffer.fill(Qt::transparent);

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

    QPainter backBufferPainter(&backBuffer);
    backBufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    if (m_previousPage->isVisible()) {
        backBufferPainter.drawPixmap(QPoint(0,0), m_previousFadeout);
    }

    if (m_nextPage->isVisible()) {
        backBufferPainter.drawPixmap(QPoint(0,height()-m_nextFadeout.height()), m_nextFadeout);
    }
    backBufferPainter.end();
    painter.drawPixmap(event->rect(), backBuffer, event->rect());
}

#include "resultview.moc"
