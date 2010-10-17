/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

class QAbstractAnimation;
class Activity;
class ActivityActionWidget;

class ActivityIcon : public Plasma::AbstractIcon
{
    Q_OBJECT

    public:
        explicit ActivityIcon(const QString &id);
        virtual ~ActivityIcon();

        void setClosable(bool closable);
        Activity* activity();
        void activityRemoved();

        QPixmap pixmap(const QSize &size);
        QMimeData* mimeData();

        void setGeometry(const QRectF & rect);

    private Q_SLOTS:
        void stopActivity();
        void startActivity();
        void showRemovalConfirmation();
        void showConfiguration();
        void makeInlineWidgetVisible();
        void hideInlineWidget();
        void startInlineAnim();
        void updateLayout();
        void updateButtons();

    private:
        QString m_id;

        ActivityActionWidget * m_buttonStop;
        ActivityActionWidget * m_buttonRemove;
        ActivityActionWidget * m_buttonStart;
        ActivityActionWidget * m_buttonConfigure;

        bool m_closable : 1;

        Activity *m_activity;
        QWeakPointer<QGraphicsWidget> m_inlineWidget;
        QAbstractAnimation *m_inlineWidgetAnim;

        friend class ActivityActionWidget;
};

#endif //APPLETICON_H
