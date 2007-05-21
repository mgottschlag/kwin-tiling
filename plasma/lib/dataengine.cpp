/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include "dataengine.h"

#include <QTimer>
#include <QVariant>

#include <KDebug>

#include "datasource.h"
#include "datavisualization.h"

namespace Plasma
{

class DataEngine::Private
{
    public:
        Private(DataEngine* e)
            : engine(e)
        {
            updateTimer = new QTimer(engine);
            updateTimer->setSingleShot(true);
        }

        DataSource* source(const QString& sourceName)
        {
            DataSource::Dict::const_iterator it = sources.find(sourceName);
            if (it != sources.constEnd()) {
                return it.value();
            }

            kDebug() << "DataEngine " << engine->objectName()
                     << ": could not find DataSource " << sourceName
                     << ", creating" << endl;
            DataSource* s = new DataSource(engine);
            s->setName(sourceName);
            sources.insert(sourceName, s);
            return s;
        }

        void queueUpdate()
        {
            if (updateTimer->isActive()) {
                return;
            }
            updateTimer->start(0);
        }

        QAtomic ref;
        DataSource::Dict sources;
        DataEngine* engine;
        QTimer* updateTimer;
};


DataEngine::DataEngine(QObject* parent)
    : QObject(parent),
      d(new Private(this))
{
    connect(d->updateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdates()));
    //FIXME: should we try and delay this call?
    init();
}

DataEngine::~DataEngine()
{
    delete d;
}

QStringList DataEngine::dataSources()
{
    return d->sources.keys();
}

void DataEngine::connectSource(const QString& source, DataVisualization* visualization)
{
    Q_UNUSED(source)
    Q_UNUSED(visualization)

    DataSource* s = d->source(source);
//     if (!s) {
//         kDebug() << "DataEngine " << objectName() << ": could not find DataSource " << source << endl;
//         return;
//     }

    connect(s, SIGNAL(updated(Plasma::DataEngine::Data)),
            visualization, SLOT(updated(Plasma::DataEngine::Data)));
}

DataEngine::Data DataEngine::query(const QString& source)
{
    Q_UNUSED(source)

    DataSource* s = d->source(source);
    return s->data();
}

void DataEngine::init()
{
    // default implementation does nothing. this is for engines that have to
    // start things in motion external to themselves before they can work
}

void DataEngine::setData(const QString& source, const QVariant& value)
{
    setData(source, source, value);
}

void DataEngine::setData(const QString& source, const QString& key, const QVariant& value)
{
    DataSource* s = d->source(source);
    s->setData(key, value);
    d->queueUpdate();
}

/*
Plasma::DataSource* DataEngine::createDataSource(const QString& source, const QString& domain)
{
    Q_UNUSED(domain)
    //TODO: add support for domains of sources
    
    if (d->source(source)) {
        kDebug() << "DataEngine " << objectName() << ": source "  << source << " already exists " << endl;
        return s
    }
}*/

void DataEngine::removeDataSource(const QString& source)
{
    DataSource::Dict::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        d->sources.erase(it);
    }
}

void DataEngine::clearAllDataSources()
{
    QMutableHashIterator<QString, Plasma::DataSource*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.remove();
    }
}

void DataEngine::ref()
{
    d->ref.ref();
}

void DataEngine::deref()
{
    d->ref.deref();
}

bool DataEngine::isUsed()
{
    return d->ref != 0;
}

void DataEngine::checkForUpdates()
{
    QHashIterator<QString, Plasma::DataSource*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        it.value()->checkForUpdate();
    }
}

}

#include "dataengine.moc"

