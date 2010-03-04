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

#include <KIcon>
#include <KIconLoader>

#include <plasma/plasma.h>
#include <Plasma/Corona>
#include <Plasma/View>
#include <Plasma/IconWidget>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <Plasma/Applet>

#include "dbussystemtraywidget.h"

#include <netinet/in.h>

namespace SystemTray
{

DBusSystemTrayTask::DBusSystemTrayTask(const QString &service, QObject *parent)
    : Task(parent),
      m_movie(0),
      m_blinkTimer(0),
      m_blink(false),
      m_valid(false),
      m_embeddable(false)
{
    setObjectName("DBusSystemTrayTask");
    qDBusRegisterMetaType<KDbusImageStruct>();
    qDBusRegisterMetaType<KDbusImageVector>();
    qDBusRegisterMetaType<KDbusToolTipStruct>();

    m_typeId = service;
    m_name = service;

    m_statusNotifierItemInterface = new org::kde::StatusNotifierItem(service, "/StatusNotifierItem",
                                                                  QDBusConnection::sessionBus(), this);

    //TODO: how to behave if its not m_valid?
    m_valid = !service.isEmpty() && m_statusNotifierItemInterface->isValid();

    if (m_valid) {
        connect(m_statusNotifierItemInterface, SIGNAL(NewIcon()), this, SLOT(refresh()));
        connect(m_statusNotifierItemInterface, SIGNAL(NewAttentionIcon()), this, SLOT(refresh()));
        connect(m_statusNotifierItemInterface, SIGNAL(NewOverlayIcon()), this, SLOT(refresh()));
        connect(m_statusNotifierItemInterface, SIGNAL(NewToolTip()), this, SLOT(refresh()));
        connect(m_statusNotifierItemInterface, SIGNAL(NewStatus(QString)), this, SLOT(syncStatus(QString)));
        refresh();
    }
}


DBusSystemTrayTask::~DBusSystemTrayTask()
{
    delete m_movie;
    delete m_blinkTimer;
}


QGraphicsWidget* DBusSystemTrayTask::createWidget(Plasma::Applet *host)
{
    DBusSystemTrayWidget *m_iconWidget = new DBusSystemTrayWidget(host, m_statusNotifierItemInterface);
    m_iconWidget->show();

    m_iconWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_iconWidget->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    m_iconWidget->setPreferredSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    //Delay because syncStatus needs that createWidget is done
    QTimer::singleShot(0, this, SLOT(refresh()));
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

void DBusSystemTrayTask::refresh()
{
    QDBusMessage message = QDBusMessage::createMethodCall(m_statusNotifierItemInterface->service(),
    m_statusNotifierItemInterface->path(), "org.freedesktop.DBus.Properties", "GetAll");

    message << m_statusNotifierItemInterface->interface();
    QDBusPendingCall call = m_statusNotifierItemInterface->connection().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(refreshCallback(QDBusPendingCallWatcher *)));
}


void DBusSystemTrayTask::refreshCallback(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;
    QVariantMap properties = reply.argumentAt<0>();
    if (reply.isError()) {
        m_valid = false;
        m_embeddable = false;
    } else {
        QString cat = properties["Category"].toString();
        if (!cat.isEmpty()) {
            int index = metaObject()->indexOfEnumerator("Category");
            int key = metaObject()->enumerator(index).keyToValue(cat.toLatin1());

            if (key != -1) {
                setCategory((Task::Category)key);
            }
        }

        syncStatus(properties["Status"].toString());

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

        QIcon overlay;
        QStringList overlayNames;

        //Icon
        {
            KDbusImageVector image;

            properties["OverlayIconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString m_iconName = properties["OverlayIconName"].toString();
                if (!m_iconName.isEmpty()) {
                    overlayNames << m_iconName;
                    overlay = KIcon(m_iconName);
                }
            } else {
                overlay = imageVectorToPixmap(image);
            }

            properties["IconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString m_iconName = properties["IconName"].toString();
                if (!m_iconName.isEmpty()) {
                    m_icon = KIcon(m_iconName, 0, overlayNames);

                    if (overlayNames.isEmpty() && !overlay.isNull()) {
                        overlayIcon(&m_icon, &overlay);
                    }
                }
            } else {
                m_icon = imageVectorToPixmap(image);
                if (!m_icon.isNull() && !overlay.isNull()) {
                    overlayIcon(&m_icon, &overlay);
                }
            }
        }

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

        //Attention m_icon
        {
            KDbusImageVector image;
            properties["AttentionIconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString m_iconName = properties["AttentionIconName"].toString();
                if (!m_iconName.isEmpty()) {
                    m_attentionIcon = KIcon(m_iconName, 0, overlayNames);

                    if (overlayNames.isEmpty() && !overlay.isNull()) {
                        overlayIcon(&m_attentionIcon, &overlay);
                    }
                }
            } else {
                m_attentionIcon = imageVectorToPixmap(image);
                if (!m_attentionIcon.isNull() && !overlay.isNull()) {
                    overlayIcon(&m_icon, &overlay);
                }
            }
        }

        QString m_movieName = properties["AttentionMovieName"].toString();
        syncMovie(m_movieName);

        KDbusToolTipStruct toolTip;
        properties["ToolTip"].value<QDBusArgument>() >> toolTip;
        syncToolTip(toolTip);

        m_embeddable = true;
    }

    emit changed(this);
    delete call;
}

QPixmap DBusSystemTrayTask::KDbusImageStructToPixmap(const KDbusImageStruct &m_icon) const
{
    //swap from network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        uint *uintBuf = (uint *) m_icon.data.data();
        for (uint i = 0; i < m_icon.data.size()/sizeof(uint); ++i) {
            *uintBuf = ntohl(*uintBuf);
            ++uintBuf;
        }
    }
    QImage m_iconImage( m_icon.width, m_icon.height, QImage::Format_ARGB32 );
    memcpy(m_iconImage.bits(), (uchar*)m_icon.data.data(), m_iconImage.numBytes());

    return QPixmap::fromImage(m_iconImage);
}

QIcon DBusSystemTrayTask::imageVectorToPixmap(const KDbusImageVector &vector) const
{
    QIcon m_icon;

    for (int i = 0; i<vector.size(); ++i) {
        m_icon.addPixmap(KDbusImageStructToPixmap(vector[i]));
    }

    return m_icon;
}

void DBusSystemTrayTask::overlayIcon(QIcon *m_icon, QIcon *overlay)
{
    QIcon tmp;
    QPixmap m_iconPixmap = m_icon->pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    QPainter p(&m_iconPixmap);

    const int size = KIconLoader::SizeSmall/2;
    p.drawPixmap(QRect(size, size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
    p.end();
    tmp.addPixmap(m_iconPixmap);

    //if an m_icon exactly that size wasn't found don't add it to the vector
    m_iconPixmap = m_icon->pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    if (m_iconPixmap.width() == KIconLoader::SizeSmallMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&m_iconPixmap);
        p.drawPixmap(QRect(m_iconPixmap.width()-size, m_iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
        tmp.addPixmap(m_iconPixmap);
    }

    m_iconPixmap = m_icon->pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    if (m_iconPixmap.width() == KIconLoader::SizeMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&m_iconPixmap);
        p.drawPixmap(QRect(m_iconPixmap.width()-size, m_iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
        tmp.addPixmap(m_iconPixmap);
    }

    m_iconPixmap = m_icon->pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
    if (m_iconPixmap.width() == KIconLoader::SizeLarge) {
        const int size = KIconLoader::SizeSmallMedium;
        QPainter p(&m_iconPixmap);
        p.drawPixmap(QRect(m_iconPixmap.width()-size, m_iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
        tmp.addPixmap(m_iconPixmap);
    }

    // We can't do 'm_icon->addPixmap()' because if 'm_icon' uses KIconEngine,
    // it will ignore the added pixmaps. This is not a bug in KIconEngine,
    // QIcon::addPixmap() doc says: "Custom m_icon engines are free to ignore
    // additionally added pixmaps".
    *m_icon = tmp;
    //hopefully huge and enormous not necessary right now, since it's quite costly
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

void DBusSystemTrayTask::syncToolTip(const KDbusToolTipStruct &tipStruct)
{
    if (tipStruct.title.isEmpty()) {
        foreach (QGraphicsWidget *widget, widgetsByHost()) {
            Plasma::ToolTipManager::self()->clearContent(widget);
        }
        return;
    }

    QIcon toolTipIcon;
    if (tipStruct.image.size() == 0) {
        toolTipIcon = KIcon(tipStruct.icon);
    } else {
        toolTipIcon = imageVectorToPixmap(tipStruct.image);
    }

    Plasma::ToolTipContent toolTipData(tipStruct.title, tipStruct.subTitle, toolTipIcon);
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
                connect(m_blinkTimer, SIGNAL(timeout()), this, SLOT(m_blinkAttention()));
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
