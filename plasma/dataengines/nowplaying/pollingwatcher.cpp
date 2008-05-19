#include "pollingwatcher.h"

#include <QTimer>

#include <KDebug>

#include "player.h"
#include "playerfactory.h"

PollingWatcher::PollingWatcher(QObject* parent)
    : QObject(parent),
      m_timer(0)
{
}

QList<Player::Ptr> PollingWatcher::players()
{
    return m_players.values();
}

void PollingWatcher::addFactory(PollingPlayerFactory* factory)
{
    if (factory->exists()) {
        Player::Ptr player = factory->create();
        if (!player.isNull()) {
            m_players.insert(player);
            m_usedFactories.insert(factory);
            emit newPlayer(player);
        } else {
            kWarning() << "Failed to create a player";
            m_polledFactories.insert(factory);
        }
    } else {
        m_polledFactories.insert(factory);
    }
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(5000);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(checkPlayers()));
        m_timer->start();
    }
}

void PollingWatcher::checkPlayers()
{
    foreach (Player::Ptr player, m_players) {
        if (!player->isRunning()) {
            m_players.remove(player);
            PollingPlayerFactory* factory =
                    qobject_cast<PollingPlayerFactory*>(player->factory());
            if (factory) {
                m_usedFactories.remove(factory);
                m_polledFactories.insert(factory);
            } else {
                kWarning() << "Missing factory for player" << player->name();
            }
            emit playerDisappeared(player);
        }
    }
    foreach (PollingPlayerFactory* factory, m_polledFactories) {
        if (factory->exists()) {
            Player::Ptr player = factory->create();
            if (!player.isNull()) {
                m_players.insert(player);
                m_polledFactories.remove(factory);
                m_usedFactories.insert(factory);
                emit newPlayer(player);
            } else {
                kWarning() << "Failed to create a player";
            }
        }
    }
    m_timer->start();
}

