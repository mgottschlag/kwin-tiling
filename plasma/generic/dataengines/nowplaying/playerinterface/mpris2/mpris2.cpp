/*
 *   Copyright 2011 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mpris2.h"
#include "mpris2_p.h"

#include <QtDBus>
#include <QFile>
#include <QTimer>

#include <kdebug.h>
#include <KIO/NetAccess>

#define MPRIS2_PATH "/org/mpris/MediaPlayer2"
#define MPRIS2_ROOT_IFACE "org.mpris.MediaPlayer2"
#define MPRIS2_PLAYER_IFACE "org.mpris.MediaPlayer2.Player"
#define DBUS_PROPS_IFACE "org.freedesktop.DBus.Properties"

Mpris2Factory::Mpris2Factory(QObject* parent)
    : DBusPlayerFactory(parent)
{
    setObjectName( QLatin1String("Mpris2Factory" ));
}

Player::Ptr Mpris2Factory::create(const QVariantList& args)
{
    if (args.isEmpty()) {
        return Player::Ptr(0);
    }

    QString dbusName(args.first().toString());
    if (dbusName.isEmpty()) {
        return Player::Ptr(0);
    }

    Mpris2* player = new Mpris2(dbusName, this);
    if (!player->isRunning()) {
        delete player;
        player = 0;
    }

    return Player::Ptr(player);
}

bool Mpris2Factory::matches(const QString& serviceName)
{
    return serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."));
}



Mpris2::Mpris2(const QString& name, PlayerFactory* factory)
    : QObject(),
      Player(factory),
      rootIface(0),
      playerIface(0),
      propsIface(0),
      m_pos(0),
      m_rate(0),
      m_currentRate(0),
      m_playerName(name),
      m_volume(0.0f),
      m_state(Stopped),
      m_canControl(false),
      m_canPlay(false),
      m_canPause(false),
      m_canGoPrevious(false),
      m_canGoNext(false),
      m_canSeek(false),
      m_artworkLoaded(false)
{
    if (!name.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
        m_playerName = "org.mpris.MediaPlayer2." + name;
    }
    setName(m_playerName);
    setup();
}

Mpris2::~Mpris2()
{
}

static Mpris2::State playbackStatusToState(const QString &status)
{
    if (status == "Playing") {
        return Mpris2::Playing;
    } else if (status == "Paused") {
        return Mpris2::Paused;
    } else {
        return Mpris2::Stopped;
    }
}

static QVariantMap demarshallMetadata(const QVariant &value)
{
    if (!value.canConvert<QDBusArgument>()) {
        const char * gotTypeCh = QDBusMetaType::typeToSignature(value.userType());
        QString gotType = gotTypeCh ? QString::fromAscii(gotTypeCh) : "<unknown>";
        kDebug() << "Expected variant containing a QDBusArgument, got ready-demarshalled item of type" << gotType;
        return QVariantMap();
    }
    QVariantMap metadata;
    QDBusArgument arg = value.value<QDBusArgument>();
    arg >> metadata;
    return metadata;
}

void Mpris2::updatePosition(qreal rate)
{
    QDBusReply<QDBusVariant> reply = propsIface->call("Get", MPRIS2_PLAYER_IFACE, "Position");
    if (!reply.isValid()) {
        kDebug() << DBUS_PROPS_IFACE ".Get(\"" MPRIS2_PLAYER_IFACE "\", \"Position\") failed at "
                    MPRIS2_PATH " on" << m_playerName << " with error " <<
                    reply.error().name();
        m_pos = position();
    } else {
        m_pos = reply.value().variant().toLongLong() / 1000L;
    }
    m_posLastUpdated = QDateTime::currentDateTime().toUTC();
    m_rate = rate;
    m_currentRate = (m_state == Playing) ? m_rate : 0.0;
}

bool Mpris2::getAllProps()
{
    QDBusReply<QDBusVariant> identityReply = propsIface->call("Get", MPRIS2_ROOT_IFACE, "Identity");
    if (!identityReply.isValid()) {
        kDebug() << DBUS_PROPS_IFACE ".Get(\"" MPRIS2_ROOT_IFACE "\", \"Identity\") failed at "
                    MPRIS2_PATH " on" << m_playerName << " with error " <<
                    identityReply.error().name();
        return false;
    }
    m_identity = identityReply.value().variant().toString();
    if (m_identity.isEmpty()) {
        kDebug() << "Empty player identity; giving up";
        return false;
    }

    QDBusReply<QVariantMap> playerPropsReply = propsIface->call("GetAll", MPRIS2_PLAYER_IFACE);
    if (!playerPropsReply.isValid()) {
        kDebug() << DBUS_PROPS_IFACE ".GetAll(\"" MPRIS2_PLAYER_IFACE "\") failed at "
                    MPRIS2_PATH " on" << m_playerName << " with error " <<
                    playerPropsReply.error().name();
        return false;
    }
    m_posLastUpdated = QDateTime::currentDateTime().toUTC();

    QVariantMap props = playerPropsReply.value();
    m_state = playbackStatusToState(props.value("PlaybackStatus").toString());
    m_metadata = demarshallMetadata(props.value("Metadata"));
    m_volume = props.value("Volume").toDouble();
    m_canControl = props.value("CanControl").toBool();
    m_canPlay = m_canControl && props.value("CanPlay").toBool();
    m_canPause = m_canControl && props.value("CanPause").toBool();
    m_canGoPrevious = m_canControl && props.value("CanGoPrevious").toBool();
    m_canGoNext = m_canControl && props.value("CanGoNext").toBool();
    m_canSeek = m_canControl && props.value("CanSeek").toBool();
    m_pos = props.value("Position").toLongLong() / 1000L;
    m_rate = props.value("Rate").toDouble();
    m_currentRate = (m_state == Playing) ? m_rate : 0.0;

    return true;
}

void Mpris2::setup()
{
    delete propsIface;
    delete rootIface;
    delete playerIface;

    propsIface = new QDBusInterface(m_playerName, MPRIS2_PATH, DBUS_PROPS_IFACE, QDBusConnection::sessionBus(), this);
    rootIface = new QDBusInterface(m_playerName, MPRIS2_PATH, MPRIS2_ROOT_IFACE, QDBusConnection::sessionBus(), this);
    playerIface = new QDBusInterface(m_playerName, MPRIS2_PATH, MPRIS2_PLAYER_IFACE, QDBusConnection::sessionBus(), this);

    if (getAllProps()) {
        QDBusConnection::sessionBus().connect(
                playerIface->service(),
                playerIface->path(),
                playerIface->interface(),
                "Seeked", /* signature, */
                this,
                SLOT(Seeked(qint64)));
        QStringList matchArgs;
        matchArgs << MPRIS2_PLAYER_IFACE;
        QDBusConnection::sessionBus().connect(
                propsIface->service(),
                propsIface->path(),
                propsIface->interface(),
                "PropertiesChanged",
                matchArgs,
                QString(), // signature
                this,
                SLOT(PropertiesChanged(QString,QVariantMap,QStringList)));
    } else {
        m_identity.clear();
    }
}

bool Mpris2::isRunning()
{
    if (m_identity.isEmpty()) {
        setup();
    }

    return !m_identity.isEmpty();
}

Player::State Mpris2::state()
{
    return m_state;
}

QString Mpris2::artist()
{
    if (m_metadata.contains("xesam:artist")) {
        QStringList entries = m_metadata.value("xesam:artist").toStringList();
        if (!entries.isEmpty())
            return entries.first();
    }
    return QString();
}

QString Mpris2::album()
{
    return m_metadata.value("xesam:album").toString();
}

QString Mpris2::title()
{
    return m_metadata.value("xesam:title").toString();
}

int Mpris2::trackNumber()
{
    QVariant track;
    if (m_metadata.contains("xesam:trackNumber")) {
        track = m_metadata.value("xesam:trackNumber");
    }
    if (track.isValid() && track.canConvert(QVariant::Int)) {
        return track.toInt();
    }
    return 0;
}

QString Mpris2::comment()
{
    if (m_metadata.contains("xesam:comment")) {
        return m_metadata.value("xesam:comment").toStringList().join("\n");
    }
    return QString();
}

QString Mpris2::genre()
{
    if (m_metadata.contains("xesam:genre")) {
        QStringList entries = m_metadata.value("xesam:genre").toStringList();
        if (!entries.isEmpty())
            return entries.first();
    }
    return QString();
}

QString Mpris2::lyrics()
{
    return m_metadata.value("xesam:asText").toString();
}

int Mpris2::length()
{
    qlonglong lengthMicros = m_metadata.value("mpris:length").toLongLong();
    return static_cast<int>(lengthMicros / 1000000L);
}

int Mpris2::position()
{
    return static_cast<int>(positionMs() / 1000L);
}

qint64 Mpris2::positionMs()
{
    qint64 elapsed = m_posLastUpdated.msecsTo(QDateTime::currentDateTime());
    return m_pos + (m_currentRate * elapsed);
}

float Mpris2::volume()
{
    return m_volume;
}

QPixmap Mpris2::artwork()
{
    if (m_artworkLoaded) {
        return m_artwork;
    }

    m_artwork = QPixmap();
    const QString arturl = m_metadata.value("mpris:artUrl").toString();
    if (!arturl.isEmpty()) {
        if (!m_artfiles.contains(arturl) ||
            (!m_artfiles[arturl].isEmpty() && !QFile::exists(m_artfiles[arturl]))) {
            QString artfile;
            if (!KIO::NetAccess::download(arturl, artfile, 0)) {
                kWarning() << "Could not download artwork from" << arturl << ":" << KIO::NetAccess::lastErrorString();
                artfile.clear();
            }

            m_artfiles[arturl] = artfile;
        }

        const QString url = m_artfiles.value(arturl);
        if (!url.isEmpty()) {
            m_artwork = QPixmap(url);
        }
    }

    m_artworkLoaded = true;
    return m_artwork;
}

bool Mpris2::canPlay()
{
    return m_canPlay;
}

void Mpris2::play()
{
    playerIface->asyncCall("Play");
}

bool Mpris2::canPause()
{
    return m_canPause;
}

void Mpris2::pause()
{
    playerIface->asyncCall("Pause");
}

bool Mpris2::canStop()
{
    return m_canControl;
}

void Mpris2::stop()
{
    playerIface->asyncCall("Stop");
}

bool Mpris2::canGoPrevious()
{
    return m_canGoPrevious;
}

void Mpris2::previous()
{
    playerIface->asyncCall("Previous");
}

bool Mpris2::canGoNext()
{
    return m_canGoNext;
}

void Mpris2::next()
{
    playerIface->asyncCall("Next");
}

bool Mpris2::canSetVolume()
{
    return m_canControl;
}

void Mpris2::setVolume(qreal volume)
{
    QDBusVariant value = QDBusVariant(QVariant(static_cast<double>(volume)));
    propsIface->asyncCall("Set", MPRIS2_PLAYER_IFACE, "Volume", QVariant::fromValue<QDBusVariant>(value));
}

bool Mpris2::canSeek()
{
    return m_canSeek;
}

void Mpris2::seek(int time)
{
    if (!m_metadata.contains("mpris:trackid")) {
        kDebug() << "No mpris:trackid; aborting seek";
        return;
    }
    QDBusObjectPath trackid = m_metadata.value("mpris:trackid").value<QDBusObjectPath>();
    if (trackid.path().isEmpty()) {
        kDebug() << "Empty path for mpris:trackid; aborting seek";
        return;
    }
    playerIface->asyncCall("SetPosition",
                           QVariant::fromValue<QDBusObjectPath>(trackid),
                           QVariant::fromValue<qint64>((qint64)time * 1000000L));
}

QVariant Mpris2::getPlayerProp(const QString& prop)
{
    QDBusReply<QDBusVariant> reply = propsIface->call("Get", MPRIS2_PLAYER_IFACE, prop);
    if (!reply.isValid()) {
        kDebug() << DBUS_PROPS_IFACE ".Get( \"" MPRIS2_PLAYER_IFACE "\","
                 << prop
                 << ") failed at " MPRIS2_PATH " on"
                 << m_playerName
                 << " with error "
                 << reply.error().name();
        return QVariant();
    }
    return reply.value().variant();
}

bool Mpris2::updateBoolProp(const QString &name, 
                            const QVariantMap& changedProperties,
                            const QStringList& invalidatedProperties,
                            bool  currentVal)
{
    if (changedProperties.contains(name)) {
        kDebug() << "Property" << name << "changed from" << currentVal << "to" << changedProperties.value(name).toBool();
        return changedProperties.value(name).toBool();
    } else if (invalidatedProperties.contains(name)) {
        bool newVal = getPlayerProp(name).toBool();
        kDebug() << "Property" << name << "changed (inv) from" << currentVal << "to" << newVal;
        return newVal;
    }
    return currentVal;
}

// SLOTS

void Mpris2::Seeked(qint64 position)
{
    m_pos = position / 1000L;
    m_posLastUpdated = QDateTime::currentDateTime().toUTC();
}

void Mpris2::PropertiesChanged(const QString& interface,
                               const QVariantMap& changedProperties,
                               const QStringList& invalidatedProperties)
{
    if (interface != MPRIS2_PLAYER_IFACE)
        return;

    State origState = m_state;
    qreal rate = m_rate;
    if (changedProperties.contains("PlaybackStatus")) {
        m_state = playbackStatusToState(changedProperties.value("PlaybackStatus").toString());
    } else if (invalidatedProperties.contains("PlaybackStatus")) {
        m_state = playbackStatusToState(getPlayerProp("PlaybackStatus").toString());
    }
    if (changedProperties.contains("Rate")) {
        rate = changedProperties.value("Rate").toDouble();
    } else if (invalidatedProperties.contains("Rate")) {
        rate = getPlayerProp("Rate").toDouble();
    }
    if (origState != m_state || !qFuzzyCompare(m_rate, rate)) {
        updatePosition(rate);
    }

    if (changedProperties.contains("Metadata")) {
        m_metadata = demarshallMetadata(changedProperties.value("Metadata"));
    } else if (invalidatedProperties.contains("Metadata")) {
        m_metadata = demarshallMetadata(getPlayerProp("Metadata"));
    }

    if (changedProperties.contains("Volume")) {
        m_volume = changedProperties.value("Volume").toDouble();
    } else if (invalidatedProperties.contains("Volume")) {
        m_volume = getPlayerProp("Volume").toDouble();
    }

    m_canPlay = m_canControl && updateBoolProp("CanPlay", changedProperties, invalidatedProperties, m_canPlay);
    m_canPause = m_canControl && updateBoolProp("CanPause", changedProperties, invalidatedProperties, m_canPause);
    m_canGoPrevious = m_canControl && updateBoolProp("CanGoPrevious", changedProperties, invalidatedProperties, m_canGoPrevious);
    m_canGoNext = m_canControl && updateBoolProp("CanGoNext", changedProperties, invalidatedProperties, m_canGoNext);
    m_canSeek = m_canControl && updateBoolProp("CanSeek", changedProperties, invalidatedProperties, m_canSeek);
}

#include "mpris2.moc"
#include "mpris2_p.moc"
// vim:et:sts=4:sw=4
