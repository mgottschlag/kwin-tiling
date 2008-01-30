/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *                 2007 Jos van den Oever <jos@vandenoever.info>
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

#include "searchrunner.h"

#include <QAction>
#include <KActionCollection>
#include <KIcon>
#include <KLocale>
#include <KRun>

SearchRunner::SearchRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner( parent )
{
    KGlobal::locale()->insertCatalog("krunner_searchrunner");
    Q_UNUSED( args );
    setObjectName( i18n( "Search" ) );
    setSpeed(AbstractRunner::SlowSpeed);
    setPriority(AbstractRunner::LowestPriority);
}

SearchRunner::~SearchRunner()
{
}

QString formatUri(const QString& uri, const QString& term) {
    Q_UNUSED( term );
    QString highlighted;
    QString path;
    int l = uri.lastIndexOf("/");
    if (l >= 0) {
        highlighted = uri.mid(l+1);
        path = uri.left(l);
    } else {
        highlighted = uri;
    }
    // it would be nice to be able to make the matching part of the string
    // stand out
    //highlighted.replace(term, "<b>"+term+"</b>");
    highlighted = highlighted + " ("+path+')';
    return highlighted;
}

void SearchRunner::match(Plasma::SearchContext *search)
{
    //TODO: in reality, we probably want to make this async

    QString term = search->searchTerm();
    QString query = "system.file_name:'" + term + "*'";
    QList<StrigiHit> hits = strigiclient.getHits(query, 5, 0);
    foreach (const StrigiHit& hit, hits) {
        QString iconname = hit.mimetype;
        iconname.replace('/', '-');
        QString formatted  = formatUri(hit.uri, term);
        Plasma::SearchMatch* action = search->addPossibleMatch(this);
        action->setIcon(KIcon(iconname));
        action->setText(formatted);
        action->setMimetype(hit.mimetype);
        action->setData(hit.uri);
        action->setRelevance(hit.score);
    }

    Plasma::SearchMatch *action = search->addPossibleMatch(this);
    action->setText(i18n("search for %1", term));
    action->setRelevance(0);
}

void SearchRunner::exec(Plasma::SearchMatch *action)
{
    QString file = action->data().toString();
    kDebug() << "openFile " << file;
    if (file.isEmpty()) {
        KRun::runCommand("strigiclient", 0);
    } else {
        KRun::runUrl(file, action->mimetype(), 0);
    }
}

#include "searchrunner.moc"
