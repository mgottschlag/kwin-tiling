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
#include <KStandardDirs>

#include <Plasma/Corona>
#include <Plasma/Theme>

//AppletToolTipWidget

AppletToolTipWidget::AppletToolTipWidget(QWidget *parent, AppletIconWidget *applet)
        : Plasma::Dialog(parent)
{
    setAcceptDrops(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    m_applet = applet;
    m_widget = new AppletInfoWidget();
    if (m_applet) {
        m_widget->setAppletItem(m_applet->appletItem());
    }
}

AppletToolTipWidget::~AppletToolTipWidget()
{
    if (m_widget->scene()) {
        Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(m_widget->scene());
        if (corona) {
            corona->removeOffscreenWidget(m_widget);
        }

        m_widget->scene()->removeItem(m_widget);
    }

    delete m_widget;
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

void AppletToolTipWidget::dragEnterEvent(QDragEnterEvent *event)
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
    m_nameLabel = new Plasma::Label(this);
    m_versionLabel = new Plasma::Label(this);
    m_aboutLabel = new Plasma::Label(this);

    m_uninstallButton = new Plasma::PushButton(this);
    m_uninstallButton->setText(i18n("Uninstall Widget"));
    m_uninstallButton->setIcon(KIcon("application-exit"));
    m_uninstallButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum, QSizePolicy::ButtonBox);
    m_uninstallButton->setVisible(false);
    connect(m_uninstallButton, SIGNAL(clicked()), this, SLOT(uninstall()));

    // layout init
    QGraphicsLinearLayout *textLayout = new QGraphicsLinearLayout(Qt::Vertical);
    textLayout->addItem(m_nameLabel);
    textLayout->addItem(m_versionLabel);
    textLayout->addStretch();

    QGraphicsLinearLayout *headerLayout = new QGraphicsLinearLayout();
    headerLayout->addItem(m_iconWidget);
    headerLayout->addItem(textLayout);
    headerLayout->setAlignment(m_iconWidget, Qt::AlignVCenter);

    m_mainVerticalLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_mainVerticalLayout->addItem(headerLayout);
    m_mainVerticalLayout->addItem(m_aboutLabel);
    m_mainVerticalLayout->addItem(m_uninstallButton);

    // header init
    m_iconWidget->setAcceptHoverEvents(false);
    m_iconWidget->setAcceptedMouseButtons(false);
    m_iconWidget->setMinimumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
    m_iconWidget->setMaximumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));

    QFont font = m_versionLabel->nativeWidget()->font();
    font.setBold(true);
    font.setPointSizeF(0.9 * font.pointSizeF());
    m_versionLabel->setFont(font);
    font = m_nameLabel->nativeWidget()->font();
    font.setBold(true);
    font.setPointSizeF(1.2 * font.pointSizeF());
    m_nameLabel->setFont(font);

    setLayout(m_mainVerticalLayout);
}

void AppletInfoWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
    updateInfo();
}

void AppletInfoWidget::updateInfo()
{
    if (m_appletItem) {
        m_iconWidget->setIcon(m_appletItem->icon());
        m_nameLabel->setText(m_appletItem->name());
        m_versionLabel->setText(i18n("Version %1", m_appletItem->version()));
        QString description = "<html><body>";
        if (!m_appletItem->description().isEmpty()) {
            description += m_appletItem->description() + "<br/><br/>";
        }

        const QString color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name();
        if (!m_appletItem->author().isEmpty()) {
            description += i18n("<font color=\"%1\">Author:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" +
                           m_appletItem->author();
            if (!m_appletItem->email().isEmpty()) {
                 description += " <a href=\"mailto:" + m_appletItem->email() + "\">" +
                                m_appletItem->email() + "</a>";
            }
            description += "</div>";
        }
        if (!m_appletItem->website().isEmpty()) {
            description += i18n("<font color=\"%1\">Website:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" + "<a href=\"" +
                           m_appletItem->website() + "\">" + m_appletItem->website() +
                           "</a></div>";
        }
        description += i18n("<font color=\"%1\">License:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" +
                       m_appletItem->license() + "</div>";
        description += "</body></html>";
        description = description.arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name());
        kDebug() << description;

        m_aboutLabel->setText(description);
        m_uninstallButton->setVisible(m_appletItem->isLocal());
    } else {
        m_iconWidget->setIcon("plasma");
        m_nameLabel->setText(i18n("Unknown Applet"));
        m_uninstallButton->setVisible(false);
    }

    adjustSize();
}

void AppletInfoWidget::uninstall()
{
    if (!m_appletItem) {
        return;
    }

    Plasma::PackageStructure installer;
    installer.uninstallPackage(m_appletItem->pluginName(),
                               KStandardDirs::locateLocal("data", "plasma/plasmoids/"));
    PlasmaAppletItemModel *model = m_appletItem->appletItemModel();
    model->takeRow(model->indexFromItem(m_appletItem).row());
    delete m_appletItem;
    m_appletItem = 0;
    hide();
}

