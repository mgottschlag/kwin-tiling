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

#include <KIconLoader>
#include <KRun>
#include <KStandardDirs>

#include <Plasma/Corona>
#include <Plasma/Theme>

//AppletToolTipWidget

AppletToolTipWidget::AppletToolTipWidget(Plasma::Location location, QWidget *parent)
        : Plasma::Dialog(parent),
          m_widget(new AppletInfoWidget()),
          m_direction(Plasma::locationToDirection(location))
{
    setAcceptDrops(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
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
    m_widget->setApplet(applet);
}

AppletIconWidget *AppletToolTipWidget::appletIconWidget() const
{
    return m_widget->applet();
}

void AppletToolTipWidget::updateContent()
{
    m_widget->updateInfo();
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

AppletInfoWidget::AppletInfoWidget(QGraphicsWidget *parent)
        : QGraphicsWidget(parent),
          m_applet(0)
{
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
    connect(m_aboutLabel, SIGNAL(linkActivated(QString)), this, SLOT(openLink(QString)));

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
    m_iconWidget->setAcceptedMouseButtons(Qt::NoButton);
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

void AppletInfoWidget::setApplet(AppletIconWidget *applet)
{
    m_applet = applet;
    updateInfo();
}

AppletIconWidget *AppletInfoWidget::applet() const
{
    return m_applet;
}

void AppletInfoWidget::updateInfo()
{
    PlasmaAppletItem *appletItem = m_applet ? m_applet->appletItem() : 0;
    if (appletItem) {
        m_iconWidget->setIcon(appletItem->icon());
        m_nameLabel->setText(appletItem->name());
        m_versionLabel->setText(i18n("Version %1", appletItem->version()));
        QString description = "<html><body>";
        if (!appletItem->description().isEmpty()) {
            description += appletItem->description() + "<br/><br/>";
        }

        const QString color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name();
        if (!appletItem->author().isEmpty()) {
            description += i18n("<font color=\"%1\">Author:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" +
                           appletItem->author();
            if (!appletItem->email().isEmpty()) {
                 description += " <a href=\"mailto:" + appletItem->email() + "\">" +
                                appletItem->email() + "</a>";
            }
            description += "</div>";
        }
        if (!appletItem->website().isEmpty()) {
            description += i18n("<font color=\"%1\">Website:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" + "<a href=\"" +
                           appletItem->website() + "\">" + appletItem->website() +
                           "</a></div>";
        }
        description += i18n("<font color=\"%1\">License:</font>", color) + "<div style=\"margin-left: 15px; color:%1\">" +
                       appletItem->license() + "</div>";
        description += "</body></html>";
        description = description.arg(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name());
        //kDebug() << description;

        m_aboutLabel->setText(description);
        m_uninstallButton->setVisible(appletItem->isLocal());
    } else {
        m_iconWidget->setIcon("plasma");
        m_nameLabel->setText(i18n("Unknown Applet"));
        m_uninstallButton->setVisible(false);
    }

    adjustSize();
}

void AppletToolTipWidget::resizeEvent(QResizeEvent *event)
{
    Plasma::Dialog::resizeEvent(event);

    if (!isVisible()) {
        return;
    }

    //offsets to stop tooltips from jumping when they resize
    int deltaX = 0;
    int deltaY = 0;
    if (m_direction == Plasma::Up) {
        deltaY = event->oldSize().height() - event->size().height();
    } else if (m_direction == Plasma::Left) {
        deltaX = event->oldSize().width() - event->size().width();
    }

    // resize then move if we're getting smaller, vice versa when getting bigger
    // this prevents overlap with the item in the smaller case, and a repaint of
    // the tipped item when getting bigger
    move(x() + deltaX, y() + deltaY);
}

void AppletInfoWidget::uninstall()
{
    if (!m_applet) {
        return;
    }

    PlasmaAppletItem *appletItem = m_applet->appletItem();
    if (!appletItem) {
        return;
    }

    Plasma::PackageStructure installer;
    installer.uninstallPackage(appletItem->pluginName(),
                               KStandardDirs::locateLocal("data", "plasma/plasmoids/"));
    PlasmaAppletItemModel *model = appletItem->appletItemModel();
    model->takeRow(model->indexFromItem(appletItem).row());
}

void AppletInfoWidget::openLink(const QString &link)
{
    new KRun(link, 0);
}

