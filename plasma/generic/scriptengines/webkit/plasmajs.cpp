/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */
#include "plasmajs.h"
#include "plasmawebapplet.h"

#include <Plasma/DataEngine>

using namespace Plasma;

ConfigGroupWrapper::ConfigGroupWrapper(const KConfigGroup &config)
: m_config(config)
{
}

void ConfigGroupWrapper::setConfig(const KConfigGroup &config)
{
    //kDebug() << config.config()->name() << config.name();
    m_config = config;
}

QVariant ConfigGroupWrapper::readEntry(const QString &key, const QVariant &aDefault) const
{
    // Should KConfig do this?
    // There is readEntry(key, QVariant) but it does not seem to work
    // (if writeEntry was not QVariant??)
    // kDebug() << aDefault.typeName();
    if (aDefault.type() == QVariant::Int) {
        return m_config.readEntry(key, aDefault.toInt());
    } else if (aDefault.type() == QVariant::Double) {
        return m_config.readEntry(key, aDefault.toDouble());
    } else if (aDefault.type() == QVariant::Bool) {
        return m_config.readEntry(key, aDefault.toBool());
    } else {
        return m_config.readEntry(key, aDefault.toString());
    }
}

void ConfigGroupWrapper::writeEntry(const QString &key, const QVariant& value)
{
    m_config.writeEntry(key, value);
}

DataEngineDataWrapper::DataEngineDataWrapper(const DataEngine::Data &data)
    : m_data(data)
{
}

int DataEngineDataWrapper::length() const
{
    return m_data.count();
}

void DataEngineDataWrapper::setData(const Plasma::DataEngine::Data &data)
{
    m_data = data;
}

QVariant DataEngineDataWrapper::value(const QString &key) const
{
    return m_data[key];
}

bool DataEngineDataWrapper::contains(const QString &key) const
{
    return m_data.keys().contains(key);
}

QStringList DataEngineDataWrapper::keys() const
{
    return m_data.keys();
}

QString DataEngineDataWrapper::key(int i) const
{
    return m_data.keys().at(i);
}

DataEngineWrapper::DataEngineWrapper(Plasma::DataEngine *engine, QObject *applet)
    : QObject(engine), m_engine(engine), m_applet(applet)
{
}

QStringList DataEngineWrapper::sources() const
{
    return m_engine->sources();
}

QString DataEngineWrapper::engineName() const
{
    return m_engine->name();
}

bool DataEngineWrapper::isValid() const
{
    return m_engine->isValid();
}

QString DataEngineWrapper::icon() const
{
    return m_engine->icon();
}

QObject * DataEngineWrapper::query(const QString &str) const
{
    DataEngine::Data data = m_engine->query(str);
    return new DataEngineDataWrapper(data);
}

DataEngineWrapper::~DataEngineWrapper()
{
}

void DataEngineWrapper::connectSource(const QString& source,
                                      uint pollingInterval, uint intervalAlignment)
{
    if (m_applet) {
        m_engine->connectSource(source, m_applet, pollingInterval,
                                (Plasma::IntervalAlignment)intervalAlignment);
    }
}

#include "plasmajs.moc"
