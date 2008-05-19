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

#include "plasma/dataenginemanager.h"
#include "plasma/dataengine.h"

#include <qdebug.h>

using namespace Plasma;

PlasmaJs::PlasmaJs(QObject *parent)
    : QObject(parent)
{
}

QObject *PlasmaJs::dataEngine(const QString &name)
{
    DataEngine *engine = DataEngineManager::self()->engine(name);
    DataEngineWrapper *wrapper =  new DataEngineWrapper(engine);
    qDebug()<<"engine is "<<wrapper;
    qDebug()<<"\t name = "<<wrapper->engineName()<<", valid = "<<wrapper->isValid();
    return wrapper;
}

QObject *PlasmaJs::loadDataEngine(const QString& name)
{
    DataEngine *engine = DataEngineManager::self()->loadEngine(name);
    //engine = new TestJs(this);
    DataEngineWrapper *wrapper = new DataEngineWrapper(engine);
    qDebug()<<"engine is "<<wrapper;
    qDebug()<<"engine sources "<<wrapper->sources();
    qDebug()<<"res = "<<wrapper->query("world");
    return wrapper;
}

void PlasmaJs::unloadDataEngine(const QString& name)
{
    return DataEngineManager::self()->unloadEngine(name);
}

QStringList PlasmaJs::knownEngines()
{
    return DataEngineManager::listAllEngines();
}

TestJs::TestJs(QObject *parent)
    : QObject(parent)
{
}

void TestJs::test()
{
    qDebug()<<"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    qDebug()<<"XXXXXXX YES XXXXXXX";
}

DataEngineDataWrapper::DataEngineDataWrapper(const DataEngine::Data &data)
    : m_data(data)
{
}

int DataEngineDataWrapper::size() const
{
    return m_data.count();
}

QVariant DataEngineDataWrapper::value(const QString &key) const
{
    return m_data[key];
}

QStringList DataEngineDataWrapper::keys() const
{
    return m_data.keys();
}

QString DataEngineDataWrapper::key(int i) const
{
    return m_data.keys().at(i);
}

DataEngineWrapper::DataEngineWrapper(Plasma::DataEngine *engine)
    : QObject(engine), m_engine(engine)
{
    DataEngineManager::self()->loadEngine(engine->name()); //FIXME it's getting loaded twice
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
    DataEngineManager::self()->unloadEngine(m_engine->name()); //FIXME it might be getting unloaded twice
}

#include "plasmajs.moc"
