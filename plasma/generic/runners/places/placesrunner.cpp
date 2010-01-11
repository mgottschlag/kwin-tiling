/*
 *   Copyright 2008 David Edmundson <kde@davidedmundson.co.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "placesrunner.h"

#include <KDebug>
#include <KIcon>
#include <KRun>
#include <KUrl>

PlacesRunner::PlacesRunner(QObject* parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName("Places");
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds file manager locations that match :q:")));
    addSyntax(Plasma::RunnerSyntax(i18n("places"), i18n("Lists all file manager locations")));
    connect(this, SIGNAL(prepare()), this, SLOT(prepPlacesModel()));
}

PlacesRunner::~PlacesRunner()
{
}

void PlacesRunner::prepPlacesModel()
{
    // this may look odd, but this makes sure we have the bookmarks and what not that the places
    // model uses set up so we can get instant matches once we get to that point
    KFilePlacesModel places;
}

void PlacesRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QList<Plasma::QueryMatch> matches;

    if (term.length() < 3) {
        return;
    }

    const bool all = term.compare(i18n("places"), Qt::CaseInsensitive) == 0;
    KFilePlacesModel places;
    for (int i = 0; i <= places.rowCount(); i++) {
        QModelIndex current_index = places.index(i, 0);
        Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
        qreal relevance = 0;

        const QString text = places.text(current_index);
        if ((all && !text.isEmpty()) || text.compare(term, Qt::CaseInsensitive) == 0) {
            type = Plasma::QueryMatch::ExactMatch;
            relevance = all ? 0.9 : 1.0;
        } else if (text.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.7;
        }

        if (type != Plasma::QueryMatch::NoMatch) {
            Plasma::QueryMatch match(this);
            match.setType(type);
            match.setRelevance(relevance);
            match.setIcon(KIcon(places.icon(current_index)));
            match.setText(text);

            //if we have to mount it set the device udi instead of the URL, as we can't open it directly
            KUrl url;
            if (places.isDevice(current_index) && places.setupNeeded(current_index)) {
                url = places.deviceForIndex(current_index).udi();
            } else {
                url = places.url(current_index);
            }

            match.setData(url);
            match.setId(url.prettyUrl());
            matches << match;
        }
    }

    context.addMatches(term, matches);
}


void PlacesRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action)
{
    Q_UNUSED(context);
    //I don't just pass the model index because the list could change before the user clicks on it, which would make everything go wrong. Ideally we don't want things to go wrong.
    if (action.data().canConvert<KUrl>()) {
        new KRun(action.data().value<KUrl>().url(), 0);
    } else if (action.data().canConvert<QString>()) {
        //search our list for the device with the same udi, then set it up (mount it).
        QString deviceUdi = action.data().toString();

        // gets deleted in setupComplete
        KFilePlacesModel *places = new KFilePlacesModel(this);
        connect(places, SIGNAL(setupDone(QModelIndex, bool)), SLOT(setupComplete(QModelIndex, bool)));
        bool found = false;

        for (int i = 0; i <= places->rowCount();i++) {
            QModelIndex current_index = places->index(i, 0);
            if (places->isDevice(current_index) && places->deviceForIndex(current_index).udi() == deviceUdi) {
                places->requestSetup(current_index);
                found = true;
                break;
            }
        }

        if (!found) {
            delete places;
        }
    }
}

//if a device needed mounting, this slot gets called when it's finished.
void PlacesRunner::setupComplete(QModelIndex index, bool success)
{
    KFilePlacesModel *places = qobject_cast<KFilePlacesModel*>(sender());
    //kDebug() << "setup complete" << places << sender();
    if (success && places) {
        new KRun(places->url(index), 0);
        places->deleteLater();
    }
}

#include "placesrunner.moc"
