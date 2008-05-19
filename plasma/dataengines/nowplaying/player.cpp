#include "player.h"

Player::Player(PlayerFactory* factory)
    : m_factory(factory)
{
}

Player::~Player()
{
}

PlayerFactory* Player::factory() const
{
    return m_factory;
}

QString Player::name() const
{
    Q_ASSERT(!m_name.isEmpty());
    return m_name;
}

QString Player::artist()
{
    return QString();
}

QString Player::album()
{
    return QString();
}

QString Player::title()
{
    return QString();
}

int Player::trackNumber()
{
    return 0;
}

QString Player::comment()
{
    return QString();
}

QString Player::genre()
{
    return QString();
}

int Player::length()
{
    return 0;
}

int Player::position()
{
    return 0;
}

float Player::volume()
{
    return -1;
}

QPixmap Player::artwork()
{
    return QPixmap();
}

bool Player::canPlay()
{
    return false;
}

void Player::play()
{
}

bool Player::canPause()
{
    return false;
}

void Player::pause()
{
}

bool Player::canStop()
{
    return false;
}

void Player::stop()
{
}

bool Player::canGoPrevious()
{
    return false;
}

void Player::previous()
{
}

bool Player::canGoNext()
{
    return false;
}

void Player::next()
{
}

bool Player::canSetVolume()
{
    return false;
}

void Player::setVolume(float)
{
}

bool Player::canSeek()
{
    return false;
}

void Player::seek(int)
{
}

void Player::setName(const QString& name)
{
    m_name = name;
}

