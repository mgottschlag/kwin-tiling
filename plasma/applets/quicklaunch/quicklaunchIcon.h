/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef QUICKLAUNCHICON_H
#define QUICKLAUNCHICON_H

#include <plasma/widgets/iconwidget.h>

#include <KUrl>
#include <KIcon>

class QAction;
class QGraphicsContextMenuEvent;
class QuicklaunchApplet;

class QuicklaunchIcon : public Plasma::IconWidget
{
    Q_OBJECT
    public:
        /**
         * Constructor
         * @param appUrl Url to desktop file
         * @param mimetype Icon mimetype
         * @param icon Icon object
         */
        QuicklaunchIcon(const KUrl & appUrl, const KIcon & icon, QuicklaunchApplet *parent);

        /**
         * Desctructor
         */
        ~QuicklaunchIcon();

        /**
         * The url for this icon
         * @return Url as KUrl object
         */
        KUrl url() const;

    public slots:
        /**
         * Open the url
         */
        void execute();

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        //bool eventFilter(QObject * object, QEvent * event);

    private:
        QuicklaunchApplet *m_launcher;
        KUrl m_appUrl;
        QAction *m_removeAction;
};

#endif
