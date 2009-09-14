#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject* parent)
    : QObject(parent)
{
    setObjectName("PlayerFactory");
}

PollingPlayerFactory::PollingPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
    setObjectName("PollingPlayerFactory");
}

DBusPlayerFactory::DBusPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
    setObjectName("DBusPlayerFactory");
}

Player::Ptr DBusPlayerFactory::create(const QString& serviceName)
{
    return create(QVariantList() << QVariant(serviceName));
}

#include "playerfactory.moc"
