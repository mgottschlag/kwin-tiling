/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Alex Merry <huntedhacker@tiscali.co.uk>
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

#ifndef EBNENGINE_H
#define EBNENGINE_H

#include <syndication/loader.h>
#include <syndication/feed.h>
#include <QtCore/QHash>

#include "plasma/dataengine.h"

class QTimer;

/**
 * Processes feeds from the English Breakfast Network
 *
 * There are two special keys returned:
 *  * <title> is the title of the feed
 *  * <link> is the link the html version of the feed
 *
 * The rest of the keys are keyed by the title of the
 * module, component or directory the item corresponds to.
 * The value is a QStringList of the following:
 *  0 the number of issues (as a QString)
 *  1 the link to the html version of the list
 *  2 (only if the item is a module or component)
 *    a source that can be requested from the engine
 *
 * TODO: we could make this a QVariantMap, with "issues",
 * "link" and "source" as keys, but (a) this is less
 * efficient, and (b) plasmaengineexplorer currently
 * can't cope with this
 */
class EbnEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        EbnEngine( QObject* parent, const QStringList& args );
        ~EbnEngine();

    protected:
        void init();
        bool sourceRequested(const QString &name);

    protected slots:
        void updateEbn();
        void processFeed(Syndication::Loader* loader,
                         Syndication::FeedPtr feed,
                         Syndication::ErrorCode error);

    private:
        QTimer* m_timer;
        QStringList m_urls;
};

K_EXPORT_PLASMA_DATAENGINE(ebn, EbnEngine)

#endif // EBNENGINE_H
