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

#include <QDir>
#include <QGraphicsWidget>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMovie>
#include <QTimer>
#include <QMetaEnum>

#include <KIcon>
#include <KIconLoader>
#include <KStandardDirs>

#include <plasma/plasma.h>
#include <Plasma/Corona>
#include <Plasma/View>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/Applet>
#include <Plasma/DataEngineManager>

#include "dbussystemtraywidget.h"

#include <netinet/in.h>

namespace SystemTray
{

DBusSystemTrayTask::DBusSystemTrayTask(const QString &service, Plasma::Service *dataService, QObject *parent)
    : Task(parent),
      m_movie(0),
      m_blinkTimer(0),
      m_service(dataService),
      m_blink(false),
      m_valid(false),
      m_embeddable(false)
{
    kDebug();
    m_typeId = service;
    m_name = service;

    //TODO: how to behave if its not m_valid?
    m_valid = !service.isEmpty();

    if (m_valid) {
        dataUpdated(service, Plasma::DataEngine::Data());
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
    DBusSystemTrayWidget *m_iconWidget = new DBusSystemTrayWidget(host, m_service);
    m_iconWidget->show();

    m_iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_iconWidget->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    //Delay because syncStatus needs that createWidget is done
//     QTimer::singleShot(0, this, SLOT(connectToData()));
    return m_iconWidget;
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
    kDebug();
    Q_UNUSED(taskName);

    QString cat = properties["Category"].toString();
    if (!cat.isEmpty()) {
        int index = metaObject()->indexOfEnumerator("Category");
        int key = metaObject()->enumerator(index).keyToValue(cat.toLatin1());

        if (key != -1) {
            setCategory((Task::Category)key);
        }
    }

    QString m_title = properties["Title"].toString();
    if (!m_title.isEmpty()) {
        m_name = m_title;

        if (m_typeId.isEmpty()) {
            m_typeId = m_title;
        }
    }

    QString id = properties["Id"].toString();
    if (!id.isEmpty()) {
        m_typeId = id;
    }

    m_icon = properties["Icon"].value<QIcon>();

    if (status() != Task::NeedsAttention) {
        foreach (QGraphicsWidget *widget, widgetsByHost()) {
            Plasma::IconWidget *m_iconWidget = qobject_cast<Plasma::IconWidget *>(widget);
            if (!m_iconWidget) {
                continue;
            }

            m_iconWidget->setIcon(m_icon);
            //This hardcoded number is needed to support pixel perfection of m_icons coming from other environments, in kde actualsize will jusrt return our usual 22x22
            QSize size = m_icon.actualSize(QSize(24, 24));
            m_iconWidget->setPreferredSize(m_iconWidget->sizeFromIconSize(qMax(size.width(), size.height())));
        }
    }

    m_attentionIcon = properties["AttentionIcon"].value<QIcon>();

    QString m_movieName = properties["AttentionMovieName"].toString();
    syncMovie(m_movieName);

    syncStatus(properties["Status"].toString());

    syncToolTip(properties["ToolTipTitle"].toString(),
                properties["ToolTipSubTitle"].toString(),
                properties["ToolTipIcon"].value<QIcon>());


    m_embeddable = true;

    emit changed(this);
}

void DBusSystemTrayTask::blinkAttention()
{
    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        Plasma::IconWidget *m_iconWidget = qobject_cast<Plasma::IconWidget *>(widget);
        if (m_iconWidget) {
            m_iconWidget->setIcon(m_blink ? m_attentionIcon : m_icon);
        }
    }
    m_blink = !m_blink;
}

void DBusSystemTrayTask::syncMovie(const QString &m_movieName)
{
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
    }
}



void DBusSystemTrayTask::updateMovieFrame()
{
    Q_ASSERT(m_movie);
    QPixmap pix = m_movie->currentPixmap();
    foreach (QGraphicsWidget *widget, widgetsByHost()) {
        Plasma::IconWidget *m_iconWidget = qobject_cast<Plasma::IconWidget *>(widget);
        if (m_iconWidget) {
            m_iconWidget->setIcon(pix);
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
            Plasma::IconWidget *m_iconWidget = qobject_cast<Plasma::IconWidget *>(widget);
            if (m_iconWidget) {
                m_iconWidget->setIcon(m_icon);
            }
        }
    }

    setStatus(status);
}

}

#include "dbussystemtraytask.moc"
