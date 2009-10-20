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


#include "deviceitem.h"

//Qt
#include <QtGui/QGraphicsOpacityEffect>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QLabel>

//KDE
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KStandardDirs>
#include <KDesktopFile>
#include <KGlobalSettings>
#include <kcapacitybar.h>
#include <kdesktopfileactions.h>

//Plasma
#include <Plasma/PaintUtils>
#include <Plasma/IconWidget>
#include <Plasma/BusyWidget>
#include <Plasma/ItemBackground>
#include <Plasma/Label>
#include <plasma/animations/fade.h>

//Own
#include "notifierdialog.h"

using namespace Notifier;

DeviceItem::DeviceItem(const QString &udi, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_udi(udi),
      m_hovered(false),
      m_mounted(false)
{
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(0);
    setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_treeLayout = new QGraphicsLinearLayout(Qt::Vertical, this);

    m_mainLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_treeLayout->addItem(m_mainLayout);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_actionsWidget = new QGraphicsWidget(this);

    m_actionsLayout = new QGraphicsLinearLayout(Qt::Vertical, m_actionsWidget);
    m_actionsLayout->setContentsMargins(30, 0, 0, 0);
    m_actionsWidget->hide();

    m_deviceIcon = new Plasma::IconWidget(this);
    m_deviceIcon->setAcceptHoverEvents(false);
    m_deviceIcon->setContentsMargins(0, 0, 0, 0);
    m_deviceIcon->setMinimumSize(m_deviceIcon->sizeFromIconSize(KIconLoader::SizeMedium));
    m_deviceIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    m_deviceIcon->setAcceptedMouseButtons(Qt::NoButton);
    m_deviceIcon->setFocusPolicy(Qt::NoFocus);

    QGraphicsLinearLayout *info_layout = new QGraphicsLinearLayout(Qt::Vertical);
    info_layout->setContentsMargins(0, 0, 0, 0);
    info_layout->setSpacing(0);
    m_nameLabel = new Plasma::Label(this);
    m_nameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_nameLabel->setPreferredWidth(0);
    m_nameLabel->nativeWidget()->setWordWrap(false);

    m_descriptionLabel = new Plasma::Label(this);
    m_descriptionLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_descriptionLabel->setPreferredWidth(0);
    m_descriptionLabel->nativeWidget()->setWordWrap(false);
    m_descriptionLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QFont font = m_descriptionLabel->font();
    font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    font.setItalic(true);
    m_descriptionLabel->setFont(font);
    m_descriptionLabel->setGraphicsEffect(new QGraphicsOpacityEffect(m_descriptionLabel));
    updateColors();

    m_descriptionLabel->hide();

    KCapacityBar *capacityBarWidget = new KCapacityBar(KCapacityBar::DrawTextInline);
    m_capacityBar = new QGraphicsProxyWidget(this);
    m_capacityBar->setWidget(capacityBarWidget);
    capacityBarWidget->setAttribute(Qt::WA_TranslucentBackground);
    capacityBarWidget->setContinuous(true);
    m_capacityBar->setAcceptHoverEvents(false);
    m_capacityBar->setMaximumHeight(12);
    m_capacityBar->setGraphicsEffect(new QGraphicsOpacityEffect(m_capacityBar));

    m_capacityBar->hide();

    info_layout->addItem(m_nameLabel);
    info_layout->addItem(m_descriptionLabel);
    info_layout->addItem(m_capacityBar);

    m_leftActionIcon = new Plasma::IconWidget(this);
    m_leftActionIcon->hide();
    m_leftActionIcon->setMaximumSize(m_leftActionIcon->sizeFromIconSize(LEFTACTION_SIZE));
    m_leftActionIcon->setSizePolicy(QSizePolicy::Fixed,  QSizePolicy::Fixed);
    m_leftActionIcon->setGraphicsEffect(new QGraphicsOpacityEffect(m_leftActionIcon));
    connect(m_leftActionIcon, SIGNAL(clicked()), this, SLOT(leftActionClicked()));

    m_mainLayout->addItem(m_deviceIcon);
    m_mainLayout->setAlignment(m_deviceIcon, Qt::AlignVCenter);
    m_mainLayout->addItem(info_layout);
    m_mainLayout->setAlignment(info_layout, Qt::AlignVCenter);
    m_mainLayout->addItem(m_leftActionIcon);
    m_mainLayout->setAlignment(m_leftActionIcon, Qt::AlignVCenter);

    m_busyWidget = new Plasma::BusyWidget(this);
    m_busyWidget->setMaximumSize(LEFTACTION_SIZE, LEFTACTION_SIZE);
    m_busyWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_busyWidget->hide();
    m_busyWidgetTimer.setSingleShot(true);
    connect(&m_busyWidgetTimer, SIGNAL(timeout()), this,  SLOT(triggerBusyWidget()));

    setLeftAction(Nothing);
}

DeviceItem::~DeviceItem()
{
}

void DeviceItem::collapse()
{
    if (!isCollapsed()) {
        updateHoverDisplay();

        m_treeLayout->removeAt(1);
        m_actionsWidget->hide();
    }
}

void DeviceItem::expand()
{
    if (isCollapsed()) {
        m_treeLayout->addItem(m_actionsWidget);
        m_actionsWidget->show();
        updateHoverDisplay();

        adjustSize();
    }
}

void DeviceItem::addAction(const QString &action)
{
    for (int i = 0; i < m_actionsLayout->count(); ++i) {
        QGraphicsLayoutItem *item = m_actionsLayout->itemAt(i);
        if (item->graphicsItem()->data(NotifierDialog::ActionRole).toString() == action) {
            return;
        }
    }

    Plasma::IconWidget *actionItem = new Plasma::IconWidget(m_actionsWidget);
    actionItem->installEventFilter(this);
    actionItem->setContentsMargins(3, 3, 3, 3);
    actionItem->setData(NotifierDialog::ActionRole, action);

    QString actionUrl = KStandardDirs::locate("data", "solid/actions/" + action);
    QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(actionUrl, true);

    QColor color;
    color.setAlpha(255);
    actionItem->setTextBackgroundColor(color);
    actionItem->setText(services[0].text());
    actionItem->setIcon(services[0].icon());
    actionItem->setOrientation(Qt::Horizontal);
    actionItem->setPreferredHeight(KIconLoader::SizeMedium+3+3);
    actionItem->setPreferredWidth(0);
    actionItem->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_actionsLayout->addItem(actionItem);
}

QString DeviceItem::udi() const
{
    return data(NotifierDialog::SolidUdiRole).toString();
}

QString DeviceItem::name() const
{
    return data(Qt::DisplayRole).toString();
}

QString DeviceItem::description() const
{
    return data(NotifierDialog::DescriptionRole).toString();
}

QIcon DeviceItem::icon() const
{
    return data(Qt::DecorationRole).value<QIcon>();
}

QString DeviceItem::iconName() const
{
    return data(NotifierDialog::IconNameRole).toString();
}

bool DeviceItem::isCollapsed() const
{
    return (m_treeLayout->count() == 1);
}

bool DeviceItem::isMounted() const
{
    return m_mounted;
}

bool DeviceItem::hovered() const
{
    return m_hovered;
}

void DeviceItem::setMounted(const bool mounted)
{
    m_mounted = mounted;
    if (m_mounted) {
        m_leftActionIcon->setToolTip(i18n("Click to safely remove this device from the computer."));
        m_deviceIcon->setToolTip(i18n("Device is plugged in and can be accessed by applications."));
    } else {
        m_leftActionIcon->setToolTip(i18n("Click to access this device from other applications."));
        m_deviceIcon->setToolTip(i18n("Device is plugged in, but not mounted for access yet."));

    }

    if (m_mounted && m_leftAction == Mount) {
        setLeftAction(Umount);
    } else if (!m_mounted && m_leftAction == Umount) {
        setLeftAction(Mount);
    }

    const bool barVisible = m_capacityBar->isVisible();
    m_capacityBar->setVisible(m_mounted && m_hovered);
    if (!barVisible && m_capacityBar->isVisible()) {
        // work around for a QGraphicsLayout bug when used with proxy widgets
        m_mainLayout->invalidate();
    }
}

void DeviceItem::setLeftAction(DeviceItem::LeftActions action)
{
    m_leftAction = action;
    if (m_leftAction == DeviceItem::Umount) {
        m_leftActionIcon->setIcon("media-eject");
    } else if (m_leftAction == Mount) {
        m_leftActionIcon->setIcon("emblem-mounted");
    } else {
        m_leftActionIcon->setIcon("");
    }
}

DeviceItem::LeftActions DeviceItem::leftAction()
{
    return m_leftAction;
}

void DeviceItem::setHovered(const bool hovered)
{
    if (hovered == m_hovered) {
        return;
    }

    m_hovered = hovered;
    if (hovered) {
        updateHoverDisplay();
    } else {
        Plasma::FadeAnimation *labelFade = new Plasma::FadeAnimation(0);
        Plasma::FadeAnimation *barFade = new Plasma::FadeAnimation(0);
        Plasma::FadeAnimation *iconFade = new Plasma::FadeAnimation(0);

        labelFade->setWidget(m_descriptionLabel);
        barFade->setWidget(m_capacityBar);
        iconFade->setWidget(m_leftActionIcon);

        labelFade->start();
        barFade->start();
        iconFade->start();
    }
}

void DeviceItem::updateHoverDisplay()
{
    const bool labelVisible = m_descriptionLabel->isVisible();
    const bool barVisible = m_capacityBar->isVisible();
    const bool makeVisible = m_hovered || !isCollapsed();

    m_leftActionIcon->setVisible(makeVisible && !m_busyWidget->isVisible());
    m_descriptionLabel->setVisible(makeVisible);
    m_capacityBar->setVisible(m_mounted && makeVisible);
    if (labelVisible != m_descriptionLabel->isVisible() ||
        barVisible != m_capacityBar->isVisible()) {
        // work around for a QGraphicsLayout bug when used with proxy widgets
        m_mainLayout->invalidate();
    }
}

void DeviceItem::setHoverDisplayOpacity(qreal opacity)
{
    if (!hovered()) {
        opacity = 1 - opacity;
    }

    QGraphicsOpacityEffect *labelEffect = dynamic_cast<QGraphicsOpacityEffect *>(m_descriptionLabel->graphicsEffect());
    QGraphicsOpacityEffect *leftActionEffect = dynamic_cast<QGraphicsOpacityEffect *>(m_leftActionIcon->graphicsEffect());
    QGraphicsOpacityEffect *capacityBarEffect = dynamic_cast<QGraphicsOpacityEffect *>(m_capacityBar->graphicsEffect());

    if (labelEffect && leftActionEffect && capacityBarEffect) {
        labelEffect->setOpacity(opacity);
        leftActionEffect->setOpacity(opacity);
        capacityBarEffect->setOpacity(opacity);
    }

    if (!hovered() && opacity == 0) {
        updateHoverDisplay();
    }
}

void DeviceItem::setFreeSpace(qulonglong freeSpace, qulonglong size)
{
    qulonglong usedSpace = size - freeSpace;
    KCapacityBar *capacityBarWidget = static_cast<KCapacityBar*>(m_capacityBar->widget());
    capacityBarWidget->setText(i18nc("@info:status Free disk space", "%1 free", KGlobal::locale()->formatByteSize(freeSpace)));
    capacityBarWidget->setValue(size > 0 ? (usedSpace * 100) / size : 0);
}

void DeviceItem::leftActionClicked()
{
    emit leftActionActivated(this);
}

void DeviceItem::setData(int key, const QVariant & value)
{
    QGraphicsItem::setData(key, value);
    switch (key) {
        case Qt::DecorationRole:
            m_icon = value.value<QIcon>();
            m_deviceIcon->setIcon(m_icon);
            break;
        case Qt::DisplayRole:
            m_nameLabel->setText(value.toString());
            m_nameLabel->setMinimumWidth(50);
            break;
        case NotifierDialog::DescriptionRole:
            m_descriptionLabel->setText(value.toString());
            m_descriptionLabel->setMinimumWidth(50);
            m_descriptionLabel->setPreferredWidth(50);
            break;
    }
}

void DeviceItem::setBusy()
{
    if (m_busyWidgetTimer.isActive()) {
        return;
    }
    m_busyWidgetTimer.start(300);
}

void DeviceItem::setReady()
{
    if (m_busyWidgetTimer.isActive()) {
        m_busyWidgetTimer.stop();
    }

    if (m_busyWidget->isVisible()) {
        m_busyWidget->hide();
        m_mainLayout->removeItem(m_busyWidget);
        m_mainLayout->addItem(m_leftActionIcon);
        m_mainLayout->setAlignment(m_leftActionIcon, Qt::AlignVCenter);
        m_leftActionIcon->setVisible(m_hovered || !isCollapsed());
    }
}

void DeviceItem::triggerBusyWidget()
{
    m_mainLayout->removeItem(m_leftActionIcon);
    m_leftActionIcon->hide();
    m_mainLayout->addItem(m_busyWidget);
    m_mainLayout->setAlignment(m_busyWidget, Qt::AlignVCenter);
    m_busyWidget->show();
}

bool DeviceItem::eventFilter(QObject* obj, QEvent *event)
{
    Plasma::IconWidget *item = dynamic_cast<Plasma::IconWidget *>(obj);
    if (item) {
        switch (event->type()) {
            case QEvent::GraphicsSceneHoverLeave:
                emit highlightActionItem(0);
                break;
            case QEvent::GraphicsSceneHoverEnter:
                emit highlightActionItem(item);
                break;
            case QEvent::GraphicsSceneMousePress: {
                QGraphicsSceneMouseEvent *e = static_cast<QGraphicsSceneMouseEvent *>(event);
                if (e->button() == Qt::LeftButton) {
                    actionClicked(item);
                    return true;
                }
                break;
                }
            default:
                break;
        }
    }

    return false;
}

bool DeviceItem::selectNextAction(Plasma::IconWidget *currentAction)
{
    if (!currentAction) {
        emit highlightActionItem(dynamic_cast<QGraphicsItem*>(m_actionsLayout->itemAt(0)));
        return true;
    } else {
        int i=0;
        while (m_actionsLayout->itemAt(i) != currentAction) {
            i++;
        }
        if (m_actionsLayout->count() > i+1) {
            emit highlightActionItem(dynamic_cast<QGraphicsItem*>(m_actionsLayout->itemAt(i+1)));
            return true;
        }
    }
    emit highlightActionItem(0);
    return false;
}

bool DeviceItem::selectPreviousAction(Plasma::IconWidget *currentAction , bool forceLast)
{
    if (forceLast) {
        emit highlightActionItem(dynamic_cast<QGraphicsItem*>(m_actionsLayout->itemAt(m_actionsLayout->count()-1)));
        return true;
    }
    if (!currentAction) {
        emit highlightActionItem(0);
        return false;
    } else {
        int i=0;
        while (m_actionsLayout->itemAt(i) != currentAction) {
            i++;
        }
        if (i > 0) {
            emit highlightActionItem(dynamic_cast<QGraphicsItem*>(m_actionsLayout->itemAt(i-1)));
            return true;
        }
    }
    emit highlightActionItem(0);
    return true;
}


void DeviceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void DeviceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !boundingRect().contains(event->pos())) {
        return;
    }
    clicked();
}

void DeviceItem::clicked()
{
     if (m_actionsLayout->count() == 1) {
        emit actionActivated(this, udi(), m_actionsLayout->itemAt(0)->graphicsItem()->data(NotifierDialog::ActionRole).toString());
    } else {
        if (isCollapsed()) {
            expand();
            emit activated(this);
        } else {
            collapse();
        }
    }
}


void DeviceItem::actionClicked(Plasma::IconWidget* item)
{
    QString action = item->data(NotifierDialog::ActionRole).toString();
    emit actionActivated(this, udi(), action);
}


void DeviceItem::updateColors()
{
    QPalette p = m_descriptionLabel->nativeWidget()->palette();
    QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    color.setAlphaF(0.6);
    p.setColor(QPalette::Normal, QPalette::WindowText, color);
    p.setColor(QPalette::Inactive, QPalette::WindowText, color);
    m_descriptionLabel->nativeWidget()->setPalette(p);
    m_descriptionLabel->update();
}

#include "deviceitem.moc"
