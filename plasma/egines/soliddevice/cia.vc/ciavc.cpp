/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
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

#include "ciavc.h"

#include <QDateTime>
#include <QTimer>

#include <KDebug>
#include <KLocale>

#include <syndication/item.h>

CiaVcEngine::CiaVcEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)

    setSourceLimit(10);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateCiaVc()));
}

CiaVcEngine::~CiaVcEngine()
{
}

void CiaVcEngine::init()
{
}

void CiaVcEngine::updateCiaVc()
{
    foreach (const QString& project, m_projects) {
        Syndication::Loader * loader = Syndication::Loader::create();
        connect(loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
                this, SLOT(processProject(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));
        loader->loadFrom(QString("http://cia.vc/stats/project/%1/.rss").arg(project));
    }
}

bool CiaVcEngine::sourceRequested(const QString &name)
{
    //TODO: projects should be configurable
    m_projects << name;
    updateCiaVc();

    if (!m_timer->isActive()) {
        //TODO: this should be configurable
        m_timer->start(60000);
    }

    // hm... we DID add it, but we don't have a source yet. delayed loading?
    return false;
}

void CiaVcEngine::processProject(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode error)
{
    Q_UNUSED(loader)
    if (error != Syndication::Success) {
        kDebug() << "syndication did not work out" << endl;
        //TODO: should probably tell the user it failed? =)
        return;
    }

    //TODO: implement domains in DataEngine
    QString domain = feed->title();
    kDebug() << "received " << feed->items().count() << " item(s) for " << domain << endl;
    foreach (const Syndication::ItemPtr& item, feed->items()) {
        //FIXME: this is ugly and should be fixed in how DataSources work
        //       is this a candidate for domains? or for a DataSource QVariant
        //       so DataSources can be the data() of another DataSource? hm.
        setData(feed->title(), item->title(), item->description());
    }
}

#include "ciavc.moc"
