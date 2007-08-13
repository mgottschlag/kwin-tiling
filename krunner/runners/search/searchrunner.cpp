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

SearchAction::SearchAction(const QString& f, const QString& iconname,
        const QString& mt, const QString& name, QObject* parent)
    : QAction(KIcon(iconname), name, parent), file(f), mimetype(mt)
{
}

SearchRunner::SearchRunner( QObject* parent, const QStringList& args )
    : Plasma::AbstractRunner( parent )
{
    Q_UNUSED( args );
    setObjectName( i18n( "Search" ) );
}

SearchRunner::~SearchRunner()
{
}

QAction* SearchRunner::accepts( const QString& term )
{
    // return an action that opens a search GUI with this term
    QAction* action = new QAction(i18n("search for %1", term), this);
    connect(action, SIGNAL("triggered"), this, SLOT("launchSearch"));
    return action;
}

bool SearchRunner::exec(QAction* action, const QString& command)
{
    Q_UNUSED(action)
    Q_UNUSED(command)
    return true;
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

void SearchRunner::fillMatches( KActionCollection* matches,
                                const QString& term,
                                int max, int offset )
{

    //TODO: in reality, we probably want to make this async and use matchesUpdated

    QString query = "system.file_name:'" + term + "*'";
    QList<StrigiHit> hits = strigiclient.getHits(query, max, offset);
    foreach(const StrigiHit& hit, hits) {
        QString iconname = hit.mimetype;
        iconname.replace('/', '-');
        QString formatted  = formatUri(hit.uri, term);
        QAction* action = new SearchAction(hit.uri, iconname, hit.mimetype,
            formatted, this);
        connect(action, SIGNAL(triggered()), this, SLOT(openFile()));
        matches->addAction(formatted, action);
    }
}
void SearchRunner::launchSearch()
{
    // TODO this does not work yet and it be better to open a nicer search
    // client e.g. kerry or strigi://
    KRun::runCommand("strigiclient", NULL);
}

void SearchRunner::openFile()
{
    SearchAction* action = qobject_cast<SearchAction*>(sender());
    qDebug() << "openFile " << action;
    if (action) {
        KRun::runUrl(action->file, action->mimetype, NULL);
    }
}

#include "searchrunner.moc"
