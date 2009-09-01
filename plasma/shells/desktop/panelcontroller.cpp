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

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>

#include <KColorUtils>
#include <KIconLoader>
#include <KIcon>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/PaintUtils>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>

#include "plasmaapp.h"
#include "positioningruler.h"
#include "toolbutton.h"
#include "widgetsExplorer/widgetexplorer.h"

#include <kephal/screens.h>

class PanelController::ButtonGroup: public QFrame
{
public:
    ButtonGroup(QWidget *parent)
       : QFrame(parent)
    {
        background = new Plasma::FrameSvg(this);
        background->setImagePath("widgets/frame");
        background->setElementPrefix("plain");
    }

    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)

        QPainter painter(this);
        background->resizeFrame(size());
        background->paintFrame(&painter);
    }

    Plasma::FrameSvg *background;
};


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
         dragging(NoElement),
         startDragPos(0,0),
         leftAlignTool(0),
         centerAlignTool(0),
         rightAlignTool(0),
         drawMoveHint(false)
    {
    }

    ToolButton *addTool(QAction *action, QWidget *parent, Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon)
    {
        ToolButton *tool = new ToolButton(parent);
        tool->setToolButtonStyle(style);
        tool->setAction(action);
        actionWidgets.append(tool);

        return tool;
    }

    ToolButton *addTool(const QString iconName, const QString iconText, QWidget *parent, Qt::ToolButtonStyle style = Qt::ToolButtonTextBesideIcon, bool checkButton = false)
    {
        //TODO take advantage of setDefaultAction using the containment's actions if possible
        ToolButton *tool = new ToolButton(parent);

        KIcon icon = KIcon(iconName);
        if (!icon.isNull() && !iconName.isNull()) {
            tool->setIcon(icon);
        }

        tool->setText(iconText);
        tool->setToolButtonStyle(style);

        if (style == Qt::ToolButtonIconOnly) {
            tool->setToolTip(iconText);
        }

        tool->setCheckable(checkButton);
        tool->setAutoExclusive(checkButton);

        return tool;
    }

    void resizeFrameHeight(const int newHeight)
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

    void rulersMoved(int offset, int minLength, int maxLength)
    {
         if (!containment) {
            return;
         }

         QSize preferredSize(containment->preferredSize().toSize());

         switch (location) {
         case Plasma::LeftEdge:
         case Plasma::RightEdge:
             containment->resize(QSize((int)containment->size().width(), qBound(minLength, preferredSize.height(), maxLength)));
             containment->setMinimumSize(QSize((int)containment->minimumSize().width(), minLength));
             containment->setMaximumSize(QSize((int)containment->maximumSize().width(), maxLength));
             break;
         case Plasma::TopEdge:
         case Plasma::BottomEdge:
         default:
             containment->resize(QSize(qBound(minLength, preferredSize.width(), maxLength), (int)containment->size().height()));
             containment->setMinimumSize(QSize(minLength, (int)containment->minimumSize().height()));
             containment->setMaximumSize(QSize(maxLength, (int)containment->maximumSize().height()));
             break;
        }

        emit q->offsetChanged(offset);
    }

    void alignToggled(bool toggle)
    {
        if (!toggle) {
            return;
        }

        if (q->sender() == leftAlignTool) {
            emit q->alignmentChanged(Qt::AlignLeft);
            ruler->setAlignment(Qt::AlignLeft);
        } else if (q->sender() == centerAlignTool) {
            emit q->alignmentChanged(Qt::AlignCenter);
            ruler->setAlignment(Qt::AlignCenter);
        } else if (q->sender() == rightAlignTool) {
            emit q->alignmentChanged(Qt::AlignRight);
            ruler->setAlignment(Qt::AlignRight);
        }

        emit q->offsetChanged(0);
        ruler->setOffset(0);
    }

    void panelVisibilityModeChanged(bool toggle)
    {
        if (!toggle) {
            return;
        }

        if (q->sender() == normalPanelTool) {
            emit q->panelVisibilityModeChanged(PanelView::NormalPanel);
        } else if (q->sender() == autoHideTool) {
            emit q->panelVisibilityModeChanged(PanelView::AutoHide);
        } else if (q->sender() == underWindowsTool) {
            emit q->panelVisibilityModeChanged(PanelView::LetWindowsCover);
        } else if (q->sender() == overWindowsTool) {
            emit q->panelVisibilityModeChanged(PanelView::WindowsGoBelow);
        }
    }

    void settingsPopup()
    {
        if (optionsDialog->isVisible()) {
            optionsDialog->hide();
        } else {
            KWindowSystem::setState(optionsDialog->winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky);
            QPoint pos = q->mapToGlobal(settingsTool->pos());
            optionsDialog->layout()->activate();
            optionsDialog->resize(optionsDialog->sizeHint());
            QSize s = optionsDialog->size();

            switch (location) {
                case Plasma::BottomEdge:
                pos = QPoint(pos.x(), pos.y() - s.height());
                break;
            case Plasma::TopEdge:
                pos = QPoint(pos.x(), pos.y() + settingsTool->size().height());
                break;
            case Plasma::LeftEdge:
                pos = QPoint(pos.x() + settingsTool->size().width(), pos.y());
                break;
            case Plasma::RightEdge:
                pos = QPoint(pos.x() - s.width(), pos.y());
                break;
            default:
                if (pos.y() - s.height() > 0) {
                    pos = QPoint(pos.x(), pos.y() - s.height());
                } else {
                    pos = QPoint(pos.x(), pos.y() + settingsTool->size().height());
                }
            }

            QRect screenRect = Kephal::ScreenUtils::screenGeometry(containment->screen());

            if (pos.rx() + s.width() > screenRect.right()) {
                pos.rx() -= ((pos.rx() + s.width()) - screenRect.right());
            }

            if (pos.ry() + s.height() > screenRect.bottom()) {
                pos.ry() -= ((pos.ry() + s.height()) - screenRect.bottom());
            }

            pos.rx() = qMax(0, pos.rx());
            optionsDialog->move(pos);
            optionsDialog->show();
        }
    }

    void syncRuler()
    {
        QRect screenGeom = Kephal::ScreenUtils::screenGeometry(containment->screen());

        switch (location) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            ruler->setAvailableLength(screenGeom.height());
            ruler->setMaxLength(qMin((int)containment->maximumSize().height(), screenGeom.height()));
            ruler->setMinLength(containment->minimumSize().height());
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
        default:
            ruler->setAvailableLength(screenGeom.width());
            ruler->setMaxLength(qMin((int)containment->maximumSize().width(), screenGeom.width()));
            ruler->setMinLength(containment->minimumSize().width());
            break;
        }
    }

    void maximizePanel()
    {
        const int length = ruler->availableLength();
        const int screen = containment->screen();
        const QRect screenGeom = PlasmaApp::self()->corona()->screenGeometry(screen);
        QRegion availGeom(screenGeom);
        foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
            if (view->containment() != containment &&
                view->screen() == screen && view->visibilityMode() == PanelView::NormalPanel) {
                availGeom = availGeom.subtracted(view->geometry());
            }
        }
        int offset = 0;
        const int w = containment->size().width();
        const int h = containment->size().height();

        switch (location) {
            case Plasma::LeftEdge: {
                QRect r = availGeom.intersected(QRect(0, 0, w, length)).boundingRect();
                offset = r.top();
            }
            break;

            case Plasma::RightEdge: {
                QRect r = availGeom.intersected(QRect(screenGeom.right() - w, 0, w, length)).boundingRect();
                offset = r.top();
            }
            break;

            case Plasma::TopEdge: {
                QRect r = availGeom.intersected(QRect(0, 0, length, h)).boundingRect();
                offset = r.left();
            }
            break;

            case Plasma::BottomEdge:
            default: {
                QRect r = availGeom.intersected(QRect(0, screenGeom.bottom() - h, length, h)).boundingRect();
                offset = r.left();
            }
            break;
        }

        rulersMoved(offset, length, length);
        ruler->setMaxLength(length);
        ruler->setMinLength(length);
    }

    void addSpace()
    {
        Plasma::Applet *spacer = containment->addApplet("panelspacer_internal");
        if (spacer) {
           QMetaObject::invokeMethod(spacer, "updateConfigurationMode", Q_ARG(bool, true));
        }
    }

     enum DragElement { NoElement = 0,
                        ResizeButtonElement,
                        MoveButtonElement
                      };

    PanelController *q;
    Plasma::Containment *containment;
    Qt::Orientation orientation;
    Plasma::Location location;
    QBoxLayout *extLayout;
    QBoxLayout *layout;
    QLabel *alignLabel;
    QLabel *modeLabel;
    DragElement dragging;
    QPoint startDragPos;
    Plasma::FrameSvg *background;
    Plasma::Dialog *optionsDialog;
    QBoxLayout *optDialogLayout;
    ToolButton *settingsTool;
    Plasma::Svg *iconSvg;

    ToolButton *moveTool;
    ToolButton *sizeTool;

    //Alignment buttons
    ToolButton *leftAlignTool;
    ToolButton *centerAlignTool;
    ToolButton *rightAlignTool;

    //Panel mode buttons
    ToolButton *normalPanelTool;
    ToolButton *autoHideTool;
    ToolButton *underWindowsTool;
    ToolButton *overWindowsTool;
    ToolButton *expandTool;

    //Widgets for actions
    QList<QWidget *> actionWidgets;

    PositioningRuler *ruler;

    bool drawMoveHint;

    static const int minimumHeight = 10;
};

PanelController::PanelController(QWidget* parent)
   : QWidget(0),
     d(new Private(this)),
     m_widgetExplorerView(0),
     m_widgetExplorer(0)
{
    Q_UNUSED(parent)

    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), Qt::transparent);
    setPalette(pal);

    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setContainsMultipleImages(true);

    d->iconSvg = new Plasma::Svg(this);
    d->iconSvg->setImagePath("widgets/configuration-icons");
    d->iconSvg->setContainsMultipleImages(true);
    d->iconSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    //setWindowFlags(Qt::Popup);
    setWindowFlags(Qt::FramelessWindowHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky);
    setAttribute(Qt::WA_DeleteOnClose);
    setFocus(Qt::ActiveWindowFocusReason);

    //layout setup
    m_mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_configWidget = new QWidget(this);
    m_mainLayout->addWidget(m_configWidget);

    d->extLayout = new QBoxLayout(QBoxLayout::TopToBottom, m_configWidget);
    setLayout(d->extLayout);

    d->background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
    d->extLayout->setContentsMargins(0, d->background->marginSize(Plasma::TopMargin), 0, 0);

    d->layout = new QBoxLayout(QBoxLayout::LeftToRight);
    d->layout->setContentsMargins(4, 4, 4, 4);
    if (QApplication::layoutDirection() == Qt::RightToLeft) {
        d->layout->setDirection(QBoxLayout::RightToLeft);
    } else {
        d->layout->setDirection(QBoxLayout::LeftToRight);
    }
    d->layout->setSpacing(4);

    d->layout->addStretch();
    d->extLayout->addLayout(d->layout);

    //Add buttons

    //alignment
    //first the container
    QFrame *alignFrame = new ButtonGroup(m_configWidget);
    QVBoxLayout *alignLayout = new QVBoxLayout(alignFrame);

    d->alignLabel = new QLabel(i18n("Panel Alignment"), m_configWidget);
    alignLayout->addWidget(d->alignLabel);

    d->leftAlignTool = d->addTool("format-justify-left", i18n("Left"), alignFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->leftAlignTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    alignLayout->addWidget(d->leftAlignTool);
    d->leftAlignTool->setChecked(true);
    connect(d->leftAlignTool, SIGNAL(toggled(bool)), this, SLOT(alignToggled(bool)));

    d->centerAlignTool = d->addTool("format-justify-center", i18n("Center"), alignFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->centerAlignTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    alignLayout->addWidget(d->centerAlignTool);
    connect(d->centerAlignTool, SIGNAL(clicked(bool)), this, SLOT(alignToggled(bool)));

    d->rightAlignTool = d->addTool("format-justify-right", i18n("Right"), alignFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->rightAlignTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    alignLayout->addWidget(d->rightAlignTool);
    connect(d->rightAlignTool, SIGNAL(clicked(bool)), this, SLOT(alignToggled(bool)));


    //Panel mode
    //first the container
    QFrame *modeFrame = new ButtonGroup(m_configWidget);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeFrame);

    d->modeLabel = new QLabel(i18n("Visibility"), m_configWidget);
    modeLayout->addWidget(d->modeLabel);

    d->normalPanelTool = d->addTool("checkmark", i18n("Always visible"), modeFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->normalPanelTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeLayout->addWidget(d->normalPanelTool);
    connect(d->normalPanelTool, SIGNAL(toggled(bool)), this, SLOT(panelVisibilityModeChanged(bool)));

    d->autoHideTool = d->addTool("video-display", i18n("Auto-hide"), modeFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->autoHideTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeLayout->addWidget(d->autoHideTool);
    connect(d->autoHideTool, SIGNAL(toggled(bool)), this, SLOT(panelVisibilityModeChanged(bool)));

    d->underWindowsTool = d->addTool("view-fullscreen", i18n("Windows can cover"), modeFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->underWindowsTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeLayout->addWidget(d->underWindowsTool);
    connect(d->underWindowsTool, SIGNAL(toggled(bool)), this, SLOT(panelVisibilityModeChanged(bool)));

    d->overWindowsTool = d->addTool("view-restore", i18n("Windows go below"), modeFrame,  Qt::ToolButtonTextBesideIcon, true);
    d->overWindowsTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    modeLayout->addWidget(d->overWindowsTool);
    connect(d->overWindowsTool, SIGNAL(toggled(bool)), this, SLOT(panelVisibilityModeChanged(bool)));

    d->layout->addStretch();
    d->moveTool = d->addTool(QString(), i18n("Screen Edge"), m_configWidget);
    d->moveTool->setIcon(d->iconSvg->pixmap("move"));
    d->moveTool->installEventFilter(this);
    d->moveTool->setCursor(Qt::SizeAllCursor);
    d->layout->addWidget(d->moveTool);

    d->sizeTool = d->addTool(QString(), i18n("Height"), m_configWidget);
    d->sizeTool->installEventFilter(this);
    d->sizeTool->setCursor(Qt::SizeVerCursor);
    d->layout->addWidget(d->sizeTool);
    d->layout->addStretch();

    //other buttons
    d->layout->addSpacing(20);

    //Settings popup menu
    d->settingsTool = d->addTool("configure", i18n("More Settings"), m_configWidget);
    d->layout->addWidget(d->settingsTool);
    connect(d->settingsTool, SIGNAL(pressed()), this, SLOT(settingsPopup()));
    d->optionsDialog = new Plasma::Dialog(0); // don't pass in a parent; breaks with some lesser WMs
    d->optionsDialog->installEventFilter(this);
    KWindowSystem::setState(d->optionsDialog->winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky);
    d->optDialogLayout = new QVBoxLayout(d->optionsDialog);
    d->optDialogLayout->setMargin(0);
    d->optDialogLayout->addWidget(alignFrame);
    d->optDialogLayout->addWidget(modeFrame);


    d->expandTool = d->addTool(QString(), i18n("Maximize Panel"), m_configWidget);
    d->expandTool->setIcon(d->iconSvg->pixmap("size-horizontal"));
    d->expandTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->optDialogLayout->addWidget(d->expandTool);
    connect(d->expandTool, SIGNAL(clicked()), this, SLOT(maximizePanel()));

    ToolButton *closeControllerTool = d->addTool("window-close", i18n("Close this configuration window"), m_configWidget, Qt::ToolButtonIconOnly, false);
    d->layout->addWidget(closeControllerTool);
    connect(closeControllerTool, SIGNAL(clicked()), this, SLOT(close()));

    d->ruler = new PositioningRuler(m_configWidget);
    connect(d->ruler, SIGNAL(rulersMoved(int, int, int)), this, SLOT(rulersMoved(int, int, int)));
    d->extLayout->addWidget(d->ruler);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(themeChanged()));
    themeChanged();
}

PanelController::~PanelController()
{
    //TODO: should we try and only call this when something has actually been
    //      altered that we care about?
    PlasmaApp::self()->corona()->requestConfigSync();
    delete m_widgetExplorer;
    delete m_widgetExplorerView;
    delete d->optionsDialog;
    d->optionsDialog = 0;
    delete d;
}

void PanelController::setContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    if (d->containment) {
        disconnect(d->containment, 0, this, 0);
    }
    

    d->containment = containment;

    if(m_widgetExplorer) {
        m_widgetExplorer->setContainment(d->containment);
    }

    QWidget *child;
    while (!d->actionWidgets.isEmpty()) {
        child = d->actionWidgets.first();
        //try to remove from both layouts
        d->layout->removeWidget(child);
        d->optDialogLayout->removeWidget(child);
        d->actionWidgets.removeFirst();
        child->deleteLater();
    }

    int insertIndex = d->layout->count() - 3;

    QAction *action = containment->action("add widgets");
    if (action && action->isEnabled()) {
        ToolButton *addWidgetTool = d->addTool(action, this);
        d->layout->insertWidget(insertIndex, addWidgetTool);
        ++insertIndex;
        connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showWidgetsExplorer()));
    }

    action = new QAction(i18n("Add Spacer"), this);
    ToolButton *addSpaceTool = d->addTool(action, this);
    d->layout->insertWidget(insertIndex, addSpaceTool);
    ++insertIndex;
    connect(action, SIGNAL(triggered()), this, SLOT(addSpace()));

    action = containment->action("lock widgets");
    if (action && action->isEnabled()) {
        ToolButton *lockWidgetsTool = d->addTool(action, this);
        d->layout->insertWidget(insertIndex, lockWidgetsTool);
        ++insertIndex;
        connect(lockWidgetsTool, SIGNAL(clicked()), this, SLOT(hide()));
    }

    action = containment->action("remove");
    if (action && action->isEnabled()) {
        ToolButton *removePanelTool = d->addTool(action, this);
        removePanelTool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        d->optDialogLayout->insertWidget(insertIndex, removePanelTool);
        connect(removePanelTool, SIGNAL(clicked()), this, SLOT(hide()));
    }

    d->syncRuler();
}

QSize PanelController::sizeHint() const
{
    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(d->containment->screen());

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
    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(d->containment->screen());

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
    d->ruler->setLocation(loc);
    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(d->containment->screen());

    switch (loc) {
    case Plasma::LeftEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        //The external layout gwts auto flipped when QApplication::layoutDirection() changes
        //and it shouldn't, the internal one no and it should, so i must manually invert both
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->extLayout->setDirection(QBoxLayout::LeftToRight);
        } else {
            d->extLayout->setDirection(QBoxLayout::RightToLeft);
        }
        d->background->setEnabledBorders(Plasma::FrameSvg::RightBorder);
        d->extLayout->setContentsMargins(0, 0, d->background->marginSize(Plasma::RightMargin), 0);

        break;
    case Plasma::RightEdge:
        d->layout->setDirection(QBoxLayout::TopToBottom);
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->extLayout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->extLayout->setDirection(QBoxLayout::LeftToRight);
        }
        d->background->setEnabledBorders(Plasma::FrameSvg::LeftBorder);
        d->extLayout->setContentsMargins(d->background->marginSize(Plasma::LeftMargin), 0, 0, 0);

        break;
    case Plasma::TopEdge:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->layout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->layout->setDirection(QBoxLayout::LeftToRight);
        }
        d->extLayout->setDirection(QBoxLayout::BottomToTop);
        d->background->setEnabledBorders(Plasma::FrameSvg::BottomBorder);
        d->extLayout->setContentsMargins(0, 0, 0, d->background->marginSize(Plasma::BottomMargin));

        break;
    case Plasma::BottomEdge:
    default:
        if (QApplication::layoutDirection() == Qt::RightToLeft) {
            d->layout->setDirection(QBoxLayout::RightToLeft);
        } else {
            d->layout->setDirection(QBoxLayout::LeftToRight);
        }
        d->extLayout->setDirection(QBoxLayout::TopToBottom);
        d->background->setEnabledBorders(Plasma::FrameSvg::TopBorder);
        d->extLayout->setContentsMargins(0, d->background->marginSize(Plasma::TopMargin), 0, 0);

        break;
    }

    switch (loc) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        d->sizeTool->setCursor(Qt::SizeHorCursor);
        d->sizeTool->setText(i18n("Width"));
        d->sizeTool->setIcon(d->iconSvg->pixmap("size-horizontal"));
        d->expandTool->setIcon(d->iconSvg->pixmap("size-vertical"));
        d->leftAlignTool->setText(i18n("Top"));
        d->rightAlignTool->setText(i18n("Bottom"));

        d->ruler->setAvailableLength(screenGeom.height());

        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        d->sizeTool->setCursor(Qt::SizeVerCursor);
        d->sizeTool->setText(i18n("Height"));
        d->sizeTool->setIcon(d->iconSvg->pixmap("size-vertical"));
        d->expandTool->setIcon(d->iconSvg->pixmap("size-horizontal"));
        d->leftAlignTool->setText(i18n("Left"));
        d->rightAlignTool->setText(i18n("Right"));

        d->ruler->setAvailableLength(screenGeom.width());
    }

    d->ruler->setMaximumSize(d->ruler->sizeHint());
    d->syncRuler();

    if(m_widgetExplorer) {
        switch (loc) {
        case Plasma::LeftEdge:
        case Plasma::RightEdge:
            m_widgetExplorer->setOrientation(Qt::Vertical);
            break;
        case Plasma::TopEdge:
        case Plasma::BottomEdge:
            m_widgetExplorer->setOrientation(Qt::Horizontal);
            break;
        }
    }

    Plasma::WindowEffects::slideWindow(this, loc);
}

Plasma::Location PanelController::location() const
{
    return d->location;
}

void PanelController::setOffset(int newOffset)
{
    if (newOffset != d->ruler->offset()) {
        d->ruler->setOffset(newOffset);
    }
}

int PanelController::offset() const
{
    return d->ruler->offset();
}

void PanelController::setAlignment(const Qt::Alignment &newAlignment)
{
    if (newAlignment != d->ruler->alignment()) {
        if (newAlignment == Qt::AlignLeft) {
            d->leftAlignTool->setChecked(true);
        } else if (newAlignment == Qt::AlignCenter) {
            d->centerAlignTool->setChecked(true);
        } else if (newAlignment == Qt::AlignRight) {
            d->rightAlignTool->setChecked(true);
        }

        d->ruler->setAlignment(newAlignment);
    }
}

Qt::Alignment PanelController::alignment() const
{
    return d->ruler->alignment();
}

void PanelController::setVisibilityMode(PanelView::VisibilityMode mode)
{
    switch (mode) {
    case PanelView::AutoHide:
        d->autoHideTool->setChecked(true);
        break;
    case PanelView::LetWindowsCover:
        d->underWindowsTool->setChecked(true);
        break;
    case PanelView::WindowsGoBelow:
        d->overWindowsTool->setChecked(true);
        break;
    case PanelView::NormalPanel:
    default:
        d->normalPanelTool->setChecked(true);
        break;
    }
}

PanelView::VisibilityMode PanelController::panelVisibilityMode() const
{
    if (d->underWindowsTool->isChecked()) {
        return PanelView::LetWindowsCover;
    } else if (d->overWindowsTool->isChecked()) {
        return PanelView::WindowsGoBelow;
    } else if (d->autoHideTool->isChecked()) {
        return PanelView::AutoHide;
    } else {
        return PanelView::NormalPanel;
    }
}

bool PanelController::isHorizontal() const
{
    return d->location == Plasma::TopEdge || d->location == Plasma::BottomEdge;
}

void PanelController::themeChanged()
{
    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette p = d->alignLabel->palette();
    p.setColor(QPalette::Normal, QPalette::WindowText, color);
    p.setColor(QPalette::Inactive, QPalette::WindowText, color);
    d->alignLabel->setPalette(p);
    d->modeLabel->setPalette(p);

    d->sizeTool->setIcon(d->iconSvg->pixmap("move"));

    if (isHorizontal()) {
        d->sizeTool->setIcon(d->iconSvg->pixmap("size-vertical"));
    } else {
        d->sizeTool->setIcon(d->iconSvg->pixmap("size-horizontal"));
    }
}

void PanelController::showWidgetsExplorer()
{
    Qt::Orientation widgetExplorerOrientation = isHorizontal() ? Qt::Horizontal : Qt::Vertical;
  
    if (!d->containment) {
        return;
    }
    
    m_configWidget->hide();

    if (!m_widgetExplorerView) {

        m_widgetExplorerView = new QGraphicsView(this);
        m_widgetExplorerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_widgetExplorerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_widgetExplorerView->setStyleSheet("background: transparent; border: none;");

        m_widgetExplorerView->setScene(d->containment->corona());
        m_widgetExplorerView->installEventFilter(this);
        m_mainLayout->addWidget(m_widgetExplorerView);
    }

    if (!m_widgetExplorer) {
        m_widgetExplorer = new Plasma::WidgetExplorer();
        m_widgetExplorer->setContainment(d->containment);
        m_widgetExplorer->setCorona(d->containment->corona());
        m_widgetExplorer->setApplication();

        m_widgetExplorer->resize(size());
        d->containment->corona()->addOffscreenWidget(m_widgetExplorer);

        m_widgetExplorerView->setSceneRect(m_widgetExplorer->geometry());

        m_widgetExplorer->installEventFilter(this);
    }

    if (m_widgetExplorer->orientation() != widgetExplorerOrientation) {
        m_widgetExplorer->setOrientation(widgetExplorerOrientation);
    }

    m_widgetExplorer->show();
    // connect signals
}

void PanelController::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source );

    d->background->resizeFrame(size());
    d->background->paintFrame(&painter);
}

void PanelController::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

bool PanelController::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->optionsDialog && event->type() == QEvent::WindowDeactivate
            && (!m_widgetExplorerView || !m_widgetExplorerView->isVisible())) {
        if (!d->settingsTool->underMouse()) {
            d->optionsDialog->hide();
        }
        if (!isActiveWindow()) {
            close();
        }
        return true;
    } else if (watched == m_widgetExplorerView && event->type() == QEvent::WindowDeactivate
               && (!m_configWidget|| !m_configWidget->isVisible())) {
        if (!d->settingsTool->underMouse()) {
            d->optionsDialog->hide();
        }
        if (!isActiveWindow()) {
            close();
        }
        return true;

    } else if (watched == d->moveTool) {
        if (event->type() == QEvent::MouseButtonPress) {
            d->dragging = Private::MoveButtonElement;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            d->dragging = Private::NoElement;
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseMoveFilter(mouseEvent);
        }
    } else if (watched == d->sizeTool) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            d->startDragPos = mouseEvent->pos();
            d->dragging = Private::ResizeButtonElement;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            //resets properties saved during the drag
            d->startDragPos = QPoint(0, 0);
            d->dragging = Private::NoElement;
            setCursor(Qt::ArrowCursor);
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseMoveFilter(mouseEvent);
        }
    }
    
    //if widgetsExplorer moves or resizes, then the view has to adjust
    if ((watched == (QObject*)m_widgetExplorer) && (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        m_widgetExplorerView->setSceneRect(m_widgetExplorer->geometry());
    } 
     
    //if the view resizes, then the widgetexplorer has to be resized
    if (watched == m_widgetExplorerView && event->type() == QEvent::Resize) { 
          m_widgetExplorer->resize(m_widgetExplorerView->geometry().size());
    }

    return false;
}

void PanelController::mouseMoveFilter(QMouseEvent *event)
{
    if (d->dragging == Private::NoElement || !d->containment) {
        return;
    }

    QRect screenGeom = Kephal::ScreenUtils::screenGeometry(d->containment->screen());

    if (d->dragging == Private::MoveButtonElement) {
        //only move when the mouse cursor is out of the controller to avoid an endless reposition cycle
        if (geometry().contains(event->globalPos())) {
            return;
        }

        if (!screenGeom.contains(event->globalPos())) {
            //move panel to new screen if dragged there
            int targetScreen = Kephal::ScreenUtils::screenId(event->globalPos());
            //kDebug() << "Moving panel from screen" << d->containment->screen() << "to screen" << targetScreen;
            d->containment->setScreen(targetScreen);
            return;
        }

        //create a dead zone so you can go across the middle without having it hop to one side
        float dzFactor = 0.35;
        QPoint offset = QPoint(screenGeom.width()*dzFactor,screenGeom.height()*dzFactor);
        QRect deadzone = QRect(screenGeom.topLeft()+offset, screenGeom.bottomRight()-offset);
        if (deadzone.contains(event->globalPos())) {
            //kDebug() << "In the deadzone:" << deadzone;
            return;
        }

        const Plasma::Location oldLocation = d->containment->location();
        Plasma::Location newLocation = oldLocation;
        float screenAspect = float(screenGeom.height())/screenGeom.width();

        /* Use diagonal lines so we get predictable behavior when moving the panel
         * y=topleft.y+(x-topleft.x)*aspectratio   topright < bottomleft
         * y=bottomleft.y-(x-topleft.x)*aspectratio   topleft < bottomright
         */
        if (event->globalY() < screenGeom.y()+(event->globalX()-screenGeom.x())*screenAspect) {
            if (event->globalY() < screenGeom.bottomLeft().y()-(event->globalX()-screenGeom.x())*screenAspect) {
                if (d->containment->location() == Plasma::TopEdge) {
                    return;
                } else {
                    newLocation = Plasma::TopEdge;
                }
            } else if (d->containment->location() == Plasma::RightEdge) {
                    return;
            } else {
                newLocation = Plasma::RightEdge;
            }
        } else {
            if (event->globalY() < screenGeom.bottomLeft().y()-(event->globalX()-screenGeom.x())*screenAspect) {
                if (d->containment->location() == Plasma::LeftEdge) {
                    return;
                } else {
                    newLocation = Plasma::LeftEdge;
                }
            } else if(d->containment->location() == Plasma::BottomEdge) {
                    return;
            } else {
                newLocation = Plasma::BottomEdge;
            }
        }


        //If the orientation changed swap width and height
        if (oldLocation != newLocation) {
            emit locationChanged(newLocation);
        }

        return;
    }

    //Resize handle moved
    switch (location()) {
    case Plasma::LeftEdge: {
        int newX = mapToGlobal(event->pos()).x() - d->startDragPos.x();
        if (newX - d->minimumHeight > screenGeom.left() &&
            newX - screenGeom.left() <= screenGeom.width()/3) {
            move(newX, pos().y());
            d->resizeFrameHeight(geometry().left() - screenGeom.left());
        }
        break;
    }
    case Plasma::RightEdge: {
        int newX = mapToGlobal(event->pos()).x() - d->startDragPos.x();
        if (newX + width() + d->minimumHeight < screenGeom.right() &&
            newX + width() - screenGeom.left() >= 2*(screenGeom.width()/3)) {
            move(newX, pos().y());
            d->resizeFrameHeight(screenGeom.right() - geometry().right());
        }
        break;
    }
    case Plasma::TopEdge: {
        int newY = mapToGlobal(event->pos()).y() - d->startDragPos.y();
        if ( newY - d->minimumHeight > screenGeom.top() &&
             newY - screenGeom.top()<= screenGeom.height()/3) {
            move(pos().x(), newY);
            d->resizeFrameHeight(geometry().top() - screenGeom.top());
        }
        break;
    }
    case Plasma::BottomEdge:
    default: {
        int newY = mapToGlobal(event->pos()).y() - d->startDragPos.y();
        if ( newY + height() + d->minimumHeight < screenGeom.bottom() &&
             newY + height() - screenGeom.top() >= 2*(screenGeom.height()/3)) {
            move(pos().x(), newY);
            d->resizeFrameHeight(screenGeom.bottom() - geometry().bottom());
        }
        break;
    }
    }
}

void PanelController::focusOutEvent(QFocusEvent * event)
{
    Q_UNUSED(event)
    if (!d->optionsDialog->isActiveWindow()) {
        //qDebug() << "m_widgetExplorerView" << (void*) m_widgetExplorerView;
        if(m_widgetExplorerView) {
            if(!m_widgetExplorerView->isVisible()) {
                d->optionsDialog->hide();
                close();
            }
        } else {
            d->optionsDialog->hide();
            close();
        }
    }
}

#include "panelcontroller.moc"
