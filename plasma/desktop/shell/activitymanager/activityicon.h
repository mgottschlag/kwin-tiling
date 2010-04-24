/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ACTIVITYICON_H
#define ACTIVITYICON_H

#include "abstracticon.h"

#include <KIcon>

class ActivityIcon : public Plasma::AbstractIcon
{
    Q_OBJECT

    public:
        explicit ActivityIcon(const QString &id);
        virtual ~ActivityIcon();

        QPixmap pixmap(const QSize &size);
        QMimeData* mimeData();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    signals:
        void activated(const QString &);

    private slots:
        void activate();

    private:
        QString m_id;
        KIcon m_icon;
};

#endif //APPLETICON_H
