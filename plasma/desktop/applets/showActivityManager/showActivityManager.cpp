/***************************************************************************
 *   Copyright (C) 2011 Aaron Seigo                                        *
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

#include "showActivityManager.h"

#include <QGraphicsLinearLayout>
#include <QDBusConnection>
#include <QDBusMessage>

#include <KIcon>
#include <KShortcut>

#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>

ShowActivityManager::ShowActivityManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args)
{
    setAspectRatioMode(Plasma::ConstrainedSquare);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setSvg("widgets/activities");
    connect(m_icon, SIGNAL(clicked()), this, SLOT(showManager()));

    layout->addItem(m_icon);
    configChanged();
}

void ShowActivityManager::configChanged()
{
    KShortcut shortcut = globalShortcut();
    QString text;

    if (shortcut.isEmpty()) {
        text = i18n("Click to show the activity manager");
    } else if (shortcut == KShortcut(Qt::Key_Meta + Qt::Key_Q)) {
        text = i18n("Click or press the Meta key and 'Q' to show the activity manager");
    } else {
        text = i18n("Click or press the associated keyboard shortcut (%1) to show the activity manager", shortcut.toString());
    }

    Plasma::ToolTipContent content(i18n("Show Activity Manager"), text, KIcon("preferences-activities"));
    Plasma::ToolTipManager::self()->setContent(this, content);
}

void ShowActivityManager::showManager()
{
    Plasma::ToolTipManager::self()->hide(this);
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.plasma-desktop",
                                                          "/App",
                                                          QString(),
                                                          "toggleActivityManager");
    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

K_EXPORT_PLASMA_APPLET(showactivitymanager, ShowActivityManager)

#include "showActivityManager.moc"

