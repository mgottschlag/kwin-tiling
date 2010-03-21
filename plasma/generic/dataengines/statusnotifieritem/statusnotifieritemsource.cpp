/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *   Copyright (C) 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>       *
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

#include "statusnotifieritemsource.h"
#include "systemtraytypes.h"
#include "statusnotifieritemservice.h"

#include <QIcon>
#include <KIcon>
#include <KIconLoader>
#include <KStandardDirs>
#include <QPainter>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QVariantMap>
#include <QImage>
#include <QPixmap>
#include <QSysInfo>

#include <netinet/in.h>

StatusNotifierItemSource::StatusNotifierItemSource(const QString &service, QObject *parent)
    : Plasma::DataContainer(parent),
      m_customIconLoader(0)
{
    setObjectName(service);
    qDBusRegisterMetaType<KDbusImageStruct>();
    qDBusRegisterMetaType<KDbusImageVector>();
    qDBusRegisterMetaType<KDbusToolTipStruct>();

    m_typeId = service;
    m_name = service;

    m_statusNotifierItemInterface = new org::kde::StatusNotifierItem(service, "/StatusNotifierItem",
                                                                     QDBusConnection::sessionBus(), this);

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

StatusNotifierItemSource::~StatusNotifierItemSource()
{
}

KIconLoader *StatusNotifierItemSource::iconLoader() const
{
    return m_customIconLoader ? m_customIconLoader : KIconLoader::global();
}

Plasma::Service *StatusNotifierItemSource::createService()
{
    return new StatusNotifierItemService(this);
}

void StatusNotifierItemSource::syncStatus(QString status)
{
    setData("Status", status);
    checkForUpdate();
}

void StatusNotifierItemSource::refresh()
{
    QDBusMessage message = QDBusMessage::createMethodCall(m_statusNotifierItemInterface->service(),
                                                          m_statusNotifierItemInterface->path(), "org.freedesktop.DBus.Properties", "GetAll");

    message << m_statusNotifierItemInterface->interface();
    QDBusPendingCall call = m_statusNotifierItemInterface->connection().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(refreshCallback(QDBusPendingCallWatcher *)));
}

/**
  \todo add a smart pointer to guard call and to automatically delete it at the end of the function
  */
void StatusNotifierItemSource::refreshCallback(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;
    QVariantMap properties = reply.argumentAt<0>();
    if (reply.isError()) {
        m_valid = false;
    } else {
        //IconThemePath (handle this one first, because it has an impact on
        //others)
        if (!m_customIconLoader) {
            QString path = properties["IconThemePath"].toString();
            if (!path.isEmpty()) {
                // FIXME: If last part of path is not "icons", this won't work!
                QStringList tokens = path.split('/', QString::SkipEmptyParts);
                if (tokens.length() >= 3 && tokens.takeLast() == "icons") {
                    QString appName = tokens.takeLast();
                    QString prefix = '/' + tokens.join("/");
                    // FIXME: Fix KIconLoader and KIconTheme so that we can use
                    // our own instance of KStandardDirs
                    KGlobal::dirs()->addResourceDir("data", prefix);
                    // We use a separate instance of KIconLoader to avoid
                    // adding all application dirs to KIconLoader::global(), to
                    // avoid potential icon name clashes between application
                    // icons
                    m_customIconLoader = new KIconLoader(appName, 0 /* dirs */, this);
                } else {
                    kWarning() << "Wrong IconThemePath" << path << ": too short or does not end with 'icons'";
                }
            }
        }

        setData("Category", properties["Category"]);
        setData("Status", properties["Status"]);
        setData("Title", properties["Title"]);
        setData("Id", properties["Id"]);
        //Attention Movie
        setData("AttentionMovieName", properties["AttentionMovieName"]);

        QIcon overlay;
        QStringList overlayNames;

        //Icon
        {
            KDbusImageVector image;
            QIcon icon;

            properties["OverlayIconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString iconName = properties["OverlayIconName"].toString();
                if (!iconName.isEmpty()) {
                    overlayNames << iconName;
                    overlay = KIcon(iconName, iconLoader());
                }
            } else {
                overlay = imageVectorToPixmap(image);
            }

            properties["IconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString iconName = properties["IconName"].toString();
                if (!iconName.isEmpty()) {
                    icon = KIcon(iconName, iconLoader(), overlayNames);

                    if (overlayNames.isEmpty() && !overlay.isNull()) {
                        overlayIcon(&icon, &overlay);
                    }
                }
            } else {
                icon = imageVectorToPixmap(image);
                if (!icon.isNull() && !overlay.isNull()) {
                    overlayIcon(&icon, &overlay);
                }
            }
            setData("Icon", icon);
        }

        //Attention icon
        {
            KDbusImageVector image;
            QIcon attentionIcon;

            properties["AttentionIconPixmap"].value<QDBusArgument>() >> image;
            if (image.isEmpty()) {
                QString iconName = properties["AttentionIconName"].toString();
                if (!iconName.isEmpty()) {
                    attentionIcon = KIcon(iconName, iconLoader(), overlayNames);

                    if (overlayNames.isEmpty() && !overlay.isNull()) {
                        overlayIcon(&attentionIcon, &overlay);
                    }
                }
            } else {
                attentionIcon = imageVectorToPixmap(image);
                if (!attentionIcon.isNull() && !overlay.isNull()) {
                    overlayIcon(&attentionIcon, &overlay);
                }
            }
            setData("AttentionIcon", attentionIcon);
        }

        //ToolTip
        {
            KDbusToolTipStruct toolTip;
            properties["ToolTip"].value<QDBusArgument>() >> toolTip;
            if (toolTip.title.isEmpty()) {
                setData("ToolTipTitle", QVariant());
                setData("ToolTipSubTitle", QVariant());
                setData("ToolTipIcon", QVariant());
            } else {
                QIcon toolTipIcon;
                if (toolTip.image.size() == 0) {
                    toolTipIcon = KIcon(toolTip.icon, iconLoader());
                } else {
                    toolTipIcon = imageVectorToPixmap(toolTip.image);
                }
                setData("ToolTipTitle", toolTip.title);
                setData("ToolTipSubTitle", toolTip.subTitle);
                setData("ToolTipIcon", toolTipIcon);
            }
        }
    }

    checkForUpdate();
    delete call;
}

QPixmap StatusNotifierItemSource::KDbusImageStructToPixmap(const KDbusImageStruct &image) const
{
    //swap from network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        uint *uintBuf = (uint *) image.data.data();
        for (uint i = 0; i < image.data.size()/sizeof(uint); ++i) {
            *uintBuf = ntohl(*uintBuf);
            ++uintBuf;
        }
    }
    QImage iconImage(image.width, image.height, QImage::Format_ARGB32 );
    memcpy(iconImage.bits(), (uchar*)image.data.data(), iconImage.numBytes());

    return QPixmap::fromImage(iconImage);
}

QIcon StatusNotifierItemSource::imageVectorToPixmap(const KDbusImageVector &vector) const
{
    QIcon icon;

    for (int i = 0; i<vector.size(); ++i) {
        icon.addPixmap(KDbusImageStructToPixmap(vector[i]));
    }

    return icon;
}

void StatusNotifierItemSource::overlayIcon(QIcon *icon, QIcon *overlay)
{
    QIcon tmp;
    QPixmap m_iconPixmap = icon->pixmap(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    QPainter p(&m_iconPixmap);

    const int size = KIconLoader::SizeSmall/2;
    p.drawPixmap(QRect(size, size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
    p.end();
    tmp.addPixmap(m_iconPixmap);

    //if an m_icon exactly that size wasn't found don't add it to the vector
    m_iconPixmap = icon->pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    if (m_iconPixmap.width() == KIconLoader::SizeSmallMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&m_iconPixmap);
        p.drawPixmap(QRect(m_iconPixmap.width()-size, m_iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
        tmp.addPixmap(m_iconPixmap);
    }

    m_iconPixmap = icon->pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    if (m_iconPixmap.width() == KIconLoader::SizeMedium) {
        const int size = KIconLoader::SizeSmall;
        QPainter p(&m_iconPixmap);
        p.drawPixmap(QRect(m_iconPixmap.width()-size, m_iconPixmap.height()-size, size, size), overlay->pixmap(size, size), QRect(0,0,size,size));
        p.end();
        tmp.addPixmap(m_iconPixmap);
    }

    m_iconPixmap = icon->pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
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
    *icon = tmp;
    //hopefully huge and enormous not necessary right now, since it's quite costly
}

void StatusNotifierItemSource::activate(int x, int y)
{
    m_statusNotifierItemInterface->call(QDBus::NoBlock, "Activate", x, y);
}

void StatusNotifierItemSource::secondaryActivate(int x, int y)
{
    m_statusNotifierItemInterface->call(QDBus::NoBlock, "SecondaryActivate", x, y);
}

void StatusNotifierItemSource::scroll(int delta, const QString &direction)
{
    m_statusNotifierItemInterface->call(QDBus::NoBlock, "Scroll", delta, direction);
}

void StatusNotifierItemSource::contextMenu(int x, int y)
{
    m_statusNotifierItemInterface->call(QDBus::NoBlock, "ContextMenu", x, y);
}

#include "statusnotifieritemsource.moc"
