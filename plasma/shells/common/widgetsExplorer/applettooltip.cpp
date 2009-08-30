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

//AppletToolTipWidget

AppletToolTipWidget::AppletToolTipWidget(QWidget *parent, AppletIconWidget *applet)
        : Plasma::Dialog(parent)
{
    m_applet = applet;
    m_widget = new AppletInfoWidget();
    QGraphicsScene * scene = new QGraphicsScene();
    scene->addItem(m_widget);
    if(m_applet) {
        m_widget->setAppletItem(m_applet->appletItem());
    }
    setGraphicsWidget(m_widget);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
}

AppletToolTipWidget::~AppletToolTipWidget()
{
}

void AppletToolTipWidget::setAppletIconWidget(AppletIconWidget *applet)
{
    m_applet = applet;
    m_widget->setAppletItem(m_applet->appletItem());
}

void AppletToolTipWidget::updateContent()
{
    m_widget->updateInfo();
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
    m_mainLayout = new QGraphicsLinearLayout();
    m_mainLayout->setOrientation(Qt::Vertical);

    m_iconWidget   = new Plasma::IconWidget();
    m_nameLabel    = new Plasma::Label();
    m_tabs         = new Plasma::TabBar();

    m_aboutLabel   = new Plasma::Label();
    m_actionsLabel = new Plasma::Label();
    m_detailsLabel = new Plasma::Label();

    // main layout init
    QGraphicsLinearLayout * headerLayout = new QGraphicsLinearLayout();
    headerLayout->setOrientation(Qt::Horizontal);
    headerLayout->addItem(m_iconWidget);
    headerLayout->addItem(m_nameLabel);

    headerLayout->setAlignment(m_iconWidget, Qt::AlignHCenter);

    m_mainLayout->addItem(headerLayout);
    m_mainLayout->addItem(m_tabs);

    m_mainLayout->setContentsMargins(10, 10, 10, 0);

    m_tabs->setPreferredSize(250, 150);

    // header init
    m_iconWidget->setAcceptHoverEvents(false);
    m_iconWidget->setAcceptedMouseButtons(false);
    m_iconWidget->setMinimumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
    m_iconWidget->setMaximumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));

    QFont font = m_nameLabel->nativeWidget()->font();
    font.setBold(true);
    font.setPointSize(1.2 * font.pointSize());
    m_nameLabel->nativeWidget()->setFont(font);
    m_nameLabel->nativeWidget()->setScaledContents(true);
    m_nameLabel->setMaximumHeight(m_iconWidget->maximumHeight());

    // about tab
    m_tabs->addTab(i18n("About"), m_aboutLabel);
    font.setBold(false);
    m_aboutLabel->setFont(font);
    m_aboutLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // actions tab
    m_tabs->addTab(i18n("Actions"), m_actionsLabel);

    // author tab
    m_tabs->addTab(i18n("Details"), m_detailsLabel);
    m_detailsLabel->nativeWidget()->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    setLayout(m_mainLayout);
}

void AppletInfoWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
}

void AppletInfoWidget::updateInfo()
{
    if(m_appletItem != 0) {
        m_iconWidget->setIcon(m_appletItem->icon());
        m_nameLabel->setText(m_appletItem->name());

        m_aboutLabel->setText(
            m_appletItem->description());

        m_detailsLabel->setText(
            i18n("<html><p>Version: %4</p><p>Author: %1 (%2)</p><p>License: %3</p></html>")
                .arg(m_appletItem->author())
                .arg(m_appletItem->email())
                .arg(m_appletItem->license())
                .arg(m_appletItem->version())
                );
    } else {
        m_iconWidget->setIcon("plasma");
        m_nameLabel->setText("Unknown applet");
    }

    m_mainLayout->invalidate();
    m_mainLayout->activate();

    QSizeF prefSize = m_mainLayout->sizeHint(Qt::PreferredSize) + QSizeF(32, 32);
    resize(prefSize);
}
