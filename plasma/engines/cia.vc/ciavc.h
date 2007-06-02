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

#ifndef CIAVC_DATAENGINE_H
#define CIAVC_DATAENGINE_H

#include <QStringList>

#include <syndication/loader.h>
#include <syndication/feed.h>

#include "plasma/dataengine.h"

class QTimer;

/**
 * This class evaluates the basic expressions given in the interface.
 */
class CiaVcEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        CiaVcEngine( QObject* parent, const QStringList& args );
        ~CiaVcEngine();

    protected:
        void init();

    protected slots:
        void updateCiaVc();
        void processProject(Syndication::Loader* loader,
                            Syndication::FeedPtr feed,
                            Syndication::ErrorCode error);

    private:
        QTimer* m_timer;
        Syndication::Loader* m_loader;
        QStringList m_projects;
};

K_EXPORT_PLASMA_DATAENGINE(ciavc, CiaVcEngine)

#endif
