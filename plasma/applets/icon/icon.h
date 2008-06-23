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

#include <plasma/applet.h>

class KPropertiesDialog;
class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class QEvent;

namespace Plasma
{
    class Icon;
}

class IconApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        IconApplet(QObject *parent, const QVariantList &args);
        ~IconApplet();

        void init();
        void setUrl(const KUrl& url);
        void constraintsEvent(Plasma::Constraints constraints);
        void setDisplayLines(int displayLines);
        int displayLines();
        QPainterPath shape() const;

    public slots:
        void openUrl();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void saveState(KConfigGroup &cg) const;
        void showConfigurationInterface();

    private slots:
        void acceptedPropertiesDialog();
        void propertiesDialogClosed();

    private:
        //dropUrls from DolphinDropController
        void dropUrls(const KUrl::List& urls,
                      const KUrl& destination,
                      Qt::KeyboardModifiers modifier);

        Plasma::Icon* m_icon;
        QString m_text;
        QString m_genericName;
        QAction *m_propertiesAction;
        KPropertiesDialog *m_dialog;
        KMimeType::Ptr m_mimetype;
        KUrl m_url;
        int m_displayLines;
};

K_EXPORT_PLASMA_APPLET(icon, IconApplet)

#endif
