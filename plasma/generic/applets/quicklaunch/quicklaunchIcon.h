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
class QuicklaunchApplet;

class QuicklaunchIcon : public Plasma::IconWidget
{
    Q_OBJECT
    public:
        /**
         * Constructor
         * @param appUrl Url to desktop file
         * @param mimetype Icon mimetype
         * @param text Desktop file name
         * @param icon Icon object
         * @param genericName Desktop file generic name
         */
        QuicklaunchIcon(const KUrl & appUrl, const QString & text, const KIcon & icon, const QString & genericName, QuicklaunchApplet *parent);

        /**
         * Desctructor
         */
        ~QuicklaunchIcon();

        /**
         * The url for this icon
         * @return Url as KUrl object
         */
        KUrl url() const;

        QString appName();

        /**
         * Set the size of the icon to be painted
         * @param size the size in pxs
         */
        void setIconSize(int px);

        /**
         * @returns the icon size
         */
        int iconSize() const;

    public slots:
        /**
         * Open the url
         */
        void execute();

        /**
         * @internal Sets the tooltip content properly before showing.
         */
        void toolTipAboutToShow();

        /**
         * @internal Clears memory when needed.
         */
        void toolTipHidden();

    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        //bool eventFilter(QObject * object, QEvent * event);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    private:
        QuicklaunchApplet *m_launcher;
        KUrl m_appUrl;
        QString m_text;
        QString m_genericName;
        QAction *m_removeAction;
        int m_iconSize;
};

#endif
