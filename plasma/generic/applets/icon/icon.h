/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org                     *
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

#ifndef ICON_APPLET_H
#define ICON_APPLET_H

#include <KMimeType>
#include <KUrl>
#include <KDirWatch>
#include <KService>

#include <Plasma/Applet>

class KPropertiesDialog;

namespace Plasma
{
    class IconWidget;
}

class IconApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        IconApplet(QObject *parent, const QVariantList &args);
        ~IconApplet();

        void init();
        void setUrl(const KUrl& url, bool fromConfigDialog = false);
        void constraintsEvent(Plasma::Constraints constraints);
        void setDisplayLines(int displayLines);
        int displayLines();
        QPainterPath shape() const;

    public slots:
        void openUrl();
        void updateDesktopFile();
        void configChanged();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void saveState(KConfigGroup &cg) const;
        void showConfigurationInterface();
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    private slots:
        void acceptedPropertiesDialog();
        void cancelledPropertiesDialog();
        void delayedDestroy();
        void checkExistenceOfUrl();
        void checkService(const QStringList &service);
        void iconSizeChanged(int group);

    private:
        //dropUrls from DolphinDropController
        void dropUrls(const KUrl::List& urls,
                      const KUrl& destination,
                      Qt::KeyboardModifiers modifier);

        Plasma::IconWidget* m_icon;
        QString m_text;
        QString m_genericName;
        QWeakPointer<KPropertiesDialog> m_dialog;
        KUrl m_url;
        KUrl m_configTarget;
        KDirWatch *m_watcher;
        QSize m_lastFreeSize;
        KService::Ptr m_service;
        bool m_hasDesktopFile;
};

K_EXPORT_PLASMA_APPLET(icon, IconApplet)

#endif
