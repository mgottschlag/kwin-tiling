#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject* parent)
    : QObject(parent)
{
}

PollingPlayerFactory::PollingPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
}

DBusPlayerFactory::DBusPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
}

Player::Ptr DBusPlayerFactory::create(const QString& serviceName)
{
    return create(QVariantList() << QVariant(serviceName));
}

#include "playerfactory.moc"
