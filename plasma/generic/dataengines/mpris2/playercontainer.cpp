/*
 * Copyright 2012  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "playercontainer.h"

#define MPRIS2_PATH "/org/mpris/MediaPlayer2"
#define DBUS_PROPS_IFACE "org.freedesktop.DBus.Properties"
#define MPRIS2_IFACE "org.mpris.MediaPlayer2"
#define MPRIS2_PLAYER_IFACE "org.mpris.MediaPlayer2.Player"
#define POS_UPD_STRING "Position last updated (UTC)"

#include <KDebug>

#include <QtDBus>
#include <QDateTime>

static QVariant::Type expPropType(const QString& propName)
{
    if (propName == QLatin1String("Identity"))
        return QVariant::String;
    else if (propName == QLatin1String("DesktopEntry"))
        return QVariant::String;
    else if (propName == QLatin1String("SupportedUriSchemes"))
        return QVariant::StringList;
    else if (propName == QLatin1String("SupportedMimeTypes"))
        return QVariant::StringList;
    else if (propName == QLatin1String("Fullscreen"))
        return QVariant::Bool;
    else if (propName == QLatin1String("PlaybackStatus"))
        return QVariant::String;
    else if (propName == QLatin1String("LoopStatus"))
        return QVariant::String;
    else if (propName == QLatin1String("Shuffle"))
        return QVariant::Bool;
    else if (propName == QLatin1String("Rate"))
        return QVariant::Double;
    else if (propName == QLatin1String("MinimumRate"))
        return QVariant::Double;
    else if (propName == QLatin1String("MaximumRate"))
        return QVariant::Double;
    else if (propName == QLatin1String("Volume"))
        return QVariant::Double;
    else if (propName == QLatin1String("Position"))
        return QVariant::LongLong;
    else if (propName == QLatin1String("Metadata"))
        return QVariant::Map;
    // we give out CanControl, as this may completely
    // change the UI of the widget
    else if (propName == QLatin1String("CanControl"))
        return QVariant::Bool;
    return QVariant::Invalid;
}

static PlayerContainer::Cap capFromName(const QString& capName)
{
    if (capName == QLatin1String("CanQuit"))
        return PlayerContainer::CanQuit;
    else if (capName == QLatin1String("CanRaise"))
        return PlayerContainer::CanRaise;
    else if (capName == QLatin1String("CanSetFullscreen"))
        return PlayerContainer::CanSetFullscreen;
    else if (capName == QLatin1String("CanControl"))
        return PlayerContainer::CanControl;
    else if (capName == QLatin1String("CanPlay"))
        return PlayerContainer::CanPlay;
    else if (capName == QLatin1String("CanPause"))
        return PlayerContainer::CanPause;
    else if (capName == QLatin1String("CanSeek"))
        return PlayerContainer::CanSeek;
    else if (capName == QLatin1String("CanGoNext"))
        return PlayerContainer::CanGoNext;
    else if (capName == QLatin1String("CanGoPrevious"))
        return PlayerContainer::CanGoPrevious;
    return PlayerContainer::NoCaps;
}

PlayerContainer::PlayerContainer(const QString& busAddress, QObject* parent)
    : DataContainer(parent)
    , m_caps(NoCaps)
    , m_fetchesPending(0)
    , m_dbusAddress(busAddress)
    , m_currentRate(0.0)
{
    Q_ASSERT(!busAddress.isEmpty());
    Q_ASSERT(busAddress.startsWith(QLatin1String("org.mpris.MediaPlayer2.")));

    m_propsIface = new QDBusInterface(busAddress,
            MPRIS2_PATH, DBUS_PROPS_IFACE,
            QDBusConnection::sessionBus(), this);

    QDBusConnection::sessionBus().connect(
            busAddress,
            MPRIS2_PATH,
            DBUS_PROPS_IFACE,
            "PropertiesChanged", /* signature, */
            this,
            SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

    QDBusConnection::sessionBus().connect(
            busAddress,
            MPRIS2_PATH,
            DBUS_PROPS_IFACE,
            "PropertiesChanged", /* signature, */
            this,
            SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

    QDBusConnection::sessionBus().connect(
            busAddress,
            MPRIS2_PATH,
            MPRIS2_PLAYER_IFACE,
            "Seeked", /* signature, */
            this,
            SLOT(seeked(qint64)));

    refresh();
}

void PlayerContainer::refresh()
{
    // despite these calls being async, we should never update values in the
    // wrong order (eg: a stale GetAll response overwriting a more recent value
    // from a PropertiesChanged signal) due to D-Bus message ordering guarantees.

    QDBusPendingCall async = m_propsIface->asyncCall("GetAll", MPRIS2_IFACE);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,    SLOT(getPropsFinished(QDBusPendingCallWatcher*)));
    ++m_fetchesPending;

    async = m_propsIface->asyncCall("GetAll", MPRIS2_PLAYER_IFACE);
    watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,    SLOT(getPropsFinished(QDBusPendingCallWatcher*)));
    ++m_fetchesPending;
}

void PlayerContainer::copyProperty(const QString& propName, const QVariant& _value, QVariant::Type expType, UpdateType updType)
{
    QVariant value = _value;
    // we protect our users from bogus values
    if (value.userType() == qMetaTypeId<QDBusArgument>()) {
        if (expType == QVariant::Map) {
            QDBusArgument arg = value.value<QDBusArgument>();
            if (arg.currentType() != QDBusArgument::MapType) {
                kWarning() << m_dbusAddress << "exports" << propName
                    << "with the wrong type; it should be D-Bus type \"a{sv}\"";
                return;
            }
            QVariantMap metadata;
            arg >> metadata;
            value = QVariant(metadata);
        }
    }
    if (value.type() != expType) {
        const char * gotTypeCh = QDBusMetaType::typeToSignature(value.userType());
        QString gotType = gotTypeCh ? QString::fromAscii(gotTypeCh) : "<unknown>";
        const char * expTypeCh = QDBusMetaType::typeToSignature(expType);
        QString expType = expTypeCh ? QString::fromAscii(expTypeCh) : "<unknown>";

        kWarning() << m_dbusAddress << "exports" << propName
            << "as D-Bus type" << gotType
            << "but it should be D-Bus type" << expType;
    }
    if (value.convert(expType)) {
        if (propName == QLatin1String("Position")) {

            setData(POS_UPD_STRING, QDateTime::currentDateTimeUtc());

        } else if (propName == QLatin1String("Metadata")) {

            if (updType == UpdatedSignal) {
                QDBusObjectPath oldTrackId = data().value("Metadata").toMap().value("mpris:trackid").value<QDBusObjectPath>();
                QDBusObjectPath newTrackId = value.toMap().value("mpris:trackid").value<QDBusObjectPath>();
                if (oldTrackId != newTrackId) {
                    setData("Position", static_cast<qlonglong>(0));
                    setData(POS_UPD_STRING, QDateTime::currentDateTimeUtc());
                }
            }

        } else if (propName == QLatin1String("Rate") &&
                data().value("PlaybackStatus").toString() == QLatin1String("Playing")) {

            if (data().contains("Position"))
                recalculatePosition();
            m_currentRate = value.toDouble();

        } else if (propName == QLatin1String("PlaybackStatus")) {

            if (data().contains("Position") && data().contains("PlaybackStatus")) {
                recalculatePosition();

                if (data().value("PlaybackStatus").toString() == QLatin1String("Playing")
                        && value.toString() == QLatin1String("Paused")) {
                    // Every time something that affects the predicted position changes
                    // (Rate or PlaybackStatus), we get a hundred milliseconds or so
                    // out, due to the time it takes for a D-Bus message to be delivered
                    // and processed.
                    //
                    // To try to prevent these errors from accumulating, we re-fetch
                    // the position when we enter paused mode; when paused, the position
                    // does not change, so we'll be about as accurate as we can be.
                    updatePosition();
                }
            }

            // update the effective rate
            if (data().contains("Rate")) {
                if (value.toString() == QLatin1String("Playing"))
                    m_currentRate = data().value("Rate").toDouble();
                else
                    m_currentRate = 0.0;
            }
            if (value.toString() == QLatin1String("Stopped")) {
                // assume the position has reset to 0, since this is really the
                // only sensible value for a stopped track
                setData("Position", static_cast<qint64>(0));
                setData(POS_UPD_STRING, QDateTime::currentDateTimeUtc());
            }
        }
        setData(propName, value);
    }
}

void PlayerContainer::updateFromMap(const QVariantMap& map, UpdateType updType)
{
    QMap<QString, QVariant>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        QVariant::Type type = expPropType(i.key());
        if (type != QVariant::Invalid) {
            copyProperty(i.key(), i.value(), type, updType);
        }

        Cap cap = capFromName(i.key());
        if (cap != NoCaps) {
            if (i.value().type() == QVariant::Bool) {
                if (i.value().toBool()) {
                    m_caps |= cap;
                } else {
                    m_caps &= ~cap;
                }
            } else {
                const char * gotTypeCh = QDBusMetaType::typeToSignature(i.value().userType());
                QString gotType = gotTypeCh ? QString::fromAscii(gotTypeCh) : "<unknown>";

                kWarning() << m_dbusAddress << "exports" << i.key()
                    << "as D-Bus type" << gotType
                    << "but it should be D-Bus type \"b\"";
            }
        }
        // fake the CanStop capability
        if (cap == CanControl || i.key() == QLatin1String("PlaybackStatus")) {
            if ((m_caps & CanControl) && i.value().toString() != QLatin1String("Stopped")) {
                kDebug() << "Enabling stop action";
                m_caps |= CanStop;
            } else {
                kDebug() << "Disabling stop action";
                m_caps &= ~CanStop;
            }
        }
        ++i;
    }
}

void PlayerContainer::getPropsFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QVariantMap> propsReply = *watcher;
    watcher->deleteLater();

    if (m_fetchesPending < 1) {
        // we already failed
        return;
    }

    if (propsReply.isError()) {
        kWarning() << m_dbusAddress << "does not implement " DBUS_PROPS_IFACE " correctly";
        kDebug() << "Error message was" << propsReply.error().name() << propsReply.error().message();
        m_fetchesPending = 0;
        emit initialFetchFailed(this);
        return;
    }

    updateFromMap(propsReply.value(), FetchAll);
    checkForUpdate();

    --m_fetchesPending;
    if (m_fetchesPending == 0) {
        emit initialFetchFinished(this);
    }
}

void PlayerContainer::updatePosition()
{
    QDBusPendingCall async = m_propsIface->asyncCall("Get", MPRIS2_PLAYER_IFACE, "Position");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this,    SLOT(getPositionFinished(QDBusPendingCallWatcher*)));
}

void PlayerContainer::getPositionFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QVariant> propsReply = *watcher;
    watcher->deleteLater();

    if (propsReply.isError()) {
        kWarning() << m_dbusAddress << "does not implement " DBUS_PROPS_IFACE " correctly";
        kDebug() << "Error message was" << propsReply.error().name() << propsReply.error().message();
        return;
    }

    setData("Position", propsReply.value().toLongLong());
    setData(POS_UPD_STRING, QDateTime::currentDateTimeUtc());
    checkForUpdate();
}

void PlayerContainer::propertiesChanged(
        const QString& interface,
        const QVariantMap& changedProperties,
        const QStringList& invalidatedProperties)
{
    Q_UNUSED(interface)

    updateFromMap(changedProperties, UpdatedSignal);
    if (!invalidatedProperties.isEmpty()) {
        refresh();
    }
    checkForUpdate();
}

void PlayerContainer::seeked(qint64 position)
{
    setData("Position", position);
    setData(POS_UPD_STRING, QDateTime::currentDateTimeUtc());
    checkForUpdate();
}

void PlayerContainer::recalculatePosition()
{
    Q_ASSERT(data().contains("Position"));

    qint64 pos = data().value("Position").toLongLong();
    QDateTime lastUpdated = data().value(POS_UPD_STRING).toDateTime();
    QDateTime now = QDateTime::currentDateTimeUtc();
    qint64 diff = lastUpdated.msecsTo(now) * 1000;
    qint64 newPos = pos + static_cast<qint64>(diff * m_currentRate);
    setData("Position", newPos);
    setData(POS_UPD_STRING, now);
}

#include "playercontainer.moc"
