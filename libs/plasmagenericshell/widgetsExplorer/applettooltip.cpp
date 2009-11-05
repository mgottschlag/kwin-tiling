/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "applettooltip.h"

#include <kiconloader.h>
#include <kpushbutton.h>
#include <ktextbrowser.h>

#include <Plasma/Corona>
#include <Plasma/Theme>

//AppletToolTipWidget

AppletToolTipWidget::AppletToolTipWidget(QWidget *parent, AppletIconWidget *applet)
        : Plasma::Dialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    m_applet = applet;
    m_widget = new AppletInfoWidget();
    if (m_applet) {
        m_widget->setAppletItem(m_applet->appletItem());
    }
}

AppletToolTipWidget::~AppletToolTipWidget()
{
}

void AppletToolTipWidget::setScene(QGraphicsScene *scene)
{
    if (scene) {
        Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(scene);
        if (corona) {
            corona->addOffscreenWidget(m_widget);
        } else {
            scene->addItem(m_widget);
        }

        setGraphicsWidget(m_widget);
    }
}

void AppletToolTipWidget::setAppletIconWidget(AppletIconWidget *applet)
{
    m_applet = applet;
    m_widget->setAppletItem(m_applet->appletItem());
}

void AppletToolTipWidget::updateContent()
{
    if(m_widget != 0) {
        m_widget->updateInfo();
    }
}

AppletIconWidget *AppletToolTipWidget::appletIconWidget()
{
    return m_applet;
}

void AppletToolTipWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit(enter());
}

void AppletToolTipWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit(leave());
}

//AppletInfoWidget

AppletInfoWidget::AppletInfoWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
        : QGraphicsWidget(parent)
{
    m_appletItem = appletItem;
    init();
}

AppletInfoWidget::~AppletInfoWidget()
{
}

void AppletInfoWidget::init()
{
    m_iconWidget = new Plasma::IconWidget(this);
    m_nameLabel = new Plasma::TextBrowser(this);
    m_aboutLabel = new Plasma::TextBrowser(this);

    m_uninstallButton = new Plasma::PushButton(this);
    m_uninstallButton->setText(i18n("Uninstall Widget"));
    m_uninstallButton->setIcon(KIcon("application-exit"));
    m_uninstallButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum, QSizePolicy::ButtonBox);
    m_uninstallButton->setVisible(false);

    // layout init
    QGraphicsLinearLayout *textLayout = new QGraphicsLinearLayout(Qt::Vertical);
    textLayout->addItem(m_nameLabel);
    textLayout->addItem(m_aboutLabel);

    m_mainLayout = new QGraphicsLinearLayout();
    m_mainLayout->addItem(m_iconWidget);
    m_mainLayout->addItem(textLayout);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setAlignment(m_iconWidget, Qt::AlignVCenter);

    m_mainVerticalLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainVerticalLayout->addItem(m_mainLayout);
    m_mainVerticalLayout->addItem(m_uninstallButton);
    m_mainVerticalLayout->setAlignment(m_uninstallButton, Qt::AlignBottom);

    // header init
    m_iconWidget->setAcceptHoverEvents(false);
    m_iconWidget->setAcceptedMouseButtons(false);
    m_iconWidget->setMinimumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
    m_iconWidget->setMaximumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));

    QFont font = m_nameLabel->nativeWidget()->font();
    font.setBold(true);
    font.setPointSize(1.2 * font.pointSize());
    m_nameLabel->setFont(font);
    m_nameLabel->nativeWidget()->setFixedHeight(m_iconWidget->maximumHeight());
    m_nameLabel->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_nameLabel->nativeWidget()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_nameLabel->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // about label
    font.setBold(false);
    m_aboutLabel->setFont(font);
    m_aboutLabel->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aboutLabel->nativeWidget()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_aboutLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_aboutLabel->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    //set palette
    QPalette plasmaPalette = QPalette();
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    plasmaPalette.setColor(QPalette::Text, textColor);
    m_nameLabel->nativeWidget()->setPalette(plasmaPalette);
    m_aboutLabel->nativeWidget()->setPalette(plasmaPalette);

    setLayout(m_mainVerticalLayout);
}

void AppletInfoWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
}

void AppletInfoWidget::updateInfo()
{
    if (m_appletItem) {
        m_iconWidget->setIcon(m_appletItem->icon());
        m_nameLabel->setText(m_appletItem->name());
        m_aboutLabel->setText(m_appletItem->description());
        m_uninstallButton->setVisible(m_appletItem->isLocal());
    } else {
        m_iconWidget->setIcon("plasma");
        m_nameLabel->setText("Unknown Applet");
        m_uninstallButton->setVisible(false);
    }
}
