/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *   Copyright (C) 2009 by John Layt <john@layt.net>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef CLOCKAPPLET_H
#define CLOCKAPPLET_H

#include <QtCore/QTime>
#include <QtCore/QDate>

#include <KDE/Plasma/DataEngine>
#include <KDE/Plasma/Dialog>
#include <KDE/Plasma/PopupApplet>
#include <KDE/Plasma/ToolTipManager>

#include "plasmaclock_export.h"

class KDialog;
class KConfigDialog;

namespace Plasma
{
    class Svg;
}

class PLASMACLOCK_EXPORT ClockApplet : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        ClockApplet(QObject *parent, const QVariantList &args);
        ~ClockApplet();

        void init();

        QString currentTimezone() const;
        QString prettyTimezone() const;
        bool isLocalTimezone() const;
        QStringList getSelectedTimezones() const;
        bool shouldDisplayTimezone() const;
        QList<QAction*> contextualActions();

        static QString localTimezone();
        static QString localTimezoneUntranslated();

        const KCalendarSystem *calendar () const;

    public Q_SLOTS:
        void configChanged();
        void toolTipAboutToShow();
        void toolTipHidden();

    protected:
        virtual void createClockConfigurationInterface(KConfigDialog *parent);
        virtual void clockConfigChanged();
        virtual void clockConfigAccepted();
        virtual void changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone);
        virtual Plasma::ToolTipContent toolTipContent();
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        void createConfigurationInterface(KConfigDialog *parent);
        void initExtenderItem(Plasma::ExtenderItem *item);
        void updateTipContent();
        void popupEvent(bool show);
        void constraintsEvent(Plasma::Constraints constraints);

    protected Q_SLOTS:
        void setCurrentTimezone(const QString &tz);
        void configAccepted();
        void updateClockDefaultsTo();
        void dateChanged(const QDate &);
        void speakTime(const QTime &);
        void launchTimeControlPanel();

    private Q_SLOTS:
        void updateClipboardMenu();
        void copyToClipboard(QAction* action);

    private:
        class Private;
        Private * const d;

        Q_PRIVATE_SLOT(d, void createCalendarExtender())
        Q_PRIVATE_SLOT(d, void createToday())
};

#endif
