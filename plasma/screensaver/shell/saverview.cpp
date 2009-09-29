/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "saverview.h"

#include <QKeyEvent>
#include <QTimer>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Svg>

#include <widgetsExplorer/widgetexplorer.h>

#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard

class ScreenSaverWidgetExplorer : public Plasma::WidgetExplorer
{
public:
    ScreenSaverWidgetExplorer(QGraphicsWidget *parent)
        : Plasma::WidgetExplorer(parent)
    {

    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->fillRect(option->rect, QColor(0, 0, 0, 160));
    }
};

SaverView::SaverView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, parent),
      m_widgetExplorer(0),
      m_suppressShow(false),
      m_setupMode(false),
      m_init(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
            Qt::X11BypassWindowManagerHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    //app is doing this for us - if needed
    //QDesktopWidget *desktop = QApplication::desktop();
    //setGeometry(desktop->screenGeometry(containment->screen()));

    setWallpaperEnabled(!PlasmaApp::hasComposite());

    installEventFilter(this);
}

SaverView::~SaverView()
{
    delete m_widgetExplorer;
}

void SaverView::enableSetupMode()
{
    if (!m_setupMode) {
        m_setupMode = true;
        update();
    }
}

void SaverView::disableSetupMode()
{
    if (m_setupMode) {
        m_setupMode = false;
        update();
    }
}

void SaverView::drawBackground(QPainter * painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, QColor(0, 0, 0, 0));
        //FIXME kwin's shadow effect is getting drawn behind me. do not want.
    } else {
        Plasma::View::drawBackground(painter, rect);
    }
}

void SaverView::showWidgetExplorer()
{
    Plasma::Containment *c = containment();
    if (!c) {
        return;
    }

    if (m_widgetExplorer) {
        delete m_widgetExplorer;
    } else {
        m_widgetExplorer = new ScreenSaverWidgetExplorer(c);
        m_widgetExplorer->setContainment(c);
        m_widgetExplorer->setOrientation(Qt::Horizontal);
        m_widgetExplorer->setIconSize(KIconLoader::SizeHuge);
        m_widgetExplorer->populateWidgetList();
        m_widgetExplorer->setMaximumWidth(width());
        m_widgetExplorer->adjustSize();
        m_widgetExplorer->setPos(0, c->geometry().height() - m_widgetExplorer->geometry().height());
        m_widgetExplorer->setZValue(1000000);
    }
}

void SaverView::hideAppletBrowser()
{
    delete m_widgetExplorer;
}

void SaverView::paintEvent(QPaintEvent *event)
{
    Plasma::View::paintEvent(event);
    if (!m_setupMode) {
        return;
    }

    // now draw a little label reminding the user their screen's not quite locked
    const QRect r = rect();
    const QString text = i18n("Setup Mode - Screen is NOT locked");
    QFont f = font();
    f.bold();
    const QFontMetrics fm(f);
    const int margin = 6;
    const int textWidth = fm.width(text);
    const QPoint centered(r.width() / 2 - textWidth / 2 - margin, r.y());
    const QRect boundingBox(centered, QSize(margin * 2 + textWidth, fm.height() + margin * 2));

    if (!viewport() || !event->rect().intersects(boundingBox)) {
        return;
    }

    QPainterPath box;
    box.moveTo(boundingBox.topLeft());
    box.lineTo(boundingBox.bottomLeft() + QPoint(0, -margin * 2));
    box.quadTo(boundingBox.bottomLeft(), boundingBox.bottomLeft() + QPoint(margin * 2, 0));
    box.lineTo(boundingBox.bottomRight() + QPoint(-margin * 2, 0));
    box.quadTo(boundingBox.bottomRight(), boundingBox.bottomRight() + QPoint(0, -margin * 2));
    box.lineTo(boundingBox.topRight());
    box.closeSubpath();

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(f);
    //kDebug() << "******************** painting from" << centered << boundingBox << rect() << event->rect();
    QColor highlight = palette().highlight().color();
    highlight.setAlphaF(0.7);
    painter.setPen(highlight.darker());
    painter.setBrush(highlight);
    painter.drawPath(box);
    painter.setPen(palette().highlightedText().color());
    painter.drawText(boundingBox, Qt::AlignCenter | Qt::AlignVCenter, text);
}

bool SaverView::eventFilter(QObject *watched, QEvent *event)
{
#if 0
    if (watched != m_widgetExplorer) {
        /*if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                hideView();
            }
        }*/
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        m_widgetExplorerDragStart = me->globalPos();
    } else if (event->type() == QEvent::MouseMove && m_widgetExplorerDragStart != QPoint()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        QPoint newPos = me->globalPos();
        QPoint curPos = m_widgetExplorer->pos();
        int x = curPos.x();
        int y = curPos.y();

        if (curPos.y() == 0 || curPos.y() + m_widgetExplorer->height() >= height()) {
           x = curPos.x() + (newPos.x() - m_widgetExplorerDragStart.x());
           if (x < 0) {
               x = 0;
           } else if (x + m_widgetExplorer->width() > width()) {
               x = width() - m_widgetExplorer->width();
           }
        }

        if (x == 0 || x + m_widgetExplorer->width() >= width()) {
            y = m_widgetExplorer->y() + (newPos.y() - m_widgetExplorerDragStart.y());

            if (y < 0) {
                y = 0;
            } else if (y + m_widgetExplorer->height() > height()) {
                y = height() - m_widgetExplorer->height();
            }
        }
        m_widgetExplorer->move(x, y);
        m_widgetExplorerDragStart = newPos;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        m_widgetExplorerDragStart = QPoint();
    }
#endif
    return false;
}

void SaverView::showView()
{
    if (isHidden()) {
        if (m_suppressShow) {
            kDebug() << "show was suppressed";
            return;
        }

        setWindowState(Qt::WindowFullScreen);
        //KWindowSystem::setOnAllDesktops(winId(), true);
        //KWindowSystem::setState(winId(), NET::KeepAbove|NET::SkipTaskbar);

        show();
        raise();

        m_suppressShow = true;
        QTimer::singleShot(SUPPRESS_SHOW_TIMEOUT, this, SLOT(suppressShowTimeout()));
    }
}

void SaverView::setContainment(Plasma::Containment *newContainment)
{
    if (m_init && newContainment == containment()) {
        return;
    }

    m_init = true;

    if (containment()) {
        disconnect(containment(), SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    }

    if (newContainment) {
        connect(newContainment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetExplorer()));
    }

    if (m_widgetExplorer) {
        m_widgetExplorer->setContainment(newContainment);
    }

    View::setContainment(newContainment);
}

void SaverView::hideView()
{
    if (isHidden()) {
        return;
    }

    delete m_widgetExplorer;

    if (containment()) {
        containment()->closeToolBox();
    }

    hide();
    //let the lockprocess know
    emit hidden();
}

void SaverView::suppressShowTimeout()
{
    kDebug() << "SaverView::suppressShowTimeout";
    m_suppressShow = false;
}

void SaverView::keyPressEvent(QKeyEvent *event)
{
    /*if (event->key() == Qt::Key_Escape) {
        hideView();
        event->accept();
        return;
    }*/

    //kDebug() << event->key() << event->spontaneous();
    Plasma::View::keyPressEvent(event);
}

#include "saverview.moc"

