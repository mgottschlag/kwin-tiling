 /*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "panelcontroller.h"

#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>

#include <KIcon>
#include <KColorUtils>
#include <KWindowSystem>

#include <plasma/theme.h>
#include <plasma/containment.h>


class PanelController::Private
{
public:
    Private(PanelController *panelControl)
       : q(panelControl),
         containment(0),
         orientation(Qt::Horizontal),
         location(Plasma::BottomEdge),
         extLayout(0),
         layout(0),
         isDragging(false),
         startDragPos(0,0)
    {
    }

    QString buttonStyleSheet()
    {
         const QColor textColor( Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
         const QColor backgroundColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
         const QColor mixedColor(KColorUtils::mix(textColor, backgroundColor, 0.6));

         return QString("QToolButton {\
             border: 2px solid " + mixedColor.name() +
             "; border-radius: 3px;\
             color: " + textColor.name() +
             "; background-color: " + backgroundColor.name() +"}"+
             "QToolButton:hover {\
               background-color: " + mixedColor.name() + "}");
    }

    QToolButton *addTool(const QString icon, const QString iconText)
    {
        QToolButton *tool = new QToolButton(q);

        tool->setIcon(KIcon(icon));
        tool->setText(iconText);
        tool->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tool->setStyleSheet(buttonStyleSheet());

        if (layout) {
            layout->addWidget(tool);
        }

        return tool;
    }

    void resizePanelHeight(const int newHeight)
    {
        if (!containment) {
            return;
        }

        switch (location) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            containment->resize(QSize(newHeight, (int)containment->size().height()));
            containment->setMinimumSize(QSize(newHeight, (int)containment->minimumSize().height()));
            containment->setMaximumSize(QSize(newHeight, (int)containment->maximumSize().height()));
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
        default:
            containment->resize(QSize((int)containment->size().width(), newHeight));
            containment->setMinimumSize(QSize((int)containment->minimumSize().width(), newHeight));
            containment->setMaximumSize(QSize((int)containment->maximumSize().width(), newHeight));
            break;
       }
    }

    PanelController *q;
    Plasma::Containment *containment;
    Qt::Orientation orientation;
    Plasma::Location location;
    QBoxLayout *extLayout;
    QBoxLayout *layout;
    bool isDragging;
    QPoint startDragPos;

    class ResizeHandle;
    ResizeHandle *panelHeightHandle;

    static const int minimumHeight = 10;
};


class PanelController::Private::ResizeHandle: public QWidget
{
public:
    ResizeHandle(QWidget *parent)
       : QWidget(parent)
    {
        setCursor(Qt::SizeVerCursor);
        setMinimumHeight(4);
        setMaximumHeight(4);
    }

    void paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        QColor backColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        backColor.setAlphaF(0.50);
        painter.fillRect(event->rect(), backColor);
    }
};

PanelController::PanelController(QWidget* parent)
   : QWidget(parent),
     d(new Private(this))
{
    // KWin setup
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::Sticky);
    KWindowSystem::setOnAllDesktops(winId(), true);

    //Resize handles
    d->panelHeightHandle = new Private::ResizeHandle(this);

    //layout setup
    d->extLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    d->extLayout->setContentsMargins(0, 1, 0, 0);
    setLayout(d->extLayout);
    d->extLayout->addWidget(d->panelHeightHandle);
    //d->extLayout->addStretch();

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    d->layout->setContentsMargins(4, 4, 4, 4);
    d->layout->setSpacing(4);
    d->layout->addStretch();
    d->extLayout->addItem(d->layout);

    //Add buttons
    QToolButton *addWidgetTool = d->addTool("list-add", i18n("Add Widgets"));
    connect(addWidgetTool, SIGNAL(clicked()), this, SIGNAL(showAddWidgets()));
    connect(addWidgetTool, SIGNAL(clicked()), this, SLOT(hideController()));

    QToolButton *removePanelTool = d->addTool("list-remove", i18n("Remove this panel"));
    connect(removePanelTool, SIGNAL(clicked()), this, SIGNAL(removePanel()));
    connect(removePanelTool, SIGNAL(clicked()), this, SLOT(hideController()));
}

PanelController::~PanelController()
{
    delete d;
}

void PanelController::setContainment( Plasma::Containment *containment)
{
    d->containment = containment;
}

QSize PanelController::sizeHint() const
{
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        return QSize(QWidget::sizeHint().width(), screenGeom.height());
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        return QSize(screenGeom.width(), QWidget::sizeHint().height());
        break;
    }
}

QPoint PanelController::positionForPanelGeometry(const QRect &panelGeom) const
{
    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (d->location) {
    case Plasma::LeftEdge:
        return QPoint(panelGeom.right(), screenGeom.top());
        break;
    case Plasma::RightEdge:
        return QPoint(panelGeom.left() - width(), screenGeom.top());
        break;
    case Plasma::TopEdge:
        return QPoint(screenGeom.left(), panelGeom.bottom());
        break;
    case Plasma::BottomEdge:
    default:
        return QPoint(screenGeom.left(), panelGeom.top() - height());
        break;
    }
}

void PanelController::setLocation(const Plasma::Location &loc)
{
    if (d->location == loc) {
        return;
    }

    d->location = loc;

    switch (loc) {
    case Plasma::LeftEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        d->extLayout->setDirection(QBoxLayout::RightToLeft);
        d->panelHeightHandle->setCursor(Qt::SizeHorCursor);
        break;
    case Plasma::RightEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        d->extLayout->setDirection(QBoxLayout::LeftToRight);
        d->panelHeightHandle->setCursor(Qt::SizeHorCursor);
        break;
    case Plasma::TopEdge:
        d->layout->setDirection(QBoxLayout::LeftToRight);
        d->extLayout->setDirection(QBoxLayout::BottomToTop);
        d->panelHeightHandle->setCursor(Qt::SizeVerCursor);
        break;
    case Plasma::BottomEdge:
    default:
        d->layout->setDirection(QBoxLayout::LeftToRight);
        d->extLayout->setDirection(QBoxLayout::TopToBottom);
        d->panelHeightHandle->setCursor(Qt::SizeVerCursor);
        break;
    }
}

Plasma::Location PanelController::location() const
{
    return d->location;
}

void PanelController::hideController()
{
    hide();
}

void PanelController::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );
    QColor backColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    backColor.setAlphaF(0.75);
    painter.fillRect(event->rect(), backColor);
}

void PanelController::mousePressEvent(QMouseEvent *event)
{
    if (d->panelHeightHandle->geometry().contains( event->pos() ) ) {
        d->startDragPos = event->pos();
        d->isDragging = true;
    }

    QWidget::mousePressEvent(event);
}

void PanelController::mouseReleaseEvent(QMouseEvent *event)
{
    d->startDragPos = QPoint(0, 0);
    d->isDragging = false;
}

void PanelController::mouseMoveEvent(QMouseEvent *event)
{
    if (!d->isDragging || !d->containment) {
        return;
    }

    QRect screenGeom =
    QApplication::desktop()->screenGeometry(d->containment->screen());

    switch (location()) {
    case Plasma::LeftEdge:
        if (mapToGlobal(event->pos()).x() -
            d->startDragPos.x() - width() - d->minimumHeight <
            screenGeom.left()) {
            move(mapToGlobal(event->pos()).x() - d->startDragPos.x(), pos().y());
            d->resizePanelHeight(geometry().left() - screenGeom.left());
        }
        break;
    case Plasma::RightEdge:
        if (mapToGlobal(event->pos()).x() -
            d->startDragPos.x() + width() + d->minimumHeight <
            screenGeom.right()) {
            move(mapToGlobal(event->pos()).x() - d->startDragPos.x(), pos().y());
            d->resizePanelHeight(screenGeom.right() - geometry().right());
        }
        break;
    case Plasma::TopEdge:
        if (mapToGlobal(event->pos()).y() -
            d->startDragPos.y() - height() - d->minimumHeight >
            screenGeom.top()) {
            move(pos().x(), mapToGlobal(event->pos()).y() - d->startDragPos.y());
            d->resizePanelHeight(geometry().top() - screenGeom.top());
        }
        break;
    case Plasma::BottomEdge:
    default:
        if (mapToGlobal(event->pos()).y() -
            d->startDragPos.y() + height() + d->minimumHeight <
            screenGeom.bottom()) {
            move(pos().x(), mapToGlobal(event->pos()).y() - d->startDragPos.y());
            d->resizePanelHeight(screenGeom.bottom() - geometry().bottom());
        }
        break;
    }
}

#include "panelcontroller.moc"
