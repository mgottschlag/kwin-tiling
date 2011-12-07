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

#include <widgetsexplorer/widgetexplorer.h>

#include "plasmaapp.h"

static const int SUPPRESS_SHOW_TIMEOUT = 500; // Number of millis to prevent reshow of dashboard


class ScreenSaverWidgetExplorer : public Plasma::WidgetExplorer
{
public:
    ScreenSaverWidgetExplorer(QGraphicsWidget *parent)
        : Plasma::WidgetExplorer(parent)
    {
        connect(this, SIGNAL(closeClicked()), this, SLOT(deleteLater()));
        m_svg = new Plasma::FrameSvg(this);
        m_svg->setImagePath("widgets/frame");
        m_svg->setElementPrefix("raised");
        m_svg->setEnabledBorders(Plasma::FrameSvg::TopBorder);
    }

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event)
    {
        m_svg->resizeFrame(event->newSize());
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)
        m_svg->paintFrame(painter);
    }

private:
    Plasma::FrameSvg *m_svg;
};

SaverView::SaverView(Plasma::Containment *containment, QWidget *parent)
    : Plasma::View(containment, parent),
      m_suppressShow(false),
      m_setupMode(false),
      m_init(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    if (!PlasmaApp::hasComposite()) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);
    }

    setWallpaperEnabled(!PlasmaApp::hasComposite());
    installEventFilter(this);
}

SaverView::~SaverView()
{
    delete m_widgetExplorer.data();
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

void SaverView::drawBackground(QPainter *painter, const QRectF & rect)
{
    if (PlasmaApp::hasComposite()) {
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(rect, Qt::transparent);
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
        delete m_widgetExplorer.data();
    } else {
        ScreenSaverWidgetExplorer *widgetExplorer = new ScreenSaverWidgetExplorer(c);
        widgetExplorer->installEventFilter(this);
        widgetExplorer->setContainment(c);
        widgetExplorer->setLocation(Plasma::BottomEdge);
        widgetExplorer->setIconSize(KIconLoader::SizeHuge);
        widgetExplorer->populateWidgetList();
        widgetExplorer->setMaximumWidth(width());
        widgetExplorer->adjustSize();
        widgetExplorer->setZValue(1000000);
        widgetExplorer->resize(width(), widgetExplorer->size().height());
        widgetExplorer->setPos(0, containment()->geometry().height() - widgetExplorer->geometry().height());
        m_widgetExplorer = widgetExplorer;
    }
}

void SaverView::hideWidgetExplorer()
{
    delete m_widgetExplorer.data();
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
    if (containment() && (watched == (QObject*)m_widgetExplorer.data()) &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        Plasma::WidgetExplorer *widgetExplorer = m_widgetExplorer.data();
        widgetExplorer->setPos(0, containment()->geometry().height() - widgetExplorer->geometry().height());
    }

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
        m_widgetExplorer.data()->setContainment(newContainment);
    }

    View::setContainment(newContainment);
}

void SaverView::hideView()
{
    if (isHidden()) {
        return;
    }

    hideWidgetExplorer();

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

void SaverView::setOpacity(qreal opacity)
{
    setWindowOpacity(opacity);
}

void SaverView::openToolBox()
{
    kDebug() << "close toolbox";
    containment()->openToolBox();
}

void SaverView::closeToolBox()
{
    kDebug() << "close toolbox";
    containment()->closeToolBox();
}

void SaverView::adjustSize(int screen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    int thisScreen = desktop->screenNumber(this);
    if(screen == thisScreen)
    {
        setGeometry(desktop->screenGeometry(screen));
    }
}

#include "saverview.moc"
