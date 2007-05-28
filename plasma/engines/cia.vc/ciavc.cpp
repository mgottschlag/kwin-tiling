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

#include <QDateTime>
#include <QTimer>

#include <KDebug>
#include <KLocale>

#include <syndication/item.h>

#include "ciavc.h"

CiaVcEngine::CiaVcEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)

    setSourceLimit(10);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateCiaVc()));

    m_loader = Syndication::Loader::create();
    connect(m_loader, SIGNAL(loadingComplete(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)),
            this, SLOT(processProject(Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode)));

    //TODO: projects should be configurable
    m_projects << "KDE";

    //TODO: this should be configurable
    m_timer->start(60000);
    updateCiaVc();
}

CiaVcEngine::~CiaVcEngine()
{
//    delete m_loader;
}

void CiaVcEngine::init()
{
}

void CiaVcEngine::updateCiaVc()
{
    foreach (const QString& project, m_projects) {
        m_loader->loadFrom(QString("http://cia.vc/stats/project/%1/.rss").arg(project));
    }
}

void CiaVcEngine::processProject(Syndication::Loader* loader, Syndication::FeedPtr feed, Syndication::ErrorCode error)
{
    Q_UNUSED(loader)
    if (error != Syndication::Success) {
        kDebug() << "syndication did not work out" << endl;
        //TODO: should probably tell the user it failed? =)
    }

    //TODO: implement domains in DataEngine
    QString domain = feed->title();
    kDebug() << "recieved " << feed->items().count() << " item(s) for " << domain << endl;
    foreach (const Syndication::ItemPtr& item, feed->items()) {
        //FIXME: this is ugly and should be fixed in how DataSources work
        //       is this a candidate for domains? or for a DataSource QVariant
        //       so DataSources can be the data() of another DataSource? hm.
        setData(item->title(), i18n("Date"), QDateTime::fromTime_t(item->datePublished()));
        setData(item->title(), i18n("Log"), item->description());
    }
}

#include "ciavc.moc"
