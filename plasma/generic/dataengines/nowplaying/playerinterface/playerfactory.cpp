#include "playerfactory.h"

PlayerFactory::PlayerFactory(QObject* parent)
    : QObject(parent)
{
    setObjectName( QLatin1String("PlayerFactory" ));
}

PollingPlayerFactory::PollingPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
    setObjectName( QLatin1String("PollingPlayerFactory" ));
}

DBusPlayerFactory::DBusPlayerFactory(QObject* parent)
    : PlayerFactory(parent)
{
    setObjectName( QLatin1String("DBusPlayerFactory" ));
}

Player::Ptr DBusPlayerFactory::create(const QString& serviceName)
{
    return create(QVariantList() << QVariant(serviceName));
}

#include "playerfactory.moc"
