/*
 *   Copyright (C) 2007 Teemu Rytilahti <tpr@iki.fi>
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

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QStringList>
#include <QVector>

#include <KDebug>
#include <KMimeType>
#include <KRun>
#include <KUrl>
#include <KIcon>

#include "xesamrunner.h"

Q_DECLARE_METATYPE(QList<QList<QVariant> >);
static int typeId = qDBusRegisterMetaType<QList<QList<QVariant> > >();

XesamRunner::XesamRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    KGlobal::locale()->insertCatalog("krunner_xesam");
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Desktop Search"));
}

XesamRunner::~XesamRunner()
{
}

void XesamRunner::match(Plasma::SearchContext *context)
{
    if (context->searchTerm().length()<3) return;

    QDBusInterface xesam("org.freedesktop.xesam.searcher",
                         "/org/freedesktop/xesam/searcher/main");

    QDBusReply<QString> reply = xesam.call("NewSession");
    if (!reply.isValid()) return;
    
    QString session = reply;
    xesam.call("SetProperty", session, "hit.fields",
               QStringList() << "uri" << "dc:title");

    QString query = 
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<request xmlns='http://freedesktop.org/standards/xesam/1.0/query'>"
        "<userQuery>%1</userQuery>"
        "</request>";
    QString userQuery = context->searchTerm();

    reply = xesam.call("NewSearch", session, query.arg(userQuery));
    if (!reply.isValid()) return;

    QString search = reply;

    QDBusReply<void> start_reply = xesam.call("StartSearch", search);
    if (!start_reply.isValid()) return;

    xesam.call("GetHitCount", search);
    QDBusReply<QList<QList<QVariant> > > hits_reply
        = xesam.call("GetHits", search, (quint32)10);
    if (!hits_reply.isValid()) return;

    QList<QList<QVariant> > hits = hits_reply;

    xesam.call("CloseSession", session);

    foreach (QList<QVariant> hit, hits) {
        if (hit.isEmpty()) continue;

        QString url = hit[0].toString();

        QString title;
        if (hit.size()>1) {
            title = hit[1].toString();
        } else {
            title = hit[0].toString();
        }

        Plasma::SearchMatch *match = new Plasma::SearchMatch(this);
        match->setType(Plasma::SearchMatch::PossibleMatch);
        match->setIcon(KIcon("text-x-generic"));
        match->setData(url);
        match->setText(title);
        match->setRelevance(1);
        context->addMatch(userQuery, match);
    }
}

void XesamRunner::exec(const Plasma::SearchContext *context,
                       const Plasma::SearchMatch *match)
{
    QMutexLocker lock(bigLock());
    new KRun(KUrl(match->data().toString()), 0);
}

#include "xesamrunner.moc"
