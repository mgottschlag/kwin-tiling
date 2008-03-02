/***************************************************************************
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
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

#ifndef TRASH_H
#define TRASH_H

#include <KUrl>
#include <QAction>
#include <KMenu>
#include <QGraphicsView>
#include <KFileItem>
#include <KDirLister>

#include <plasma/applet.h>

class KPropertiesDialog;
class KFilePlacesModel;
class QAction;

namespace Plasma
{
    class Icon;
}

class Trash : public Plasma::Applet
{
    Q_OBJECT
    public:
        Trash(QObject *parent, const QVariantList &args);
        virtual QList<QAction*> contextActions();
        ~Trash();

        void init();
        void constraintsUpdated(Plasma::Constraints constraints);
        Qt::Orientations expandingDirections() const;

    public slots:
        void slotOpen();
        void slotEmpty();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void createMenu();
        void setIcon();

    protected slots:
        void popup();
        void slotClear();
        void slotCompleted();
        void slotDeleteItem(const KFileItem &);

    private:
        Plasma::Icon* m_icon;
        QList<QAction*> actions;
        KPropertiesDialog *m_dialog;
        KDirLister *m_dirLister;
        KUrl m_trashUrl;
        KMenu m_menu;
        QAction *emptyTrash;
        int m_count;
        bool m_showText;
        Plasma::ToolTipData m_data;
        KFilePlacesModel *m_places;
};

K_EXPORT_PLASMA_APPLET(trash, Trash)

#endif
