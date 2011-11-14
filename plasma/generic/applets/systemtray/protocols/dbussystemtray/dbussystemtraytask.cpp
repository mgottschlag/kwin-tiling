/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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

#include "dbussystemtraytask.h"

#include <QAction>
#include <QDir>
#include <QGraphicsWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMovie>
#include <QTimer>
#include <QMetaEnum>

#include <KAction>
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KStandardDirs>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/DataContainer>
#include <Plasma/DataEngineManager>
#include <Plasma/View>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/Plasma>

#include "dbussystemtraywidget.h"

#include <netinet/in.h>

namespace SystemTray
{

DBusSystemTrayTask::DBusSystemTrayTask(const QString &serviceName, Plasma::DataEngine *dataEngine, QObject *parent)
    : Task(parent),
      m_serviceName(serviceName),
      m_typeId(serviceName),
      m_name(serviceName),
      m_movie(0),
      m_blinkTimer(0),
      m_dataEngine(dataEngine),
      m_service(dataEngine->serviceForSource(serviceName)),
      m_blink(false),
      m_valid(false),
      m_embeddable(false)
{
    kDebug();
    m_service->setParent(this);

    //TODO: how to behave if its not m_valid?
    m_valid = !serviceName.isEmpty();

    if (m_valid) {
        //TODO: is this call to dataUpdated required?
        dataUpdated(serviceName, Plasma::DataEngine::Data());
        m_dataEngine->connectSource(serviceName, this);
    }
}

DBusSystemTrayTask::~DBusSystemTrayTask()
{
    delete m_movie;
    delete m_blinkTimer;
}

QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet *host)
{
    kDebug();
    DBusSystemTrayWidget *iconWidget = new DBusSystemTrayWidget(host, m_service);
    // we may already have an icon, so this isn't the first time we've been created,
    // or the request will come delayed after the creation of this widget and
    // the data likely already exists in the DataEngine. we can't wait for an update
    // from the status notifier item since it may never come or only after a while
    // when the item actually changes.
    // the call also needs to be delayed, since createWidget is called before the widget is added
    // to the widgetsByHost() collection, and so an immediate call won't atually work here
    QTimer::singleShot(0, this, SLOT(updateWidgets()));

    iconWidget->show();

    iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    iconWidget->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    //standard fdo icon sizes is 24x24, opposed to the 22x22 SizeSmallMedium
    iconWidget->setPreferredSize(24, 24);
    return iconWidget;
}

void DBusSystemTrayTask::updateWidgets()
{
    if (Plasma::DataContainer *c = m_dataEngine->containerForSource(m_serviceName)) {
        // fairly inneficient as it updates _all_ icons!
        Plasma::DataEngine::Data data = c->data();
        data["IconsChanged"] = true;
        data["StatusChanged"] = true;
        data["ToolTipChanged"] = true;
        dataUpdated(m_serviceName, data);
    }
}

bool DBusSystemTrayTask::isValid() const
{
    return m_valid;
}

bool DBusSystemTrayTask::isEmbeddable() const
{
    return m_embeddable;
}

QString DBusSystemTrayTask::name() const
{
    return m_name;
}

QString DBusSystemTrayTask::typeId() const
{
    return m_typeId;
}

QIcon DBusSystemTrayTask::icon() const
{
    return m_icon;
}

void DBusSystemTrayTask::dataUpdated(const QString &taskName, const Plasma::DataEngine::Data &properties)
{
    Q_UNUSED(taskName);

    const QString oldTypeId = m_typeId;

    QString cat = properties["Category"].toString();
    if (!cat.isEmpty()) {
        int index = metaObject()->indexOfEnumerator("Category");
        int key = metaObject()->enumerator(index).keyToValue(cat.toLatin1());

        if (key != -1) {
            setCategory((Task::Category)key);
        }
    }

    if (properties["TitleChanged"].toBool()) {
        QString m_title = properties["Title"].toString();
        if (!m_title.isEmpty()) {
            m_name = m_title;

            if (m_typeId.isEmpty()) {
                m_typeId = m_title;
            }
        }
    }

    /*
    kDebug() << m_name
    << "status:" << properties["StatusChanged"].toBool() << "title:" <<  properties["TitleChanged"].toBool()
    << "icons:" << properties["IconsChanged"].toBool() << "tooltip:" << properties["ToolTipChanged"].toBool();
    */

    QString id = properties["Id"].toString();
    if (!id.isEmpty()) {
        m_typeId = id;
    }

    if (properties["IconsChanged"].toBool()) {
        syncIcons(properties);
    }

    if (properties["StatusChanged"].toBool()) {
        syncStatus(properties["Status"].toString());
    }

    if (properties["ToolTipChanged"].toBool()) {
        syncToolTip(properties["ToolTipTitle"].toString(),
                    properties["ToolTipSubTitle"].toString(),
                    properties["ToolTipIcon"].value<QIcon>());
    }

    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        DBusSystemTrayWidget *iconWidget = qobject_cast<DBusSystemTrayWidget *>(widget);
        if (iconWidget) {
            iconWidget->setItemIsMenu(properties["ItemIsMenu"].toBool());
        }
    }

    if (m_typeId != oldTypeId) {
        QHashIterator<Plasma::Applet *, QGraphicsWidget *> it(widgetsByHost());
        while (it.hasNext()) {
            it.next();

            Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(it.value());
            if (!icon) {
                continue;
            }

            icon->action()->setObjectName(QString("Systemtray-%1-%2").arg(m_typeId).arg(it.key()->id()));

            KConfigGroup cg = it.key()->config();
            KConfigGroup shortcutsConfig = KConfigGroup(&cg, "Shortcuts");

            //FIXME: quite ugly, checks if the applet is klipper and if is less than 2 widgets have been created. if so, assign a default global shortcut
            QString shortcutText;
            if (it.key()->property("firstRun").toBool() == true && name() == "Klipper" && widgetsByHost().count() < 2) {
                QString file = KStandardDirs::locateLocal("config", "kglobalshortcutsrc");
                KConfig config(file);
                KConfigGroup cg(&config, "klipper");
                QStringList shortcutTextList = cg.readEntry("show_klipper_popup", QStringList());

                if (shortcutTextList.size() >= 2) {
                    shortcutText = shortcutTextList.first();
                    if (shortcutText.isEmpty()) {
                        shortcutText = shortcutTextList[1];
                    }

                    if (shortcutText.isEmpty()) {
                        shortcutText = "Ctrl+Alt+V";
                    }
                } else {
                    shortcutText = "Ctrl+Alt+V";
                }
            }

            shortcutText = shortcutsConfig.readEntryUntranslated(icon->action()->objectName(), shortcutText);
            KAction *action = qobject_cast<KAction *>(icon->action());
            if (action && !shortcutText.isEmpty()) {
                action->setGlobalShortcut(KShortcut(shortcutText),
                            KAction::ShortcutTypes(KAction::ActiveShortcut | KAction::DefaultShortcut),
                            KAction::NoAutoloading);
                shortcutsConfig.writeEntry(icon->action()->objectName(), shortcutText);
            }
        }
    }

    m_embeddable = true;

    if (oldTypeId != m_typeId || properties["StatusChanged"].toBool() || properties["TitleChanged"].toBool()) {
        //kDebug() << "signaling a change";
        emit changed(this);
    }
}

void DBusSystemTrayTask::syncIcons(const Plasma::DataEngine::Data &properties)
{
    m_icon = properties["Icon"].value<QIcon>();
    m_iconName = properties["IconName"].toString();

    if (status() != Task::NeedsAttention) {
        foreach (QGraphicsWidget *widget, widgetsByHost()) {
            DBusSystemTrayWidget *iconWidget = qobject_cast<DBusSystemTrayWidget *>(widget);
            if (!iconWidget) {
                continue;
            }

            iconWidget->setIcon(m_iconName, m_icon);

            //This hardcoded number is needed to support pixel perfection of m_icons coming from other environments, in kde actualsize will just return our usual 22x22
            if (iconWidget->svg().isEmpty()) {
                QSize size = m_icon.actualSize(QSize(24, 24));
                iconWidget->setPreferredSize(iconWidget->sizeFromIconSize(qMax(size.width(), size.height())));
            } else {
                iconWidget->setPreferredSize(24, 24);
            }
        }
    }

    m_attentionIcon = properties["AttentionIcon"].value<QIcon>();
    m_attentionIconName = properties["AttentionIconName"].toString();

    QString m_movieName = properties["AttentionMovieName"].toString();
    syncMovie(m_movieName);

    //FIXME: this is used only on the monochrome ones, the third place where the overlay painting is implemented
    QIcon overlayIcon = properties["OverlayIcon"].value<QIcon>();
    if (overlayIcon.isNull() && !properties["OverlayIconName"].value<QString>().isEmpty()) {
        overlayIcon = KIcon(properties["OverlayIconName"].value<QString>());
    }

    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        DBusSystemTrayWidget *iconWidget = qobject_cast<DBusSystemTrayWidget *>(widget);
        if (iconWidget) {
            iconWidget->setOverlayIcon(overlayIcon);
        }
    }
}

void DBusSystemTrayTask::blinkAttention()
{
    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        DBusSystemTrayWidget *iconWidget = qobject_cast<DBusSystemTrayWidget *>(widget);
        if (iconWidget) {
            iconWidget->setIcon(m_blink ? m_attentionIconName : m_iconName, m_blink ? m_attentionIcon : m_icon);
        }
    }
    m_blink = !m_blink;
}

void DBusSystemTrayTask::syncMovie(const QString &m_movieName)
{
    bool wasRunning = false;
    if (m_movie) {
        wasRunning = m_movie->state() == QMovie::Running;
    }

    delete m_movie;
    if (m_movieName.isEmpty()) {
        m_movie = 0;
        return;
    }
    if (QDir::isAbsolutePath(m_movieName)) {
        m_movie = new QMovie(m_movieName);
    } else {
        m_movie = KIconLoader::global()->loadMovie(m_movieName, KIconLoader::Panel);
    }
    if (m_movie) {
        connect(m_movie, SIGNAL(frameChanged(int)), this, SLOT(updateMovieFrame()));

        if (wasRunning) {
            m_movie->start();
        }
    }
}



void DBusSystemTrayTask::updateMovieFrame()
{
    Q_ASSERT(m_movie);
    QPixmap pix = m_movie->currentPixmap();
    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        Plasma::IconWidget *iconWidget = qobject_cast<Plasma::IconWidget *>(widget);
        if (iconWidget) {
            iconWidget->setIcon(pix);
        }
    }
}


//toolTip

void DBusSystemTrayTask::syncToolTip(const QString &title, const QString &subTitle, const QIcon &toolTipIcon)
{
    if (title.isEmpty()) {
        foreach (QGraphicsWidget *widget, widgetsByHost()) {
            Plasma::ToolTipManager::self()->clearContent(widget);
        }
        return;
    }

    Plasma::ToolTipContent toolTipData(title, subTitle, toolTipIcon);
    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        Plasma::ToolTipManager::self()->setContent(widget, toolTipData);
    }
}


//Status

void DBusSystemTrayTask::syncStatus(QString newStatus)
{
    Task::Status status = (Task::Status)metaObject()->enumerator(metaObject()->indexOfEnumerator("Status")).keyToValue(newStatus.toLatin1());

    if (this->status() == status) {
        return;
    }

    if (status == Task::NeedsAttention) {
        if (m_movie) {
            m_movie->stop();
            m_movie->start();
        } else if (!m_attentionIcon.isNull()) {
            if (!m_blinkTimer) {
                m_blinkTimer = new QTimer(this);
                connect(m_blinkTimer, SIGNAL(timeout()), this, SLOT(blinkAttention()));
                m_blinkTimer->start(500);
            }
        }
    } else {
        if (m_movie) {
            m_movie->stop();
        }

        if (m_blinkTimer) {
            m_blinkTimer->stop();
            m_blinkTimer->deleteLater();
            m_blinkTimer = 0;
        }

        foreach (QGraphicsWidget *widget, widgetsByHost()) {
            DBusSystemTrayWidget *iconWidget = qobject_cast<DBusSystemTrayWidget *>(widget);
            if (iconWidget) {
                iconWidget->setIcon(m_iconName, m_icon);
            }
        }
    }

    setStatus(status);
}

}

#include "dbussystemtraytask.moc"
