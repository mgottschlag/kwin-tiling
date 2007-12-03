/***************************************************************************
 *   Copyright 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
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

#ifndef URL_H
#define URL_H

#include <KUrl>

#include <plasma/applet.h>

class KPropertiesDialog;

namespace Plasma
{
    class Icon;
}

class Url : public Plasma::Applet
{
    Q_OBJECT
    public:
        Url(QObject *parent, const QVariantList &args);
        ~Url();

        void init();
//        QSizeF contentSizeHint() const;
        void setUrl(const KUrl& url);
        void constraintsUpdated(Plasma::Constraints constraints);

    public slots:
        void propertiesDialog();
        void openUrl();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void saveState(KConfigGroup *cg) const;
        //void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    protected slots:
        void acceptedPropertiesDialog();

    private:
        Plasma::Icon* m_icon;
        QString m_text;
        KPropertiesDialog *m_dialog;
        QString m_mimetype;
        KUrl m_url;
};

K_EXPORT_PLASMA_APPLET(url, Url)

#endif
